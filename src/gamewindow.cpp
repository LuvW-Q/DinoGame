#include "gamewindow.h"
#include "gameconfig.h"
#include <QPainter>
#include <QKeyEvent>
#include <QFont>
#include <QRandomGenerator>
#include <QMouseEvent>
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
    cactusScale = GameConfig::cactusScale; // 缩小障碍物尺寸

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
        // update world
        groundOffset += speed;
        dino->update();
        updateCacti();
        // move clouds slower for parallax
        for (auto &c : clouds) {
            c.x -= speed / 3; // slower than ground
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
        }
    }
    update();
}

void GameWindow::resetGame() {
    isRunning = false;
    isGameOver = false;
    groundOffset = 0;
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

    // scale down obstacle
    if (cactusScale > 0.0 && cactusScale != 1.0) {
        pix = pix.scaled(pix.width() * cactusScale, pix.height() * cactusScale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

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
        int interval = QRandomGenerator::global()->bounded(spawnIntervalMin, spawnIntervalMax + 1);
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
