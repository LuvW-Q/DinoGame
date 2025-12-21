#ifndef GAMECONFIG_H
#define GAMECONFIG_H

namespace GameConfig {
// 窗口与地面
constexpr int windowWidth = 800;
constexpr int windowHeight = 300;
constexpr int groundY = 264;       // 地面基准线（用于障碍物/赛道）
constexpr int groundAlignOffset = 5; // 赛道贴图与地面对齐微调
constexpr int dinoGroundY = 220;    // 恐龙站立高度（dino.cpp 使用）

// 速度配置
constexpr int gameSpeed = 6;        // 地面/障碍物移动速度（像素/帧）

// 障碍物
constexpr double cactusScale = 0.8; // 障碍物缩放比例
constexpr int spawnIntervalMin = 60; // 生成间隔下限（帧）
constexpr int spawnIntervalMax = 120; // 生成间隔上限（帧）

// 恐龙尺寸
constexpr int dinoWidth = 44;
constexpr int dinoHeight = 44;
constexpr int dinoDuckHeight = 24;
constexpr int dinoDuckYOffset = 20;
constexpr int collisionInsetX = 4; // 碰撞矩形水平方向向内收缩像素
constexpr int collisionInsetY = 8; // 碰撞矩形竖直方向向内收缩像素

// 背景云朵
constexpr int cloudCount = 5;
constexpr int cloudYMin = 40;
constexpr int cloudYMax = 140;
}

#endif // GAMECONFIG_H

