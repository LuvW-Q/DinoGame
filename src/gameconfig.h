#ifndef GAMECONFIG_H
#define GAMECONFIG_H

// 游戏全局配置常量，统一管理尺寸、速度、生成与昼夜参数
namespace GameConfig {
    // 加密配置
    const QString ENCRYPTION_KEY = "ee7d5971-c06e-485d-8b09-abae73aef66d"; // 高分加密密钥
    const QString HIGHSCORE_FILE = "highscore.dat"; // 高分存储文件名

    // 窗口与地面
    constexpr int windowWidth = 800;   // 窗口宽度（像素）
    constexpr int windowHeight = 300;  // 窗口高度（像素）
    constexpr int groundY = 264;       // 地面基准线（用于障碍物/赛道）
    constexpr int groundAlignOffset = 5; // 赛道贴图与地面对齐微调
    constexpr int dinoGroundY = 220;    // 恐龙站立高度（dino.cpp 使用）

    // 速度与计分
    constexpr int gameSpeed = 6;        // 地面/障碍物移动速度（像素/帧）
    constexpr int scorePerFrame = 1;    // 记分速度（每帧增加的分数）

    // 障碍物生成与缩放
    constexpr int spawnIntervalMin = 70;  // 生成间隔下限（帧）
    constexpr int spawnIntervalMax = 130; // 生成间隔上限（帧）
    constexpr double cactusScaleSmallMin = 0.72; // 小型仙人掌缩放下限
    constexpr double cactusScaleSmallMax = 0.9;  // 小型仙人掌缩放上限
    constexpr double cactusScaleLargeMin = 0.58; // 大型仙人掌缩放下限
    constexpr double cactusScaleLargeMax = 0.75; // 大型仙人掌缩放上限
    constexpr double cactusScaleLarge3Cap = 0.62; // 特别压缩 LargeCactus3 宽度

    // 无齿翼龙（鸟类）
    // birdHeight* 表示：鸟的“中心”距地面的像素距离，越小越贴地
    constexpr int birdSpawnScoreThreshold = 500; // 达到此分数后才会生成鸟
    constexpr double birdScaleMin = 0.35;        // 鸟缩放下限
    constexpr double birdScaleMax = 0.5;         // 鸟缩放上限
    constexpr int birdHeightLow = 20;            // 低飞行高度（中心距地）
    constexpr int birdHeightHigh = 60;           // 高飞行高度（中心距地）
    constexpr int birdAnimationFrames = 12;      // 鸟动画帧切换间隔（帧）
    constexpr int birdSpawnProbability = 30;     // 障碍生成时鸟出现概率（%）

    // 恐龙尺寸与碰撞
    constexpr int dinoWidth = 44;          // 恐龙站立状态宽度
    constexpr int dinoHeight = 44;         // 恐龙站立状态高度
    constexpr int dinoDuckHeight = 24;     // 下蹲状态高度
    constexpr int dinoDuckYOffset = 20;    // 下蹲时 Y 轴偏移
    constexpr int collisionInsetX = 4;     // 碰撞矩形水平方向向内收缩像素
    constexpr int collisionInsetY = 4;     // 碰撞矩形竖直方向向内收缩像素

    // 背景云朵
    constexpr int cloudCount = 5;          // 同屏云朵数量
    constexpr int cloudYMin = 40;          // 云朵 Y 最小值
    constexpr int cloudYMax = 140;         // 云朵 Y 最大值
    constexpr int cloudSpeedDivisor = 3;   // 云速 = 地速 / cloudSpeedDivisor

    // 时间与场景切换（昼夜）
    constexpr int dayNightCycleFrames = 3000; // 一个完整昼夜周期帧数（约5分钟，60FPS）
    constexpr int dayDurationFrames = 1500;    // 白天持续帧数
    constexpr int nightDurationFrames = 1100;  // 黑夜持续帧数（不含过渡）
    constexpr int transitionFrames = 200;      // 过渡期帧数（昼->夜或夜->昼）
}

#endif // GAMECONFIG_H

