#ifndef DINO_H
#define DINO_H

#include <QObject>
#include <QPixmap>

class QPainter;
class QRect;

/**
 * 简单的恐龙（玩家）类，负责绘制与基本物理（跳跃/下蹲）。
 */
class Dino : public QObject {
    Q_OBJECT
public:
    /**
     * 构造函数。
     * @param parent 所属的 QObject 父对象（可空）。
     */
    explicit Dino(QObject *parent = nullptr);

    /**
     * 在给定的 QPainter 上绘制恐龙。
     * @param painter 用于绘制的画家对象（非空）。
     */
    void draw(QPainter *painter);

    /**
     * 每帧更新恐龙状态（位置、速度等）。
     */
    void update();

    /**
     * 触发跳跃（如果当前不在跳跃中）。
     */
    void jump();

    /**
     * 设置是否下蹲。
     * @param ducking true 表示开始下蹲，false 表示结束下蹲。
     */
    void setDucking(bool ducking);

    /**
     * 标记游戏是否已开始（用于切换 Start 帧到跑/蹲动画）。
     */
    void setGameStarted(bool started);

    /**
     * 设置死亡状态（绘制死亡贴图）。
     */
    void setDead(bool dead);

    /**
     * 返回用于碰撞检测的包围矩形（根据是否下蹲返回不同尺寸）。
     */
    [[nodiscard]] QRect boundingRect() const; // 用于碰撞检测（后续使用）

    /**
     * 重置恐龙到初始位置与状态。
     */
    void reset();
private:
    QPixmap dinoImg;
    QPixmap duckImg;
    QPixmap dinoImg2;
    QPixmap duckImg2;
    QPixmap deadImg;
    QPixmap startImg;
    QPixmap jumpImg;
    int x, y;
    int vy; // 垂直速度
    bool isJumping;
    bool isDucking;
    bool isDead;
    bool hasStarted;
    bool animToggle;
    int animCounter;
    int groundY;
    const int jumpSpeed = -16;
    const int gravity = 1;
};

#endif // DINO_H
