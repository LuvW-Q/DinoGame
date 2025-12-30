#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
#include <QTimer>
#include <vector>
#include "dino.h"

class QMouseEvent;

class GameWindow : public QWidget {
    Q_OBJECT
public:
    /**
     * 构造并初始化游戏窗口（设置大小、计时器等）。
     * @param parent 父 QWidget（可空）。
     */
    explicit GameWindow(QWidget *parent = nullptr);

    /**
     * 析构函数，清理资源。
     */
    ~GameWindow() override;
protected:
    /**
     * 绘制窗口内容（背景、地面、恐龙、提示文字）。
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * 处理按键按下事件（用于开始游戏、跳跃与下蹲）。
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * 处理按键释放事件（用于结束下蹲）。
     */
    void keyReleaseEvent(QKeyEvent *event) override;

    /**
     * 处理鼠标点击事件（用于点击重开按钮）。
     */
    void mousePressEvent(QMouseEvent *event) override;
private slots:
    /**
     * 游戏主循环：每帧更新世界并重绘。
     */
    void gameLoop();
private:
    struct Cactus {
        QPixmap pix;
        int x;
        int y;
        int w;
        int h;
    };

    struct Bird {
        QPixmap pix;
        int x;
        int y;
        int w;
        int h;
        int animationFrame; // 当前动画帧（0 或 1）
        int animationCounter; // 动画计数器
    };

    struct Cloud {
        int x;
        int y;
    };

    void resetGame();
    void spawnObstacle(); // 统一的障碍物生成方法
    void spawnCactus();
    void spawnBird();
    void updateCacti();
    void updateBirds();
    bool checkCollision() const;
    double randomScale(double min, double max) const;
    void loadHighScore();
    void saveHighScore();
    QString encryptScore(int score);
    int decryptScore(const QString &encrypted);
    void updateDayNightCycle();
    QColor interpolateColor(const QColor &from, const QColor &to, float alpha) const;
    float getCloudAlpha() const;

    QTimer *timer;
    Dino *dino;

    // game state
    bool isRunning; // 游戏是否在运行（开始后为 true）
    bool isGameOver; // 游戏是否结束
    int groundOffset; // 地面滚动偏移
    int speed; // 游戏速度（像素/帧）
    int score; // 当前分数
    int highScore; // 历史最高分

    // time and day-night cycle
    int gameFrameCount; // 总游戏帧数计数
    bool isNight; // 当前是否为黑夜
    int cyclePosition; // 昼夜周期内的位置（0 到 dayNightCycleFrames）
    float dayNightTransitionAlpha; // 白天-黑夜过渡进度（0.0 - 1.0）
    QColor currentBackgroundColor; // 当前背景颜色

    // obstacles
    std::vector<Cactus> cacti;
    std::vector<Bird> birds;
    std::vector<Cloud> clouds;
    int spawnCooldown; // 帧计数器，<=0 时生成
    int spawnIntervalMin;
    int spawnIntervalMax;

    // assets
    QPixmap trackImg;
    QPixmap gameOverImg;
    QPixmap resetImg;
    QPixmap cloudImg;
    std::vector<QPixmap> smallCactusImgs;
    std::vector<QPixmap> largeCactusImgs;
    std::vector<QPixmap> birdImgs; // 鸟类的两帧动画图片

    QRect resetRect; // 记录重开按钮的绘制区域
};

#endif // GAMEWINDOW_H
