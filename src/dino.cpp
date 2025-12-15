#include "dino.h"
#include <QPainter>
#include <QDir>

/**
 * 构造函数：加载图片并初始化位置与状态。
 */
Dino::Dino(QObject *parent) : QObject(parent), x(50), y(0), vy(0), isJumping(false), isDucking(false) {
    dinoImg = QPixmap(":/dino/DinoRun1.png"); // 使用 Qt 资源系统
    duckImg = QPixmap(":/dino/DinoDuck1.png");
    groundY = 220; // 地面高度
    y = groundY;
}

/**
 * 绘制恐龙，根据是否下蹲选择不同的目标矩形。
 */
void Dino::draw(QPainter *painter) {
    if (isDucking) {
        QRect dest(x, y + 20, 44, 24);
        painter->drawPixmap(dest, duckImg);
    } else {
        QRect dest(x, y, 44, 44);
        painter->drawPixmap(dest, dinoImg);
    }
}

/**
 * 每帧更新位置与速度；处理着陆逻辑。
 */
void Dino::update() {
    if (isJumping) {
        y += vy;
        vy += gravity;
        if (y >= groundY) {
            y = groundY;
            isJumping = false;
            vy = 0;
        }
    }
}

/**
 * 触发跳跃（仅当不在跳跃时才会生效）。
 */
void Dino::jump() {
    if (!isJumping) {
        isJumping = true;
        vy = jumpSpeed;
    }
}

/**
 * 设置下蹲状态。
 */
void Dino::setDucking(bool ducking) {
    isDucking = ducking;
}

/**
 * 返回用于碰撞检测的包围矩形。
 */
QRect Dino::boundingRect() const {
    if (isDucking) {
        return {x, y + 20, 44, 24};
    }
    return {x, y, 44, 44};
}
