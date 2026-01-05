# 游戏界面与数据结构文档

## 关键常量（来自 gameconfig.h）

`GameConfig` 命名空间定义了游戏的关键常量，以下分组列出：

- **加密配置**：
  - `ENCRYPTION_KEY`：高分加密密钥（字符串）。
  - `HIGHSCORE_FILE`：高分存储文件名（字符串）。

- **窗口与地面**：
  - `windowWidth`：窗口宽度（800 像素）。
  - `windowHeight`：窗口高度（300 像素）。
  - `groundY`：地面基准线（264 像素，用于障碍物/赛道）。
  - `groundAlignOffset`：赛道贴图与地面对齐微调（5 像素）。
  - `dinoGroundY`：恐龙站立高度（220 像素）。

- **速度与计分**：
  - `gameSpeed`：地面/障碍物移动速度（6 像素/帧）。
  - `scorePerFrame`：记分速度（1 分/帧）。

- **障碍物生成与缩放**：
  - `spawnIntervalMin`：生成间隔下限（70 帧）。
  - `spawnIntervalMax`：生成间隔上限（130 帧）。
  - `cactusScaleSmallMin`：小型仙人掌缩放下限（0.72）。
  - `cactusScaleSmallMax`：小型仙人掌缩放上限（0.9）。
  - `cactusScaleLargeMin`：大型仙人掌缩放下限（0.58）。
  - `cactusScaleLargeMax`：大型仙人掌缩放上限（0.75）。
  - `cactusScaleLarge3Cap`：特别压缩 LargeCactus3 宽度（0.62）。

- **无齿翼龙（鸟类）**：
  - `birdSpawnScoreThreshold`：达到此分数后生成鸟（500 分）。
  - `birdScaleMin`：鸟缩放下限（0.35）。
  - `birdScaleMax`：鸟缩放上限（0.5）。
  - `birdHeightLow`：低飞行高度（20 像素，中心距地）。
  - `birdHeightHigh`：高飞行高度（60 像素，中心距地）。
  - `birdAnimationFrames`：鸟动画帧切换间隔（12 帧）。
  - `birdSpawnProbability`：障碍生成时鸟出现概率（30%）。

- **恐龙尺寸与碰撞**：
  - `dinoWidth`：恐龙站立状态宽度（44 像素）。
  - `dinoHeight`：恐龙站立状态高度（44 像素）。
  - `dinoDuckHeight`：下蹲状态高度（24 像素）。
  - `dinoDuckYOffset`：下蹲时 Y 轴偏移（20 像素）。
  - `collisionInsetX`：碰撞矩形水平方向向内收缩像素（4 像素）。
  - `collisionInsetY`：碰撞矩形竖直方向向内收缩像素（4 像素）。

- **背景云朵**：
  - `cloudCount`：同屏云朵数量（5 个）。
  - `cloudYMin`：云朵 Y 最小值（40 像素）。
  - `cloudYMax`：云朵 Y 最大值（140 像素）。
  - `cloudSpeedDivisor`：云速 = 地速 / cloudSpeedDivisor（3）。

- **时间与场景切换（昼夜）**：
  - `dayNightCycleFrames`：一个完整昼夜周期帧数（3000 帧，约5分钟，60FPS）。
  - `dayDurationFrames`：白天持续帧数（1500 帧）。
  - `nightDurationFrames`：黑夜持续帧数（1100 帧，不含过渡）。
  - `transitionFrames`：过渡期帧数（200 帧，昼->夜或夜->昼）。

## 关键数据结构

代码中定义了以下结构体（struct）用于表示游戏元素：

- **Cloud**：表示云朵的位置。
  - `int x`：X 坐标。
  - `int y`：Y 坐标。

- **Cactus**：表示仙人掌障碍物。
  - `QPixmap pix`：贴图。
  - `int x, y, w, h`：位置和尺寸。

- **Bird**：表示鸟类障碍物。
  - `QPixmap pix`：贴图。
  - `int x, y, w, h`：位置和尺寸。
  - `int animationFrame`：当前动画帧（0 或 1）。
  - `int animationCounter`：动画计数器。

- **Dino**：恐龙类（定义在其他文件，如 `dino.h`），管理恐龙状态、绘制和行为。

## 成员变量（类内变量）

`GameWindow` 类包含以下关键成员变量（无全局变量，所有变量均为类成员）：

- **游戏状态**：
  - `bool isRunning`：游戏是否运行中。
  - `bool isGameOver`：游戏是否结束。
  - `double runtimeSeconds`：运行时间（秒）。
  - `int score, highScore`：当前分数和最高分。
  - `double scoreAccumulator`：分数累积器。
  - `int gameFrameCount`：游戏帧计数。

- **速度和偏移**：
  - `double groundOffset`：地面滚动偏移。
  - `double speedPxPerSec, baseSpeedPxPerSec`：当前和基础速度（像素/秒）。

- **昼夜周期**：
  - `bool isNight`：是否夜晚。
  - `int cyclePosition`：周期位置。
  - `float dayNightTransitionAlpha`：过渡透明度。
  - `QColor currentBackgroundColor`：当前背景色。

- **障碍生成**：
  - `double spawnCooldownMs, spawnIntervalMinMs, spawnIntervalMaxMs`：生成冷却和间隔（毫秒）。

- **容器**：
  - `std::vector<Cloud> clouds`：云朵列表。
  - `std::vector<Cactus> cacti`：仙人掌列表。
  - `std::vector<Bird> birds`：鸟类列表。

- **资源**：
  - `QPixmap trackImg, gameOverImg, resetImg, cloudImg`：赛道、游戏结束、重置、云朵贴图。
  - `std::vector<QPixmap> smallCactusImgs, largeCactusImgs, birdImgs`：仙人掌和鸟类贴图列表。

- **其他**：
  - `QTimer* timer`：游戏循环定时器。
  - `Dino* dino`：恐龙对象。
  - `QRect resetRect`：重置按钮矩形。
  - `QElapsedTimer frameTimer`：帧计时器。
  - `qint64 lastFrameMs`：上一帧时间戳。
