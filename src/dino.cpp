#include "dino.h"
#include "gameconfig.h"
#include <QPainter>
#include <QDir>

/**
 * 构造函数：加载图片并初始化位置与状态。
 */
Dino::Dino(QObject *parent) : QObject(parent), x(50), y(0), vy(0), isJumping(false), isDucking(false), isDead(false), hasStarted(false), animToggle(false), animCounter(0) {
    dinoImg = QPixmap(":/dino/DinoRun1.png"); // 使用 Qt 资源系统
    dinoImg2 = QPixmap(":/dino/DinoRun2.png");
    duckImg = QPixmap(":/dino/DinoDuck1.png");
    duckImg2 = QPixmap(":/dino/DinoDuck2.png");
    deadImg = QPixmap(":/dino/DinoDead.png");
    startImg = QPixmap(":/dino/DinoStart.png");
    jumpImg = QPixmap(":/dino/DinoJump.png");
    groundY = GameConfig::dinoGroundY; // 地面高度
    y = groundY;
}

/**
 * 绘制恐龙，根据状态选择不同的目标矩形。
 */
void Dino::draw(QPainter *painter) {
    QPixmap current;
    QRect dest;

    if (isDead) {
        current = deadImg;
        dest = QRect(x, y, GameConfig::dinoWidth, GameConfig::dinoHeight);
    } else if (!hasStarted) {
        current = startImg;
        dest = QRect(x, y, GameConfig::dinoWidth, GameConfig::dinoHeight);
    } else if (isJumping) {
        current = jumpImg;
        dest = QRect(x, y, GameConfig::dinoWidth, GameConfig::dinoHeight);
    } else if (isDucking) {
        current = animToggle ? duckImg2 : duckImg;
        dest = QRect(x, y + GameConfig::dinoDuckYOffset, GameConfig::dinoWidth, GameConfig::dinoDuckHeight);
    } else {
        current = animToggle ? dinoImg2 : dinoImg;
        dest = QRect(x, y, GameConfig::dinoWidth, GameConfig::dinoHeight);
    }

    painter->drawPixmap(dest, current);
}

/**
 * 每帧更新位置与速度；处理着陆逻辑，并更新动画计数器。
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

    if (!isDead && hasStarted) {
        // 动画切换：简单计数器，每 8 帧切换一次
        animCounter++;
        if (animCounter >= 8) {
            animCounter = 0;
            animToggle = !animToggle;
        }
    }
}

/**
 * 触发跳跃（仅当不在跳跃且未死亡时才会生效）。
 */
void Dino::jump() {
    if (!isJumping && !isDead) {
        isJumping = true;
        vy = jumpSpeed;
    }
}

/**
 * 设置下蹲状态。
 */
void Dino::setDucking(bool ducking) {
    if (!isDead) {
        isDucking = ducking;
    }
}

/**
 * 设置游戏开始状态（用于切换起始帧）。
 * @param started true 表示已开始。
 */
void Dino::setGameStarted(bool started) {
    hasStarted = started;
}

/**
 * 设置死亡状态，切换死亡贴图。
 * @param dead true 表示死亡。
 */
void Dino::setDead(bool dead) {
    isDead = dead;
}

/**
 * 返回当前用于碰撞检测的包围矩形（含收缩边距）。
 */
QRect Dino::boundingRect() const {
    if (isDucking) {
        return {x + GameConfig::collisionInsetX,
                y + GameConfig::dinoDuckYOffset + GameConfig::collisionInsetY,
                GameConfig::dinoWidth - 2 * GameConfig::collisionInsetX,
                GameConfig::dinoDuckHeight - 2 * GameConfig::collisionInsetY};
    }
    return {x + GameConfig::collisionInsetX,
            y + GameConfig::collisionInsetY,
            GameConfig::dinoWidth - 2 * GameConfig::collisionInsetX,
            GameConfig::dinoHeight - 2 * GameConfig::collisionInsetY};
}

/**
 * 重置恐龙状态到初始值。
 */
void Dino::reset() {
    isJumping = false;
    isDucking = false;
    isDead = false;
    hasStarted = false;
    animToggle = false;
    animCounter = 0;
    vy = 0;
    y = groundY;
}

/**
 * 获取当前绘制帧与目标矩形，用于像素级碰撞。
 * @param outPixmap 输出：当前贴图。
 * @param outRect 输出：当前绘制矩形（屏幕坐标）。
 */
void Dino::currentFrame(QPixmap &outPixmap, QRect &outRect) const {
    QPixmap current;
    QRect dest;

    if (isDead) {
        current = deadImg;
        dest = QRect(x, y, GameConfig::dinoWidth, GameConfig::dinoHeight);
    } else if (!hasStarted) {
        current = startImg;
        dest = QRect(x, y, GameConfig::dinoWidth, GameConfig::dinoHeight);
    } else if (isJumping) {
        current = jumpImg;
        dest = QRect(x, y, GameConfig::dinoWidth, GameConfig::dinoHeight);
    } else if (isDucking) {
        current = animToggle ? duckImg2 : duckImg;
        dest = QRect(x, y + GameConfig::dinoDuckYOffset, GameConfig::dinoWidth, GameConfig::dinoDuckHeight);
    } else {
        current = animToggle ? dinoImg2 : dinoImg;
        dest = QRect(x, y, GameConfig::dinoWidth, GameConfig::dinoHeight);
    }

    outPixmap = current;
    outRect = dest;
}
