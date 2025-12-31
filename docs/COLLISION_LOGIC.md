# 碰撞检测逻辑说明

本文描述当前游戏中恐龙与障碍物（仙人掌、鸟）的碰撞检测流程，涵盖粗判与像素级判定实现细节。

## 总览
- 所有碰撞先做矩形粗判，避免不必要的像素级遍历。
- 粗判命中后，对重叠区域做逐像素 alpha 检查（透明度>0 即视为实像素）。
- 仙人掌、鸟两类障碍均使用相同的像素级判定；恐龙当前帧贴图与目标绘制矩形来自 `Dino::currentFrame`。

## 关键实现位置
- `src/gamewindow.cpp` → `GameWindow::checkCollision()`：统一的碰撞管线。
- `src/dino.h` / `src/dino.cpp` → `currentFrame(...)`：返回当前恐龙使用的贴图与绘制矩形，保证与视觉一致。

## 流程详解（GameWindow::checkCollision）
1. **获取恐龙数据**
   - 通过 `dino->boundingRect()` 获取用于粗判的恐龙矩形（包含 inset）。
   - 通过 `dino->currentFrame(dinoPix, dinoDrawRect)` 获得当前帧贴图与绘制矩形。
   - 将恐龙贴图转换为 `QImage ARGB32`：`dinoPix.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied)`，便于逐像素读取。

2. **仙人掌碰撞**
   - 遍历 `cacti`：
     - 粗判：`dinoRect.intersects(cactusRect)` 不命中则跳过。
     - 计算重叠矩形 `overlap = dinoDrawRect.intersected(cactusRect)`；为空跳过。
     - 将 `overlap` 映射到恐龙、仙人掌各自贴图坐标，得到偏移。
     - 将仙人掌 `QPixmap` 转为 `QImage ARGB32`，逐行取 `constScanLine`，若任意像素双方 alpha>0，判定碰撞。

3. **鸟类碰撞**（与仙人掌相同的像素级步骤）
   - 遍历 `birds`：先矩形粗判，再计算 `overlap`，转换鸟贴图为 `QImage ARGB32`，对重叠区域逐像素检查 alpha>0。

4. **返回值**
   - 任意一次像素重叠即返回 `true`（撞击），否则全流程结束返回 `false`。

## 相关参数
- 碰撞矩形收缩量：`GameConfig::collisionInsetX`, `collisionInsetY`（目前为 4，减少漏判）。
- 鸟生成与高度：`birdHeightLow/High` 表示“鸟的中心距地面”的像素距离，`spawnBird()` 计算 `b.y = groundBase - flightY - b.h/2`。

## 性能提示
- 每次检测会将恐龙当前帧和每个命中重叠的障碍贴图转为 `QImage` 进行扫描。若需进一步优化，可：
  - 在生成障碍时缓存其 `QImage`（ARGB32）以避免重复转换。
  - 每帧仅当恐龙动画帧切换时更新一次恐龙帧的 `QImage` 缓存。
  - 如需要，再增加一个开关回退到仅矩形判定（对性能敏感的平台）。

## 参考代码片段
- `src/gamewindow.cpp` 中 `checkCollision()` 粗判 + 像素级判定
- `src/dino.cpp` 中 `currentFrame()` 提供视觉一致的贴图与矩形


