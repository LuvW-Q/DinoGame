#include "gamewindow.h"
#include "gameconfig.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QFile>
#include <cmath>
#include <algorithm>

/**
 * 构造：初始化窗口、资源、定时器与初始状态。
 * @param parent 父级窗口，可为空。
 */
GameWindow::GameWindow(QWidget* parent)
    : QWidget(parent),
      timer(new QTimer(this)),
      dino(new Dino(this)),
      isRunning(false),
      isGameOver(false),
      groundOffset(0.0),
      speedPxPerSec(0.0),
      baseSpeedPxPerSec(0.0),
      runtimeSeconds(0.0),
      scoreAccumulator(0.0),
      score(0),
      highScore(0),
      gameFrameCount(0),
      isNight(false),
      cyclePosition(0),
      dayNightTransitionAlpha(0.0f),
      currentBackgroundColor(Qt::white),
      spawnCooldownMs(0.0),
      spawnIntervalMinMs(0.0),
      spawnIntervalMaxMs(0.0),
      lastFrameMs(0) {
    setFixedSize(GameConfig::windowWidth, GameConfig::windowHeight);
    connect(timer, &QTimer::timeout, this, &GameWindow::gameLoop);
    timer->start(16); // ~60 FPS tick, 具体步长由 deltaTime 决定
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

    validateAssetsOrExit();

    // init game state
    baseSpeedPxPerSec = GameConfig::gameSpeed * 60.0; // px/frame * 60fps
    speedPxPerSec = baseSpeedPxPerSec;
    spawnIntervalMinMs = GameConfig::spawnIntervalMin * (1000.0 / 60.0);
    spawnIntervalMaxMs = GameConfig::spawnIntervalMax * (1000.0 / 60.0);
    spawnCooldownMs = spawnIntervalMinMs;
    runtimeSeconds = 0.0;
    scoreAccumulator = 0.0;
    score = 0;
    highScore = 0;
    loadHighScore();

    // init clouds positions
    clouds.clear();
    for (int i = 0; i < GameConfig::cloudCount; ++i) {
        Cloud c{}; // init to avoid clang-tidy warning
        c.x = QRandomGenerator::global()->bounded(GameConfig::windowWidth);
        c.y = QRandomGenerator::global()->bounded(GameConfig::cloudYMin, GameConfig::cloudYMax + 1);
        clouds.push_back(c);
    }

    frameTimer.start();
    lastFrameMs = frameTimer.elapsed();

    resetGame();
}

/**
 * 析构：释放动态资源。
 */
GameWindow::~GameWindow() {
    delete dino;
}

/**
 * 绘制场景：背景、云朵、地面、障碍、恐龙、得分与提示。
 */
void GameWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    // 背景填充
    painter.fillRect(rect(), currentBackgroundColor);

    // 绘制云朵（带视差）
    if (!cloudImg.isNull()) {
        painter.save();
        painter.setOpacity(getCloudAlpha());
        for (const auto& c : clouds) {
            painter.drawPixmap(c.x, c.y, cloudImg);
        }
        painter.restore();
    }

    // 绘制地面：优先使用赛道贴图，缺失则灰色填充
    int groundY = GameConfig::groundY;
    if (!trackImg.isNull()) {
        int w = trackImg.width();
        int h = trackImg.height();
        int xStart = -(static_cast<int>(groundOffset) % w);
        for (int x = xStart; x < width(); x += w) {
            painter.drawPixmap(x, groundY - h + GameConfig::groundAlignOffset, trackImg); // 微调对齐
        }
    }
    else {
        painter.setBrush(QColor(83, 83, 83));
        painter.setPen(Qt::NoPen);
        int tileW = 40;
        int xStart = -(static_cast<int>(groundOffset) % tileW);
        for (int x = xStart; x < width(); x += tileW) {
            painter.drawRect(x, groundY, tileW, height() - groundY);
        }
    }

    // 绘制仙人掌
    for (const auto& c : cacti) {
        painter.drawPixmap(c.x, c.y, c.w, c.h, c.pix);
    }
    for (const auto& b : birds) {
        painter.drawPixmap(b.x, b.y, b.w, b.h, b.pix);
    }

    // 绘制恐龙
    dino->draw(&painter);

    // 绘制分数与最高分（右上角）
    QFont scoreFont = painter.font();
    scoreFont.setPointSize(14);
    painter.setFont(scoreFont);
    // 文本颜色随昼夜切换以保证对比度
    const int bgR = currentBackgroundColor.red();
    const int bgG = currentBackgroundColor.green();
    const int bgB = currentBackgroundColor.blue();
    const int brightness = static_cast<int>(0.299 * bgR + 0.587 * bgG + 0.114 * bgB);
    painter.setPen(brightness < 128 ? Qt::white : Qt::black);
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
        // 开始提示
        painter.setPen(Qt::black);
        QFont f = painter.font();
        f.setPointSize(18);
        painter.setFont(f);
        painter.drawText(rect(), Qt::AlignCenter, "Press SPACE to Start");
    }

    if (isGameOver) {
        // 结束提示
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
        }
        else {
            resetRect = QRect();
        }
    }
    else {
        resetRect = QRect();
    }
}

/**
 * 处理按键按下：空格启动/跳跃，方向下键下蹲，结束后空格重开。
 */
void GameWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        if (!isRunning && !isGameOver) {
            isRunning = true; // start the game
            dino->setGameStarted(true);
        }
        else if (!isGameOver) {
            dino->jump();
        }
        else {
            // restart
            resetGame();
        }
    }
    else if (event->key() == Qt::Key_Down) {
        if (!isGameOver) {
            dino->setDucking(true);
        }
    }
}

/**
 * 处理按键释放：松开下蹲。
 */
void GameWindow::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Down) {
        dino->setDucking(false);
    }
}

/**
 * 游戏主循环：基于 delta time 更新速度、分数、昼夜、障碍与碰撞。
 */
void GameWindow::gameLoop() {
    const qint64 nowMs = frameTimer.elapsed();
    double deltaSeconds = static_cast<double>(nowMs - lastFrameMs) / 1000.0;
    if (deltaSeconds <= 0 || deltaSeconds > 0.25) {
        deltaSeconds = 0.016; // 防止长时间挂起导致跳变
    }
    lastFrameMs = nowMs;

    if (isRunning && !isGameOver) {
        runtimeSeconds += deltaSeconds;
        speedPxPerSec = currentSpeedForSeconds(runtimeSeconds);
        groundOffset += speedPxPerSec * deltaSeconds;

        // 计分：按 60fps 等效节奏
        scoreAccumulator += deltaSeconds * 60.0 * GameConfig::scorePerFrame;
        while (scoreAccumulator >= 1.0) {
            score += 1;
            scoreAccumulator -= 1.0;
        }

        gameFrameCount += static_cast<int>(std::lround(deltaSeconds * 60.0));
        updateDayNightCycle();
        dino->update();
        updateCacti(deltaSeconds);
        updateBirds(deltaSeconds);

        // 云朵随地面移动但速度更慢形成视差
        const double cloudSpeed = speedPxPerSec / GameConfig::cloudSpeedDivisor;
        for (auto& c : clouds) {
            c.x -= static_cast<int>(std::lround(cloudSpeed * deltaSeconds));
        }
        // wrap clouds
        for (auto& c : clouds) {
            if (c.x + cloudImg.width() < 0) {
                c.x = width();
                c.y = QRandomGenerator::global()->bounded(GameConfig::cloudYMin, GameConfig::cloudYMax + 1);
            }
        }
        if (checkCollision()) {
            isGameOver = true;
            isRunning = false;
            dino->setDead(true);
            highScore = std::max(highScore, score);
            saveHighScore();
        }
    }
    update();
}

/**
 * 重置一局数据，等待再次开始。
 */
void GameWindow::resetGame() {
    isRunning = false;
    isGameOver = false;
    groundOffset = 0.0;
    score = 0;
    scoreAccumulator = 0.0;
    gameFrameCount = 0;
    runtimeSeconds = 0.0;
    isNight = false;
    cyclePosition = 0;
    dayNightTransitionAlpha = 0.0f;
    currentBackgroundColor = QColor(255, 255, 255);
    cacti.clear();
    birds.clear();
    spawnCooldownMs = spawnIntervalMinMs;
    dino->reset();
    lastFrameMs = frameTimer.restart();
}

/**
 * 生成仙人掌障碍。
 */
void GameWindow::spawnCactus() {
    bool useLarge = QRandomGenerator::global()->bounded(2) == 0;
    const auto& list = useLarge ? largeCactusImgs : smallCactusImgs;
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

/**
 * 更新仙人掌：生成计时、移动、回收。
 * @param deltaSeconds 本帧耗时（秒）。
 */
void GameWindow::updateCacti(double deltaSeconds) {
    // 生成计时（毫秒）
    spawnCooldownMs -= deltaSeconds * 1000.0;
    if (spawnCooldownMs <= 0.0) {
        spawnObstacle();
        double interval = spawnIntervalMinMs + (spawnIntervalMaxMs - spawnIntervalMinMs) * QRandomGenerator::global()->generateDouble();
        spawnCooldownMs = interval;
    }

    // 移动仙人掌
    for (auto& c : cacti) {
        c.x -= static_cast<int>(std::lround(speedPxPerSec * deltaSeconds));
    }

    // 移除越界仙人掌
    cacti.erase(std::remove_if(cacti.begin(), cacti.end(), [&](const Cactus& c) {
        return c.x + c.w < 0;
        }), cacti.end());
}

/**
 * 更新鸟类：移动、动画、回收。
 * @param deltaSeconds 本帧耗时（秒）。
 */
void GameWindow::updateBirds(double deltaSeconds) {
    if (birds.empty()) return;

    const int animInterval = std::max(1, GameConfig::birdAnimationFrames);
    int frameAdvance = std::max(1, static_cast<int>(std::lround(deltaSeconds * 60.0)));

    for (auto &b : birds) {
        b.x -= static_cast<int>(std::lround(speedPxPerSec * deltaSeconds));

        b.animationCounter += frameAdvance;
        if (b.animationCounter >= animInterval) {
            b.animationCounter = 0;
            b.animationFrame = 1 - b.animationFrame;
            if (birdImgs.size() > static_cast<size_t>(b.animationFrame)) {
                b.pix = birdImgs[b.animationFrame].scaled(b.w, b.h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
        }
    }

    birds.erase(std::remove_if(birds.begin(), birds.end(), [&](const Bird &b){ return b.x + b.w < 0; }), birds.end());
}

/**
 * 碰撞检测，先矩形再像素级判定。
 * @return true 表示碰撞。
 */
bool GameWindow::checkCollision() const {
    // 矩形粗判
    QRect dinoRect = dino->boundingRect();
    for (const auto& c : cacti) {
        QRect cactusRect(c.x, c.y, c.w, c.h);
        if (!dinoRect.intersects(cactusRect)) {
            continue;
        }
        // 像素级检测：恐龙当前帧对比仙人掌贴图
        QPixmap dinoPix;
        QRect dinoDrawRect;
        dino->currentFrame(dinoPix, dinoDrawRect);
        // 使用仙人掌贴图的屏幕矩形
        if (pixelCollision(dinoPix, dinoDrawRect, c.pix, cactusRect)) {
            return true;
        }
    }
    for (const auto& b : birds) {
        QRect birdRect(b.x, b.y, b.w, b.h);
        if (!dinoRect.intersects(birdRect)) continue;
        QPixmap dinoPix;
        QRect dinoDrawRect;
        dino->currentFrame(dinoPix, dinoDrawRect);
        if (pixelCollision(dinoPix, dinoDrawRect, b.pix, birdRect)) {
            return true;
        }
    }
    return false;
}

/**
 * 鼠标点击：用于点击重开按钮。
 */
void GameWindow::mousePressEvent(QMouseEvent* event) {
    if (isGameOver && resetRect.isValid() && resetRect.contains(event->pos())) {
        resetGame();
    }
    QWidget::mousePressEvent(event);
}

/**
 * 返回[min,max]内随机缩放系数。
 */
 double GameWindow::randomScale(double min, double max) const { // NOLINT(readability-convert-member-functions-to-static)
    if (min >= max) return min;
    // use uniform double
    double t = QRandomGenerator::global()->generateDouble();
    return min + (max - min) * t;
}

/**
 * 更新昼夜周期与背景色。
 */
void GameWindow::updateDayNightCycle() {
    const QColor dayColor(255, 255, 255);
    const QColor nightColor(32, 32, 32);
    const int cycleTotal = std::max(1, GameConfig::dayNightCycleFrames);
    const int transition = std::max(1, GameConfig::transitionFrames);
    const int dayLen = GameConfig::dayDurationFrames;
    const int nightLen = GameConfig::nightDurationFrames;

    int pos = gameFrameCount % cycleTotal;
    cyclePosition = pos;

    int dayEnd = dayLen;
    int dnTransEnd = dayEnd + transition;
    int nightEnd = dnTransEnd + nightLen;
    int ndTransEnd = nightEnd + transition;

    if (pos < dayEnd) {
        isNight = false;
        dayNightTransitionAlpha = 0.0f;
        currentBackgroundColor = dayColor;
    }
    else if (pos < dnTransEnd) {
        isNight = false; // transitioning to night
        float alpha = float(pos - dayEnd) / float(transition);
        dayNightTransitionAlpha = alpha;
        currentBackgroundColor = interpolateColor(dayColor, nightColor, alpha);
    }
    else if (pos < nightEnd) {
        isNight = true;
        dayNightTransitionAlpha = 0.0f;
        currentBackgroundColor = nightColor;
    }
    else { // night -> day transition
        isNight = true;
        float alpha = float(pos - nightEnd) / float(transition);
        dayNightTransitionAlpha = alpha;
        currentBackgroundColor = interpolateColor(nightColor, dayColor, alpha);
    }

    // wrap safety if config sums differ
    if (ndTransEnd < cycleTotal && pos >= ndTransEnd) {
        // unexpected config mismatch, restart to day baseline
        isNight = false;
        dayNightTransitionAlpha = 0.0f;
        currentBackgroundColor = dayColor;
    }
}

/**
 * 颜色线性插值。
 * @param from 起始色。
 * @param to 目标色。
 * @param alpha 0-1 插值系数。
 * @return 插值结果颜色。
 */
QColor GameWindow::interpolateColor(const QColor &from, const QColor &to, float alpha) const { // NOLINT(readability-convert-member-functions-to-static)
     alpha = std::clamp(alpha, 0.0f, 1.0f);
     double rF = static_cast<double>(from.red()) + static_cast<double>(to.red() - from.red()) * alpha;
     double gF = static_cast<double>(from.green()) + static_cast<double>(to.green() - from.green()) * alpha;
     double bF = static_cast<double>(from.blue()) + static_cast<double>(to.blue() - from.blue()) * alpha;
     int r = static_cast<int>(std::lround(rF));
     int g = static_cast<int>(std::lround(gF));
     int b = static_cast<int>(std::lround(bF));
     return {r, g, b};
 }

/**
 * 获取云朵透明度（受昼夜过渡影响）。
 */
float GameWindow::getCloudAlpha() const {
    constexpr float nightAlpha = 0.55f;
    constexpr float dayAlpha = 1.0f;
    if (dayNightTransitionAlpha > 0.0f) {
        if (!isNight) { // day -> night
            return dayAlpha - (dayAlpha - nightAlpha) * dayNightTransitionAlpha;
        }
        else { // night -> day
            return nightAlpha + (dayAlpha - nightAlpha) * dayNightTransitionAlpha;
        }
    }
    return isNight ? nightAlpha : dayAlpha;
}

/**
 * 像素级 alpha 碰撞检测。
 * @param aPix A 贴图。
 * @param aRect A 屏幕矩形。
 * @param bPix B 贴图。
 * @param bRect B 屏幕矩形。
 * @return true 表示两者在重叠区域均有不透明像素。
 */
bool GameWindow::pixelCollision(const QPixmap &aPix, const QRect &aRect, const QPixmap &bPix, const QRect &bRect) const { // NOLINT(readability-convert-member-functions-to-static)
    QRect overlap = aRect.intersected(bRect);
    if (overlap.isEmpty()) return false;

    // scale images to their on-screen rects so sampling aligns with what is drawn
    QImage aImg = aPix.toImage().convertToFormat(QImage::Format_ARGB32);
    if (aImg.size() != aRect.size()) {
        aImg = aImg.scaled(aRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    QImage bImg = bPix.toImage().convertToFormat(QImage::Format_ARGB32);
    if (bImg.size() != bRect.size()) {
        bImg = bImg.scaled(bRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    constexpr int alphaThreshold = 32; // 忽略弱透明边缘像素

    for (int y = overlap.top(); y <= overlap.bottom(); ++y) {
        int aY = y - aRect.top();
        int bY = y - bRect.top();
        const QRgb *aScan = reinterpret_cast<const QRgb*>(aImg.scanLine(aY));
        const QRgb *bScan = reinterpret_cast<const QRgb*>(bImg.scanLine(bY));
        for (int x = overlap.left(); x <= overlap.right(); ++x) {
            int aX = x - aRect.left();
            int bX = x - bRect.left();
            if (qAlpha(aScan[aX]) > alphaThreshold && qAlpha(bScan[bX]) > alphaThreshold) {
                return true;
            }
        }
    }
    return false;
}

/**
 * 按概率生成障碍（鸟或仙人掌）。
 */
void GameWindow::spawnObstacle() {
    bool canSpawnBird = score >= GameConfig::birdSpawnScoreThreshold && birdImgs.size() >= 2 && !birdImgs[0].isNull() && !birdImgs[1].isNull();
    int roll = QRandomGenerator::global()->bounded(100);
    if (canSpawnBird && roll < GameConfig::birdSpawnProbability) {
        spawnBird();
    } else {
        spawnCactus();
    }
}

/**
 * 根据运行时间计算当前速度（含上限）。
 * @param runSeconds 运行时间（秒）。
 * @return 速度（像素/秒）。
 */
 double GameWindow::currentSpeedForSeconds(double runSeconds) const {
    constexpr double accelPerSecond = 18.0; // px/s 每秒增加，约等于每分钟+1080 px/s（平滑变难）
    constexpr double maxSpeed = 15.0 * 60.0; // 上限：15px/帧 @60fps
    double s = baseSpeedPxPerSec + accelPerSecond * runSeconds;
    return std::min(s, maxSpeed);
}

/**
 * 校验关键资源，缺失则退出。
 */
void GameWindow::validateAssetsOrExit() {
    auto fail = [](const QString &msg) {
        qCritical().noquote() << msg;
        QCoreApplication::exit(1);
    };

    if (trackImg.isNull()) fail("缺少赛道贴图:/other/Track.png");
    if (gameOverImg.isNull()) fail("缺少 GameOver 贴图:/other/GameOver.png");
    if (resetImg.isNull()) fail("缺少 Reset 贴图:/other/Reset.png");
    if (cloudImg.isNull()) fail("缺少云朵贴图:/other/Cloud.png");
    if (smallCactusImgs.empty() || std::any_of(smallCactusImgs.begin(), smallCactusImgs.end(), [](const QPixmap &p){return p.isNull();})) {
        fail("缺少小型仙人掌贴图:/cactus/SmallCactus*.png");
    }
    if (largeCactusImgs.empty() || std::any_of(largeCactusImgs.begin(), largeCactusImgs.end(), [](const QPixmap &p){return p.isNull();})) {
        fail("缺少大型仙人掌贴图:/cactus/LargeCactus*.png");
    }
    if (birdImgs.size() < 2 || birdImgs[0].isNull() || birdImgs[1].isNull()) {
        fail("缺少鸟类贴图:/bird/Bird1.png 或 /bird/Bird2.png");
    }
}

/**
 * 读取最高分（含解密）。
 */
void GameWindow::loadHighScore() {
    QFile f(GameConfig::HIGHSCORE_FILE);
    if (!f.exists()) return;
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "无法读取高分文件" << f.errorString();
        return;
    }
    const QByteArray data = f.readAll();
    f.close();
    const int loaded = decryptScore(QString::fromUtf8(data));
    if (loaded >= 0) {
        highScore = loaded;
    }
}

/**
 * 保存最高分（含加密）。
 */
void GameWindow::saveHighScore() {
    QFile f(GameConfig::HIGHSCORE_FILE);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "无法写入高分文件" << f.errorString();
        return;
    }
    const QString enc = encryptScore(highScore);
    f.write(enc.toUtf8());
    f.close();
}

/**
 * 加密分数。
 * @param score 分数。
 * @return 加密后的字符串。
 */
QString GameWindow::encryptScore(int score) { // NOLINT(readability-convert-member-functions-to-static)
    QByteArray raw = QByteArray::number(score);
    const QByteArray key = GameConfig::ENCRYPTION_KEY.toUtf8();
    if (key.isEmpty()) return QString::fromUtf8(raw.toBase64());
    for (int i = 0; i < raw.size(); ++i) {
        raw[i] = static_cast<char>(static_cast<unsigned char>(raw[i]) ^ static_cast<unsigned char>(key[i % key.size()]));
    }
    return QString::fromUtf8(raw.toBase64());
}

/**
 * 解密分数。
 * @param encrypted 加密字符串。
 * @return 解密得到的整数，失败返回 0。
 */
int GameWindow::decryptScore(const QString &encrypted) { // NOLINT(readability-convert-member-functions-to-static)
    QByteArray raw = QByteArray::fromBase64(encrypted.toUtf8());
    const QByteArray key = GameConfig::ENCRYPTION_KEY.toUtf8();
    if (!key.isEmpty()) {
        for (int i = 0; i < raw.size(); ++i) {
            raw[i] = static_cast<char>(static_cast<unsigned char>(raw[i]) ^ static_cast<unsigned char>(key[i % key.size()]));
        }
    }
    bool ok = false;
    int v = QString::fromUtf8(raw).toInt(&ok);
    return ok ? v : 0;
}

/**
 * 生成鸟类障碍。
 */
void GameWindow::spawnBird() {
    if (birdImgs.size() < 2 || birdImgs[0].isNull()) return;
    if (score < GameConfig::birdSpawnScoreThreshold) return;

    QPixmap base = birdImgs[0];
    double scale = randomScale(GameConfig::birdScaleMin, GameConfig::birdScaleMax);
    base = base.scaled(static_cast<int>(base.width() * scale), static_cast<int>(base.height() * scale), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    Bird b{};
    b.pix = base;
    b.w = base.width();
    b.h = base.height();
    b.x = width();

    bool high = QRandomGenerator::global()->bounded(2) == 0;
    int centerOffset = high ? GameConfig::birdHeightHigh : GameConfig::birdHeightLow;
    b.y = GameConfig::groundY - centerOffset - b.h / 2 + GameConfig::groundAlignOffset;

    b.animationFrame = 0;
    b.animationCounter = 0;
    birds.push_back(b);
}
