#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <QString>

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

// 恐龙尺寸
constexpr int dinoWidth = 44;
constexpr int dinoHeight = 44;
constexpr int dinoDuckHeight = 24;
constexpr int dinoDuckYOffset = 20;
constexpr int collisionInsetX = 6; // 碰撞矩形水平方向向内收缩像素
constexpr int collisionInsetY = 10; // 碰撞矩形竖直方向向内收缩像素

// 背景云朵
constexpr int cloudCount = 5;
constexpr int cloudYMin = 40;
constexpr int cloudYMax = 140;
constexpr int cloudSpeedDivisor = 3; // 云速 = 地速 / cloudSpeedDivisor
}

#endif // GAMECONFIG_H

