#include "gamewindow.h"
#include <QPainter>
#include <QKeyEvent>
#include <QFont>

/**
 * 构造：设置窗口、计时器并初始化游戏状态。
 */
GameWindow::GameWindow(QWidget *parent) : QWidget(parent) {
    setFixedSize(800, 300);
    dino = new Dino(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWindow::gameLoop);
    timer->start(16); // ~60 FPS
    setFocusPolicy(Qt::StrongFocus);

    // init game state
    isRunning = false; // start screen
    groundOffset = 0;
    speed = 6; // initial ground speed
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

    // draw ground as a repeating line/rect for now
    int groundY = 264;
    painter.setBrush(QColor(83, 83, 83));
    painter.setPen(Qt::NoPen);
    // draw two tiles to ensure seamless scrolling
    int tileW = 40;
    int xStart = - (groundOffset % tileW);
    for (int x = xStart; x < width(); x += tileW) {
        painter.drawRect(x, groundY, tileW, height() - groundY);
    }

    // draw dino
    dino->draw(&painter);

    if (!isRunning) {
        // start screen overlay
        painter.setPen(Qt::black);
        QFont f = painter.font();
        f.setPointSize(18);
        painter.setFont(f);
        painter.drawText(rect(), Qt::AlignCenter, "Press SPACE to Start");
    }
}

/**
 * 处理按键按下：空格用于开始/跳跃，下键用于下蹲。
 */
void GameWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space) {
        if (!isRunning) {
            isRunning = true; // start the game
        } else {
            dino->jump();
        }
    } else if (event->key() == Qt::Key_Down) {
        dino->setDucking(true);
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
    if (isRunning) {
        // update world
        groundOffset += speed;
        dino->update();
    }
    update();
}
