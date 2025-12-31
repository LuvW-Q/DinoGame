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
     * @param parent Qt 对象父指针，可为空。
     */
    explicit Dino(QObject *parent = nullptr);

    /**
     * 绘制恐龙，根据当前状态选择帧。
     * @param painter 画家对象，外部创建并传入。
     */
    void draw(QPainter *painter);

    /**
     * 每帧更新恐龙位置与动画。
     */
    void update();

    /**
     * 触发跳跃（仅当未死亡且不在跳跃中）。
     */
    void jump();

    /**
     * 设置下蹲状态。
     * @param ducking true 表示按下下蹲，false 表示松开。
     */
    void setDucking(bool ducking);

    /**
     * 设置游戏开始状态（用于切换起始帧）。
     * @param started true 表示已开始。
     */
    void setGameStarted(bool started);

    /**
     * 设置死亡状态，切换死亡贴图。
     * @param dead true 表示死亡。
     */
    void setDead(bool dead);

    /**
     * 返回当前用于碰撞检测的包围矩形（含收缩边距）。
     */
    [[nodiscard]] QRect boundingRect() const;

    /**
     * 获取当前绘制帧与目标矩形，用于像素级碰撞。
     * @param outPixmap 输出：当前贴图。
     * @param outRect 输出：当前绘制矩形（屏幕坐标）。
     */
    void currentFrame(QPixmap &outPixmap, QRect &outRect) const;

    /**
     * 重置恐龙状态到初始值。
     */
    void reset();
private:
    QPixmap dinoImg;   // 站立帧 1
    QPixmap duckImg;   // 下蹲帧 1
    QPixmap dinoImg2;  // 站立帧 2
    QPixmap duckImg2;  // 下蹲帧 2
    QPixmap deadImg;   // 死亡帧
    QPixmap startImg;  // 起始静止帧
    QPixmap jumpImg;   // 跳跃帧
    int x, y;          // 左上角坐标
    int vy;            // 垂直速度
    bool isJumping;    // 是否正在跳跃
    bool isDucking;    // 是否正在下蹲
    bool isDead;       // 是否死亡
    bool hasStarted;   // 是否已开始游戏
    bool animToggle;   // 动画帧切换标志
    int animCounter;   // 动画计数器
    int groundY;       // 地面基准高度
    const int jumpSpeed = -16; // 起跳初速度
    const int gravity = 1;     // 重力加速度
};

#endif // DINO_H
