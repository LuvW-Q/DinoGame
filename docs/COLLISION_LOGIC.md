# 碰撞检测逻辑说明

本文描述当前游戏中恐龙与障碍物（仙人掌、鸟）的碰撞检测流程，涵盖粗判与像素级判定实现细节。

## 总览
- 所有碰撞先做矩形粗判，避免不必要的像素级遍历。
- 粗判命中后，对重叠区域做逐像素 alpha 检查（透明度>阈值即视为实像素，当前阈值 80）。
- 仙人掌、鸟两类障碍均使用相同的像素级判定；恐龙当前帧贴图与目标绘制矩形来自 `Dino::currentFrame`。
- 为减小反锯齿透明边导致的误判，重叠矩形会向内收缩 1 像素；若收缩后宽或高不足以检测，则直接视为无碰撞。

## 关键实现位置
- `src/gamewindow.cpp` → `GameWindow::checkCollision()`：统一的碰撞管线。
- `src/dino.h` / `src/dino.cpp` → `currentFrame(...)`：返回当前恐龙使用的贴图与绘制矩形，保证与视觉一致。
- `src/dino.cpp` → `boundingRect()`：恐龙粗判矩形，含基础 inset 与额外尾部横向收缩（tailInsetX=2）以减少尾巴擦边误判。

## 流程详解（GameWindow::checkCollision）
1. **获取恐龙数据**
   - 通过 `dino->boundingRect()` 获取用于粗判的恐龙矩形（包含 inset 和尾部收缩）。
   - 通过 `dino->currentFrame(dinoPix, dinoDrawRect)` 获得当前帧贴图与绘制矩形。
   - 将恐龙贴图转换为 `QImage ARGB32`：`dinoPix.toImage().convertToFormat(QImage::Format_ARGB32)`，便于逐像素读取。

2. **仙人掌/鸟类碰撞**
   - 遍历障碍：先做粗判 `dinoRect.intersects(cactusRect/birdRect)`，不命中则跳过。
   - 计算重叠矩形 `overlap = dinoDrawRect.intersected(obstacleRect)`；若宽/高不足以在向内收缩 1 像素后仍有效，则直接跳过。
   - 对 `overlap` 向内收缩 1 像素，映射到恐龙、障碍各自贴图坐标。
   - 将障碍 `QPixmap` 转为 `QImage ARGB32`，逐行取 `constScanLine`，若任意像素双方 `alpha > 80`，判定碰撞。

3. **返回值**
   - 任意一次像素重叠即返回 `true`（撞击），否则全流程结束返回 `false`。

## 相关参数
- 粗判矩形收缩：`GameConfig::collisionInsetX`, `collisionInsetY`（目前为 4）。
- 尾部额外收缩：`tailInsetX = 2`（仅影响恐龙宽度粗判）。
- 像素级 alpha 阈值：`alphaThreshold = 80`（过滤透明羽化边）。
- 边缘收缩：overlap 向内收缩 1 像素，避免反锯齿边产生“虚空碰撞”。
- 鸟生成与高度：`birdHeightLow/High` 表示鸟中心距地面的像素距离，`spawnBird()` 计算 `b.y = groundBase - flightY - b.h/2`。

## 性能提示
- 每次检测会将恐龙当前帧和每个命中重叠的障碍贴图转为 `QImage` 进行扫描。若需进一步优化，可：
  - 在生成障碍时缓存其 `QImage`（ARGB32）以避免重复转换。
  - 每帧仅当恐龙动画帧切换时更新一次恐龙帧的 `QImage` 缓存。
  - 如需要，再增加一个开关回退到仅矩形判定（对性能敏感的平台）。

## 参考代码片段
- `src/gamewindow.cpp` 中 `checkCollision()` 粗判 + 像素级判定
- `src/dino.cpp` 中 `currentFrame()` 提供视觉一致的贴图与矩形
- `src/dino.cpp` 中 `boundingRect()` 含尾部额外收缩
