#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
#include <QTimer>
#include "dino.h"

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
private slots:
    /**
     * 游戏主循环：每帧更新世界并重绘。
     */
    void gameLoop();
private:
    QTimer *timer;
    Dino *dino;

    // game state
    bool isRunning; // 游戏是否在运行（开始后为 true）
    int groundOffset; // 地面滚动偏移
    int speed; // 游戏速度（像素/帧）
};

#endif // GAMEWINDOW_H
