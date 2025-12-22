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

    struct Cloud {
        int x;
        int y;
    };

    void resetGame();
    void spawnCactus();
    void updateCacti();
    bool checkCollision() const;
    double randomScale(double min, double max) const;

    QTimer *timer;
    Dino *dino;

    // game state
    bool isRunning; // 游戏是否在运行（开始后为 true）
    bool isGameOver; // 游戏是否结束
    int groundOffset; // 地面滚动偏移
    int speed; // 游戏速度（像素/帧）

    // obstacles
    std::vector<Cactus> cacti;
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

    QRect resetRect; // 记录重开按钮的绘制区域
};

#endif // GAMEWINDOW_H
