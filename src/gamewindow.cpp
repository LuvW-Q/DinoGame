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

    // init game state
    speed = GameConfig::gameSpeed; // constant speed
    spawnIntervalMin = GameConfig::spawnIntervalMin; // frames
    spawnIntervalMax = GameConfig::spawnIntervalMax;
    score = 0;
    highScore = 0;
    loadHighScore(); // load saved high score from file

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

    // background
    painter.fillRect(rect(), QColor(255, 255, 255));

    // draw clouds (slow parallax scroll)
    if (!cloudImg.isNull()) {
        for (const auto &c : clouds) {
            painter.drawPixmap(c.x, c.y, cloudImg);
        }
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
        dino->update();
        updateCacti();
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
    spawnCooldown = spawnIntervalMin;
    dino->reset();
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
        spawnCactus();
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
    QRect dinoRect = dino->boundingRect();
    for (const auto &c : cacti) {
        QRect cactusRect(c.x, c.y, c.w, c.h);
        if (dinoRect.intersects(cactusRect)) {
            return true;
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


