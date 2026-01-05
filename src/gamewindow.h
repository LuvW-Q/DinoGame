#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QElapsedTimer>
#include <vector>
#include "dino.h"
#include "gameconfig.h"

class QMouseEvent;

class GameWindow : public QWidget {
    Q_OBJECT
public:
    /**
     * 构造函数：初始化窗口、资源、计时器与状态。
     * @param parent 父 QWidget，可为空。
     */
    explicit GameWindow(QWidget *parent = nullptr);

    /**
     * 析构：释放内部资源。
     */
    ~GameWindow() override;
protected:
    /**
     * 绘制窗口内容（背景、地面、云朵、障碍、恐龙、UI）。
     * @param event Qt 绘制事件（未使用）。
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * 处理按键按下：启动/跳跃或下蹲。
     * @param event 键盘事件。
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * 处理按键释放：结束下蹲。
     * @param event 键盘事件。
     */
    void keyReleaseEvent(QKeyEvent *event) override;

    /**
     * 处理鼠标点击：用于点击重开按钮。
     * @param event 鼠标事件。
     */
    void mousePressEvent(QMouseEvent *event) override;
private slots:
    /**
     * 游戏主循环：基于 deltaTime 更新状态并请求重绘。
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
    /**
     * 障碍生成入口：根据概率生成仙人掌或鸟。
     */
    void spawnObstacle();
    /**
     * 生成仙人掌障碍。
     */
    void spawnCactus();
    /**
     * 生成鸟类障碍。
     */
    void spawnBird();
    /**
     * 更新仙人掌位置、生成与回收。
     * @param deltaSeconds 本帧耗时（秒）。
     */
    void updateCacti(double deltaSeconds);
    /**
     * 更新鸟类位置、动画与回收。
     * @param deltaSeconds 本帧耗时（秒）。
     */
    void updateBirds(double deltaSeconds);
    /**
     * 碰撞检测：先 AABB，再像素级 alpha 判定。
     * @return true 表示发生碰撞。
     */
    [[nodiscard]] bool checkCollision() const;
    /**
     * 像素级 alpha 碰撞检测。
     * @param aPix 贴图 A。
     * @param aRect A 的屏幕矩形。
     * @param bPix 贴图 B。
     * @param bRect B 的屏幕矩形。
     * @return true 表示重叠区域均存在不透明像素。
     */
    [[nodiscard]] bool pixelCollision(const QPixmap &aPix, const QRect &aRect, const QPixmap &bPix, const QRect &bRect) const;
    /**
     * 获取区间[min,max]内的随机双精度数。
     */
    [[nodiscard]] double randomScale(double min, double max) const;
    /**
     * 读取最高分。
     */
    void loadHighScore();
    /**
     * 保存最高分。
     */
    void saveHighScore();
    /**
     * 加密分数。
     */
    QString encryptScore(int score);
    /**
     * 解密分数。
     */
    int decryptScore(const QString &encrypted);
    /**
     * 更新昼夜状态与背景色。
     */
    void updateDayNightCycle();
    /**
     * 线性插值颜色。
     */
    [[nodiscard]] QColor interpolateColor(const QColor &from, const QColor &to, float alpha) const;
    /**
     * 获取云朵透明度。
     */
    [[nodiscard]] float getCloudAlpha() const;
    /**
     * 校验资源并在缺失时退出。
     */
    void validateAssetsOrExit();
    /**
     * 按运行时间计算当前速度。
     */
    [[nodiscard]] double currentSpeedForSeconds(double runSeconds) const;

    QTimer *timer; // 帧定时器
    Dino *dino;   // 玩家对象

    // game state
    bool isRunning;      // 游戏是否在运行（开始后为 true）
    bool isGameOver;     // 游戏是否结束
    double groundOffset; // 地面滚动偏移（像素，支持小数）
    double speedPxPerSec;           // 当前速度（像素/秒）
    double baseSpeedPxPerSec;       // 初始速度（像素/秒）
    double runtimeSeconds;          // 本轮运行时长（秒）
    double scoreAccumulator;        // 记分累加器（用于小数帧）
    int score;           // 当前分数
    int highScore;       // 历史最高分

    // time and day-night cycle
    int gameFrameCount;      // 总游戏帧数计数（基于 60FPS 换算）
    bool isNight;            // 当前是否黑夜
    int cyclePosition;       // 昼夜周期位置
    float dayNightTransitionAlpha; // 昼夜过渡进度 0-1
    QColor currentBackgroundColor; // 当前背景颜色

    // obstacles
    std::vector<Cactus> cacti;
    std::vector<Bird> birds;
    std::vector<Cloud> clouds;
    double spawnCooldownMs;   // 生成计时器（毫秒）
    double spawnIntervalMinMs;
    double spawnIntervalMaxMs;

    // assets
    QPixmap trackImg;
    QPixmap gameOverImg;
    QPixmap resetImg;
    QPixmap cloudImg;
    std::vector<QPixmap> smallCactusImgs;
    std::vector<QPixmap> largeCactusImgs;
    std::vector<QPixmap> birdImgs; // 鸟类两帧动画

    QRect resetRect; // 重开按钮绘制区域
    QElapsedTimer frameTimer; // 用于计算 delta time
    qint64 lastFrameMs = 0;   // 上一帧时间戳
};

#endif // GAMEWINDOW_H
