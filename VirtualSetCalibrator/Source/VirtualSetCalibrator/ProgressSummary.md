# VirtualSetCalibrator 进度总结

更新时间：2026-03-26

## 项目目标
在虚拟制作语境下，提供一条稳定、可迁移、可封装的离线单帧流程：

- 导入图像
- 加载深度模型
- 执行 NNE 深度推理
- 生成并导出 `xyz` 点云

## 当前有效范围

### 1. 核心流程
- 固定相机模式：`iPhone 14 Pro 1x`
- 固定内参与 `focal_px`
- 单张图像导入
- NNE CPU 深度推理
- 相机坐标点云生成与导出

### 2. 可选后处理
- 保留单平面几何后处理
- 支持平面检测
- 支持 Manhattan 约束
- 支持基于主平面的点云投影修正
- 输出仍以相机坐标点云为准

## 本次重构结论

### 1. 已删除的开发期残留
- `NNE Smoke Test` 面板按钮
- `检查模型文件` 面板按钮
- `微调 ICP` 占位按钮
- `LED Detect / Pose` 占位流程
- 基于占位 Pose 的世界坐标点云导出
- `ply` 点云导出链路
- 与上述功能绑定的测试日志和状态展示

### 2. 新的源码结构
- `Public/Core`：基础类型定义
- `Public/Inference` / `Private/Inference`：NNE 深度推理
- `Public/PointCloud` / `Private/PointCloud`：点云生成、`xyz` 导出、几何后处理
- `Public/Panel` / `Private/Panel`：面板 UI、流程执行、视图状态

### 3. 推理层变化
- `VSCalibratorNNEHelper` 收敛为推理流程编排入口
- 模型装载与 runtime 解析拆分到 `VSCalibratorNNEModelUtils`
- tensor shape 解析、图像解码、letterbox 预处理拆分到 `VSCalibratorNNETensorUtils`
- 推理层输出语句统一为英语

### 4. 点云层变化
- `VSCalibratorPointCloudUtils` 仅保留深度图反投影与 `xyz` 导出
- 几何后处理独立拆分到 `VSCalibratorGeometryPostprocessUtils`
- 平面检测、Manhattan 轴估计、平面投影不再与基础点云导出混在同一文件中

### 5. 面板层变化
- UI 聚焦到“导入图像 + 生成深度与点云”
- 保留必要的模型目录输入与几何开关
- 日志区改为简洁的运行摘要
- 删除无实际输出的占位结果区块
- 点云导出摘要仅保留 `xyz`

## 当前代码主线

### 面板
- `Public/Panel/VSCalibratorPanel.h`
- `Private/Panel/VSCalibratorPanel.cpp`
- `Private/Panel/VSCalibratorPanelActions.cpp`
- `Private/Panel/VSCalibratorPanelView.cpp`

### 推理
- `Public/Inference/VSCalibratorNNEHelper.h`
- `Private/Inference/VSCalibratorNNEHelper.cpp`
- `Private/Inference/VSCalibratorNNEModelUtils.h`
- `Private/Inference/VSCalibratorNNEModelUtils.cpp`
- `Private/Inference/VSCalibratorNNETensorUtils.h`
- `Private/Inference/VSCalibratorNNETensorUtils.cpp`

### 点云与几何
- `Public/PointCloud/VSCalibratorPointCloudUtils.h`
- `Private/PointCloud/VSCalibratorPointCloudUtils.cpp`
- `Public/PointCloud/VSCalibratorGeometryPostprocessUtils.h`
- `Private/PointCloud/VSCalibratorGeometryPostprocessUtils.cpp`

### 基础类型
- `Public/Core/VSCalibratorTypes.h`

## 当前边界
- 当前仍为 `Editor` 插件模块
- 当前仍使用固定相机内参，不支持 UI 导入内参
- 当前不包含真实的位姿估计链路
- 当前点云导出为相机坐标系结果
- 当前仅导出 `xyz`，不再导出 `ply`

## 当前状态
- 代码结构已从“单大文件面板驱动”收敛为按模块分层
- 推理层已进一步拆分为编排、模型装载、tensor 预处理三个层次
- 点云基础生成与几何后处理职责已经明确分离
- 插件主功能已与测试残留明确切开
- 面板更适合后续打包、预编译和迁移到蓝图类 UE 项目中使用
