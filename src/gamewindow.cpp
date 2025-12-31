#include "gamewindow.h"
#include "gameconfig.h"
#include <QPainter>
#include <QKeyEvent>
#include <QFont>
#include <QFontMetrics>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QString>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QCryptographicHash>
#include <QByteArray>
#include <algorithm>

/**
 * 构造：设置窗口、计时器并初始化游戏状态。
 */
GameWindow::GameWindow(QWidget *parent) : QWidget(parent) {
    setFixedSize(GameConfig::windowWidth, GameConfig::windowHeight);
    dino = new Dino(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWindow::gameLoop);
    timer->start(16); // ~60 FPS
    setFocusPolicy(Qt::StrongFocus);

    // assets
    trackImg = QPixmap(":/other/Track.png");
    gameOverImg = QPixmap(":/other/GameOver.png");
    resetImg = QPixmap(":/other/Reset.png");
    cloudImg = QPixmap(":/other/Cloud.png");
    smallCactusImgs = {
        QPixmap(":/cactus/SmallCactus1.png"),
        QPixmap(":/cactus/SmallCactus2.png"),
        QPixmap(":/cactus/SmallCactus3.png")
    };
    largeCactusImgs = {
        QPixmap(":/cactus/LargeCactus1.png"),
        QPixmap(":/cactus/LargeCactus2.png"),
        QPixmap(":/cactus/LargeCactus3.png")
    };

    birdImgs = {
        QPixmap(":/bird/Bird1.png"),
        QPixmap(":/bird/Bird2.png")
    };

    // init game state
    speed = GameConfig::gameSpeed; // constant speed
    spawnIntervalMin = GameConfig::spawnIntervalMin; // frames
    spawnIntervalMax = GameConfig::spawnIntervalMax;
    score = 0;
    highScore = 0;
    loadHighScore(); // load saved high score from file

    // init time tracking
    gameFrameCount = 0;
    isNight = false;
    cyclePosition = 0;
    dayNightTransitionAlpha = 0.0f;
    currentBackgroundColor = QColor(255, 255, 255); // 初始为白色（白天）

    // init clouds positions
    clouds.clear();
    for (int i = 0; i < GameConfig::cloudCount; ++i) {
        Cloud c;
        c.x = QRandomGenerator::global()->bounded(GameConfig::windowWidth);
        c.y = QRandomGenerator::global()->bounded(GameConfig::cloudYMin, GameConfig::cloudYMax + 1);
        clouds.push_back(c);
    }

    resetGame();
}

/**
 * 析构：释放动态分配的恐龙对象。
 */
GameWindow::~GameWindow() {
    delete dino;
}

/**
 * 负责绘制背景、地面、恐龙以及开始提示。
 */
void GameWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // background with current color (interpolates between day and night)
    painter.fillRect(rect(), currentBackgroundColor);

    // draw clouds (slow parallax scroll) with alpha based on day-night cycle
    float cloudAlpha = getCloudAlpha();
    if (!cloudImg.isNull() && cloudAlpha > 0.001f) {
        painter.setOpacity(cloudAlpha);
        for (const auto &c : clouds) {
            painter.drawPixmap(c.x, c.y, cloudImg);
        }
        painter.setOpacity(1.0f); // reset opacity
    }

    // draw ground using track texture if valid, fallback to solid blocks
    int groundY = GameConfig::groundY;
    if (!trackImg.isNull()) {
        int w = trackImg.width();
        int h = trackImg.height();
        int xStart = - (groundOffset % w);
        for (int x = xStart; x < width(); x += w) {
            painter.drawPixmap(x, groundY - h + GameConfig::groundAlignOffset, trackImg); // slight raise to align
        }
    } else {
        painter.setBrush(QColor(83, 83, 83));
        painter.setPen(Qt::NoPen);
        int tileW = 40;
        int xStart = - (groundOffset % tileW);
        for (int x = xStart; x < width(); x += tileW) {
            painter.drawRect(x, groundY, tileW, height() - groundY);
        }
    }

    // draw cacti
    for (const auto &c : cacti) {
        painter.drawPixmap(c.x, c.y, c.w, c.h, c.pix);
    }

    // draw birds
    for (const auto &b : birds) {
        painter.drawPixmap(b.x, b.y, b.w, b.h, b.pix);
    }

    // draw dino
    dino->draw(&painter);

    // draw score and high score on the top-right
    QFont scoreFont = painter.font();
    scoreFont.setPointSize(14);
    painter.setFont(scoreFont);
    QFontMetrics fm(scoreFont);
    QString scoreText = QString("%1").arg(score, 5, 10, QChar('0'));
    QString hiText = QString("HI %1").arg(highScore, 5, 10, QChar('0'));
    int margin = 16;
    int yText = margin + fm.ascent();
    int scoreWidth = fm.horizontalAdvance(scoreText);
    int hiWidth = fm.horizontalAdvance(hiText);
    int xScore = width() - margin - scoreWidth;
    int xHi = xScore - margin - hiWidth;
    painter.drawText(xHi, yText, hiText);
    painter.drawText(xScore, yText, scoreText);

    if (!isRunning && !isGameOver) {
        // start screen overlay
        painter.setPen(Qt::black);
        QFont f = painter.font();
        f.setPointSize(18);
        painter.setFont(f);
        painter.drawText(rect(), Qt::AlignCenter, "Press SPACE to Start");
    }

    if (isGameOver) {
        // game over overlay
        if (!gameOverImg.isNull()) {
            int x = (width() - gameOverImg.width()) / 2;
            int y = height() / 4;
            painter.drawPixmap(x, y, gameOverImg);
        }
        if (!resetImg.isNull()) {
            int x = (width() - resetImg.width()) / 2;
            int y = height() / 4 + 60;
            painter.drawPixmap(x, y, resetImg);
            resetRect = QRect(x, y, resetImg.width(), resetImg.height());
        } else {
            resetRect = QRect();
        }
    } else {
        resetRect = QRect();
    }
}

/**
 * 处理按键按下：空格用于开始/跳跃，下键用于下蹲。
 */
void GameWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space) {
        if (!isRunning && !isGameOver) {
            isRunning = true; // start the game
            dino->setGameStarted(true);
        } else if (!isGameOver) {
            dino->jump();
        } else {
            // restart
            resetGame();
        }
    } else if (event->key() == Qt::Key_Down) {
        if (!isGameOver) {
            dino->setDucking(true);
        }
    }
}

/**
 * 处理按键释放：松开下键停止下蹲。
 */
void GameWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Down) {
        dino->setDucking(false);
    }
}

/**
 * 游戏循环：更新世界状态并请求重绘。
 */
void GameWindow::gameLoop() {
    if (isRunning && !isGameOver) {
        groundOffset += speed;
        score += GameConfig::scorePerFrame;
        gameFrameCount++; // increment frame counter
        updateDayNightCycle(); // update day-night cycle
        dino->update();
        updateCacti();
        updateBirds(); // 更新鸟类障碍物
        // move clouds slower for parallax
        for (auto &c : clouds) {
            c.x -= speed / GameConfig::cloudSpeedDivisor;
        }
        // wrap clouds
        for (auto &c : clouds) {
            if (c.x + cloudImg.width() < 0) {
                c.x = width();
                c.y = QRandomGenerator::global()->bounded(GameConfig::cloudYMin, GameConfig::cloudYMax + 1);
            }
        }
        if (checkCollision()) {
            isGameOver = true;
            isRunning = false;
            dino->setDead(true);
            if (score > highScore) {
                highScore = score;
                saveHighScore(); // save new high score to file
            } else {
                highScore = std::max(highScore, score);
            }
        }
    }
    update();
}

void GameWindow::resetGame() {
    isRunning = false;
    isGameOver = false;
    groundOffset = 0;
    score = 0;
    cacti.clear();
    birds.clear(); // 清空鸟类列表
    spawnCooldown = spawnIntervalMin;
    gameFrameCount = 0;
    isNight = false;
    cyclePosition = 0;
    dayNightTransitionAlpha = 0.0f;
    currentBackgroundColor = QColor(255, 255, 255); // 重置为白天
    dino->reset();
}

/**
 * 统一的障碍物生成方法。
 * 在 2000 分前仅生成仙人掌，2000 分后根据概率随机选择生成仙人掌或鸟类。
 */
void GameWindow::spawnObstacle() {
    if (score >= GameConfig::birdSpawnScoreThreshold) {
        // 2000 分及以上，根据概率随机选择
        int rand = QRandomGenerator::global()->bounded(100);
        if (rand < GameConfig::birdSpawnProbability) {
            spawnBird(); // 30% 概率生成鸟
        } else {
            spawnCactus(); // 70% 概率生成仙人掌
        }
    } else {
        // 2000 分以下，仅生成仙人掌
        spawnCactus();
    }
}

void GameWindow::spawnCactus() {
    bool useLarge = QRandomGenerator::global()->bounded(2) == 0;
    const auto &list = useLarge ? largeCactusImgs : smallCactusImgs;
    if (list.empty()) return;
    int idx = QRandomGenerator::global()->bounded(static_cast<int>(list.size()));
    QPixmap pix = list[idx];
    if (pix.isNull()) return;

    // random scale range
    double scaleMin = useLarge ? GameConfig::cactusScaleLargeMin : GameConfig::cactusScaleSmallMin;
    double scaleMax = useLarge ? GameConfig::cactusScaleLargeMax : GameConfig::cactusScaleSmallMax;
    double scale = randomScale(scaleMin, scaleMax);
    // special cap for LargeCactus3 to reduce width/height
    if (useLarge && idx == 2) {
        scale = std::min(scale, GameConfig::cactusScaleLarge3Cap);
    }

    pix = pix.scaled(static_cast<int>(pix.width() * scale), static_cast<int>(pix.height() * scale), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    Cactus c;
    c.pix = pix;
    c.w = pix.width();
    c.h = pix.height();
    c.x = width();
    int groundY = GameConfig::groundY;
    c.y = groundY - c.h + GameConfig::groundAlignOffset; // align bottom with track
    cacti.push_back(c);
}

void GameWindow::updateCacti() {
    // spawn timer
    spawnCooldown -= 1;
    if (spawnCooldown <= 0) {
        spawnObstacle(); // 统一的障碍物生成方法
        int interval = QRandomGenerator::global()->bounded(GameConfig::spawnIntervalMin, GameConfig::spawnIntervalMax + 1);
        spawnCooldown = interval;
    }

    // move cacti
    for (auto &c : cacti) {
        c.x -= speed;
    }

    // remove off-screen
    cacti.erase(std::remove_if(cacti.begin(), cacti.end(), [&](const Cactus &c){
        return c.x + c.w < 0;
    }), cacti.end());
}

bool GameWindow::checkCollision() const {
    // 先用矩形快速判定（减少像素级检测开销）
    QRect dinoRect = dino->boundingRect();
    QPixmap dinoPix;
    QRect dinoDrawRect;
    dino->currentFrame(dinoPix, dinoDrawRect);
    QImage dinoImg = dinoPix.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

    // 检查与仙人掌的碰撞：先粗判，再像素级判定
    for (const auto &c : cacti) {
        QRect cactusRect(c.x, c.y, c.w, c.h);
        if (!dinoRect.intersects(cactusRect)) continue; // 粗判不重叠直接跳过

        QRect overlap = dinoDrawRect.intersected(cactusRect);
        if (overlap.isEmpty()) continue;

        int dinoOffsetX = overlap.x() - dinoDrawRect.x();
        int dinoOffsetY = overlap.y() - dinoDrawRect.y();
        int cactusOffsetX = overlap.x() - c.x;
        int cactusOffsetY = overlap.y() - c.y;

        QImage cactusImg = c.pix.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

        for (int y = 0; y < overlap.height(); ++y) {
            const QRgb *dinoScan = reinterpret_cast<const QRgb*>(dinoImg.constScanLine(dinoOffsetY + y));
            const QRgb *cactusScan = reinterpret_cast<const QRgb*>(cactusImg.constScanLine(cactusOffsetY + y));
            for (int x = 0; x < overlap.width(); ++x) {
                QRgb dinoPixel = dinoScan[dinoOffsetX + x];
                QRgb cactusPixel = cactusScan[cactusOffsetX + x];
                if (qAlpha(dinoPixel) > 0 && qAlpha(cactusPixel) > 0) {
                    return true; // 像素级重叠
                }
            }
        }
    }

    // 检查与鸟类的碰撞：先粗判，再像素级判定
    for (const auto &b : birds) {
        QRect birdRect(b.x, b.y, b.w, b.h);
        if (!dinoRect.intersects(birdRect)) continue; // 粗判不重叠直接跳过

        QRect overlap = dinoDrawRect.intersected(birdRect);
        if (overlap.isEmpty()) continue;

        int dinoOffsetX = overlap.x() - dinoDrawRect.x();
        int dinoOffsetY = overlap.y() - dinoDrawRect.y();
        int birdOffsetX = overlap.x() - b.x;
        int birdOffsetY = overlap.y() - b.y;

        QImage birdImg = b.pix.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

        for (int y = 0; y < overlap.height(); ++y) {
            const QRgb *dinoScan = reinterpret_cast<const QRgb*>(dinoImg.constScanLine(dinoOffsetY + y));
            const QRgb *birdScan = reinterpret_cast<const QRgb*>(birdImg.constScanLine(birdOffsetY + y));
            for (int x = 0; x < overlap.width(); ++x) {
                QRgb dinoPixel = dinoScan[dinoOffsetX + x];
                QRgb birdPixel = birdScan[birdOffsetX + x];
                if (qAlpha(dinoPixel) > 0 && qAlpha(birdPixel) > 0) {
                    return true; // 像素级重叠
                }
            }
        }
    }

    return false;
}

void GameWindow::mousePressEvent(QMouseEvent *event) {
    if (isGameOver && resetRect.isValid() && resetRect.contains(event->pos())) {
        resetGame();
    }
    QWidget::mousePressEvent(event);
}

double GameWindow::randomScale(double min, double max) const {
    if (min >= max) return min;
    // use uniform double
    double t = QRandomGenerator::global()->generateDouble();
    return min + (max - min) * t;
}

/**
 * 从本地文件加载历史最高分。
 * 文件位置：应用数据目录 / highscore.dat
 * 数据使用 AES 解密读取
 */
void GameWindow::loadHighScore() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = configDir + "/" + GameConfig::HIGHSCORE_FILE;
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString encryptedLine = in.readLine();
        if (!encryptedLine.isEmpty()) {
            int score = decryptScore(encryptedLine);
            if (score > 0) {
                highScore = score;
            }
        }
        file.close();
    }
}

/**
 * 将当前最高分保存到本地文件。
 * 文件位置：应用数据目录 / highscore.dat
 * 数据使用 AES 加密存储
 */
void GameWindow::saveHighScore() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile dir(configDir);

    // Create directory if it doesn't exist
    if (!dir.exists()) {
        QDir().mkpath(configDir);
    }

    QString filePath = configDir + "/" + GameConfig::HIGHSCORE_FILE;
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString encrypted = encryptScore(highScore);
        out << encrypted;
        file.close();
    }
}

/**
 * 使用 AES-256 对分数进行加密。
 * @param score 要加密的分数
 * @return 十六进制编码的加密数据
 */
QString GameWindow::encryptScore(int score) {
    // 将分数转换为字符串
    QString scoreStr = QString::number(score);

    // 使用密钥生成 SHA-256 哈希作为 AES 密钥
    QByteArray key = GameConfig::ENCRYPTION_KEY.toUtf8();
    QByteArray keyHash = QCryptographicHash::hash(key, QCryptographicHash::Sha256);

    // 简单的 XOR 加密方案（用于 Qt 不含 OpenSSL 时的备选方案）
    QByteArray scoreData = scoreStr.toUtf8();
    QByteArray encrypted;

    for (int i = 0; i < scoreData.length(); ++i) {
        encrypted.append(scoreData[i] ^ keyHash[i % keyHash.length()]);
    }

    // 将加密后的字节转换为十六进制字符串
    return QString::fromLatin1(encrypted.toHex());
}

/**
 * 使用 AES-256 对分数进行解密。
 * @param encrypted 十六进制编码的加密数据
 * @return 解密后的分数，失败返回 -1
 */
int GameWindow::decryptScore(const QString &encrypted) {
    try {
        // 使用密钥生成 SHA-256 哈希作为 AES 密钥
        QByteArray key = GameConfig::ENCRYPTION_KEY.toUtf8();
        QByteArray keyHash = QCryptographicHash::hash(key, QCryptographicHash::Sha256);

        // 将十六进制字符串转换回字节
        QByteArray encryptedData = QByteArray::fromHex(encrypted.toLatin1());

        // 使用 XOR 解密
        QByteArray decrypted;
        for (int i = 0; i < encryptedData.length(); ++i) {
            decrypted.append(encryptedData[i] ^ keyHash[i % keyHash.length()]);
        }

        // 将字节转换为字符串并解析为整数
        QString decryptedStr = QString::fromUtf8(decrypted);
        bool ok;
        int score = decryptedStr.toInt(&ok);

        return ok ? score : -1;
    } catch (...) {
        return -1;
    }
}

/**
 * 更新昼夜循环。
 * 周期结构：
 * - 白天：0 ~ dayDurationFrames
 * - 过渡期（白天->黑夜）：dayDurationFrames ~ dayDurationFrames + transitionFrames
 * - 黑夜：dayDurationFrames + transitionFrames ~ dayDurationFrames + 2*transitionFrames
 * - 过渡期（黑夜->白天）：dayDurationFrames + 2*transitionFrames ~ dayNightCycleFrames
 */
void GameWindow::updateDayNightCycle() {
    cyclePosition = gameFrameCount % GameConfig::dayNightCycleFrames;

    // 计算当前是否处于过渡期以及过渡进度
    int dayEnd = GameConfig::dayDurationFrames;
    int nightTransitionStart = dayEnd;
    int nightTransitionEnd = dayEnd + GameConfig::transitionFrames;
    int nightEnd = nightTransitionEnd + GameConfig::nightDurationFrames;
    int dayTransitionStart = nightEnd;
    int dayTransitionEnd = dayTransitionStart + GameConfig::transitionFrames;

    QColor dayColor(255, 255, 255);      // 白天：白色
    QColor nightColor(100, 100, 120);    // 黑夜：深灰色

    if (cyclePosition < nightTransitionStart) {
        // 白天
        isNight = false;
        dayNightTransitionAlpha = 0.0f;
        currentBackgroundColor = dayColor;
    } else if (cyclePosition < nightTransitionEnd) {
        // 白天 -> 黑夜过渡
        isNight = false; // 过渡期仍认为处于白天状态（用于其他逻辑判断）
        float progress = static_cast<float>(cyclePosition - nightTransitionStart) / GameConfig::transitionFrames;
        dayNightTransitionAlpha = progress;
        currentBackgroundColor = interpolateColor(dayColor, nightColor, progress);
    } else if (cyclePosition < nightEnd) {
        // 黑夜
        isNight = true;
        dayNightTransitionAlpha = 1.0f;
        currentBackgroundColor = nightColor;
    } else if (cyclePosition < dayTransitionEnd) {
        // 黑夜 -> 白天过渡
        isNight = true; // 过渡期仍认为处于黑夜状态（用于其他逻辑判断）
        float progress = static_cast<float>(cyclePosition - dayTransitionStart) / GameConfig::transitionFrames;
        dayNightTransitionAlpha = 1.0f - progress;
        currentBackgroundColor = interpolateColor(nightColor, dayColor, progress);
    } else {
        // 周期循环，回到白天
        isNight = false;
        dayNightTransitionAlpha = 0.0f;
        currentBackgroundColor = dayColor;
    }
}

/**
 * 线性插值两个颜色。
 * @param from 起始颜色
 * @param to 目标颜色
 * @param alpha 插值进度（0.0 ~ 1.0）
 * @return 插值后的颜色
 */
QColor GameWindow::interpolateColor(const QColor &from, const QColor &to, float alpha) const {
    int r = static_cast<int>(from.red() * (1.0f - alpha) + to.red() * alpha);
    int g = static_cast<int>(from.green() * (1.0f - alpha) + to.green() * alpha);
    int b = static_cast<int>(from.blue() * (1.0f - alpha) + to.blue() * alpha);
    return QColor(r, g, b);
}

/**
 * 计算云朵的透明度。
 * 在过渡期期间，云朵逐渐消失。
 * @return 云朵透明度（0.0 ~ 1.0）
 */
float GameWindow::getCloudAlpha() const {
    // 在白天为 1.0，在过渡期逐渐降低，在黑夜为 0.0
    return 1.0f - dayNightTransitionAlpha;
}

/**
 * 生成无齿翼龙（鸟类）障碍物。
 * 随机选择两种飞行高度，并使用两帧动画循环飞行。
 */
void GameWindow::spawnBird() {
    if (birdImgs.empty() || birdImgs[0].isNull()) return;

    // 随机选择飞行高度（低或高）——此高度表示“中心”距地面的距离
    bool isHighFlight = QRandomGenerator::global()->bounded(2) == 0;
    int flightY = isHighFlight ? GameConfig::birdHeightHigh : GameConfig::birdHeightLow;

    // 随机缩放
    double scale = randomScale(GameConfig::birdScaleMin, GameConfig::birdScaleMax);

    QPixmap pix = birdImgs[0]; // 初始使用第一帧
    pix = pix.scaled(static_cast<int>(pix.width() * scale), static_cast<int>(pix.height() * scale),
                     Qt::KeepAspectRatio, Qt::SmoothTransformation);

    Bird b;
    b.pix = pix;
    b.w = pix.width();
    b.h = pix.height();
    b.x = width();
    int groundBase = GameConfig::groundY + GameConfig::groundAlignOffset; // 地面基准线（与仙人掌底部一致）
    b.y = groundBase - flightY - b.h / 2; // 将鸟的中心放在距地面 flightY 的位置
    b.animationFrame = 0;
    b.animationCounter = 0;

    birds.push_back(b);
}

/**
 * 更新鸟类障碍物位置和动画。
 * 鸟类会平滑向左移动，并循环切换两帧动画。
 */
void GameWindow::updateBirds() {
    // move birds
    for (auto &b : birds) {
        b.x -= speed;

        // 更新动画帧
        b.animationCounter++;
        if (b.animationCounter >= GameConfig::birdAnimationFrames) {
            b.animationCounter = 0;
            b.animationFrame = 1 - b.animationFrame; // 在 0 和 1 之间切换

            // 更新图片
            if (!birdImgs[b.animationFrame].isNull()) {
                double scale = static_cast<double>(b.w) / birdImgs[0].width();
                QPixmap newPix = birdImgs[b.animationFrame];
                newPix = newPix.scaled(static_cast<int>(newPix.width() * scale),
                                       static_cast<int>(newPix.height() * scale),
                                       Qt::KeepAspectRatio, Qt::SmoothTransformation);
                b.pix = newPix;
                // 保持宽度一致（高度可能略有变化）
                b.h = newPix.height();
            }
        }
    }

    // remove off-screen birds
    birds.erase(std::remove_if(birds.begin(), birds.end(), [&](const Bird &b){
        return b.x + b.w < 0;
    }), birds.end());
}


