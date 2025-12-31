#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <vector>
#include "dino.h"
#include "gameconfig.h"

class QMouseEvent;

class GameWindow : public QWidget {
    Q_OBJECT
public:
    /**
     * 构造并初始化游戏窗口（加载资源、定时器、初始状态）。
     * @param parent 父 QWidget，可为空。
     */
    explicit GameWindow(QWidget *parent = nullptr);

    /**
     * 析构函数，释放内部资源。
     */
    ~GameWindow() override;
protected:
    /**
     * 绘制窗口内容（背景、地面、云朵、障碍、恐龙、UI）。
     * @param event Qt 绘制事件（未使用）。
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * 处理按键按下（开始/跳跃、下蹲）。
     * @param event 键盘事件。
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * 处理按键释放（结束下蹲）。
     * @param event 键盘事件。
     */
    void keyReleaseEvent(QKeyEvent *event) override;

    /**
     * 处理鼠标点击（点击重开按钮）。
     * @param event 鼠标事件。
     */
    void mousePressEvent(QMouseEvent *event) override;
private slots:
    /**
     * 游戏主循环：每帧更新世界状态并请求重绘。
     */
    void gameLoop();
private:
    struct Cactus {
        QPixmap pix; // 仙人掌贴图
        int x;       // 左上角 X
        int y;       // 左上角 Y
        int w;       // 宽度
        int h;       // 高度
    };

    struct Bird {
        QPixmap pix; // 鸟当前帧贴图
        int x;
        int y;
        int w;
        int h;
        int animationFrame;   // 当前动画帧索引（0/1）
        int animationCounter; // 动画计数器
    };

    struct Cloud {
        int x;
        int y;
    };

    /** 重置游戏到初始状态。 */
    void resetGame();
    /** 障碍生成入口：按分数与概率生成仙人掌或鸟。 */
    void spawnObstacle();
    /** 生成仙人掌障碍。 */
    void spawnCactus();
    /** 生成鸟类障碍。 */
    void spawnBird();
    /** 更新仙人掌位置、生成、清理。 */
    void updateCacti();
    /** 更新鸟类位置、动画、清理。 */
    void updateBirds();
    /**
     * 碰撞检测：矩形粗判 + 像素级 alpha 判定。
     * @return true 表示碰撞发生。
     */
    bool checkCollision() const;
    /**
     * 返回 [min,max] 区间内的随机双精度数。
     * @param min 下限。
     * @param max 上限。
     */
    double randomScale(double min, double max) const;
    /** 加载最高分（本地加密存储）。 */
    void loadHighScore();
    /** 保存最高分（本地加密存储）。 */
    void saveHighScore();
    /** AES/XOR 简化加密分数。 */
    QString encryptScore(int score);
    /** AES/XOR 简化解密分数。 */
    int decryptScore(const QString &encrypted);
    /** 更新昼夜周期状态与背景颜色。 */
    void updateDayNightCycle();
    /** 颜色线性插值。 */
    QColor interpolateColor(const QColor &from, const QColor &to, float alpha) const;
    /** 获取云朵透明度（受昼夜过渡影响）。 */
    float getCloudAlpha() const;

    QTimer *timer; // 帧定时器
    Dino *dino;   // 玩家对象

    // game state
    bool isRunning;      // 游戏是否在运行（开始后为 true）
    bool isGameOver;     // 游戏是否结束
    int groundOffset;    // 地面滚动偏移
    int speed;           // 游戏速度（像素/帧）
    int score;           // 当前分数
    int highScore;       // 历史最高分

    // time and day-night cycle
    int gameFrameCount;      // 总游戏帧数计数
    bool isNight;            // 当前是否黑夜
    int cyclePosition;       // 昼夜周期位置
    float dayNightTransitionAlpha; // 昼夜过渡进度 0-1
    QColor currentBackgroundColor; // 当前背景颜色

    // obstacles
    std::vector<Cactus> cacti;
    std::vector<Bird> birds;
    std::vector<Cloud> clouds;
    int spawnCooldown;   // 帧计数器，<=0 时生成
    int spawnIntervalMin;
    int spawnIntervalMax;

    // assets
    QPixmap trackImg;
    QPixmap gameOverImg;
    QPixmap resetImg;
    QPixmap cloudImg;
    std::vector<QPixmap> smallCactusImgs;
    std::vector<QPixmap> largeCactusImgs;
    std::vector<QPixmap> birdImgs; // 鸟类两帧动画

    QRect resetRect; // 重开按钮绘制区域
};

#endif // GAMEWINDOW_H
