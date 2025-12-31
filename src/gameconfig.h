#ifndef GAMECONFIG_H
#define GAMECONFIG_H

namespace GameConfig {
// 加密配置
const QString ENCRYPTION_KEY = "ee7d5971-c06e-485d-8b09-abae73aef66d";
const QString HIGHSCORE_FILE = "highscore.dat";
// 窗口与地面
constexpr int windowWidth = 800;
constexpr int windowHeight = 300;
constexpr int groundY = 264;       // 地面基准线（用于障碍物/赛道）
constexpr int groundAlignOffset = 5; // 赛道贴图与地面对齐微调
constexpr int dinoGroundY = 220;    // 恐龙站立高度（dino.cpp 使用）

// 速度配置
constexpr int gameSpeed = 6;        // 地面/障碍物移动速度（像素/帧）
constexpr int scorePerFrame = 1;    // 记分速度（每帧增加的分数）

// 障碍物
constexpr int spawnIntervalMin = 70;  // 生成间隔下限（帧）
constexpr int spawnIntervalMax = 130; // 生成间隔上限（帧）
// 随机缩放范围（更小的障碍物）
constexpr double cactusScaleSmallMin = 0.72;
constexpr double cactusScaleSmallMax = 0.9;
constexpr double cactusScaleLargeMin = 0.58;
constexpr double cactusScaleLargeMax = 0.75;
constexpr double cactusScaleLarge3Cap = 0.62; // 特别压缩 LargeCactus3 宽度

// 无齿翼龙（鸟类）障碍物
// birdHeight* 现表示：鸟的“中心”距地面的像素距离，越小越贴近地面
constexpr int birdSpawnScoreThreshold = 500; // 2000 分开始生成
constexpr double birdScaleMin = 0.35;
constexpr double birdScaleMax = 0.5;  // 鸟类缩放较小，让其更容易躲避
constexpr int birdHeightLow = 20;   // 低飞行高度（相对地面）
constexpr int birdHeightHigh = 60;  // 高飞行高度（相对地面）
constexpr int birdAnimationFrames = 12; // 鸟类飞行动画帧间隔
constexpr int birdSpawnProbability = 30; // 在障碍物生成时，鸟出现的概率百分比（0-100）

// 恐龙尺寸
constexpr int dinoWidth = 44;
constexpr int dinoHeight = 44;
constexpr int dinoDuckHeight = 24;
constexpr int dinoDuckYOffset = 20;
constexpr int collisionInsetX = 4; // 碰撞矩形水平方向向内收缩像素（收缩更小避免漏判）
constexpr int collisionInsetY = 4; // 碰撞矩形竖直方向向内收缩像素（收缩更小避免漏判）

// 背景云朵
constexpr int cloudCount = 5;
constexpr int cloudYMin = 40;
constexpr int cloudYMax = 140;
constexpr int cloudSpeedDivisor = 3; // 云速 = 地速 / cloudSpeedDivisor

// 时间与场景切换
constexpr int dayNightCycleFrames = 3000; // 一个完整的昼夜周期帧数（约5分钟，60FPS）
constexpr int dayDurationFrames = 1500;    // 白天持续帧数
constexpr int nightDurationFrames = 1100;  // 黑夜持续帧数（不含过渡期）
constexpr int transitionFrames = 200;     // 过渡期帧数（白天->黑夜 或 黑夜->白天）
}

#endif // GAMECONFIG_H

