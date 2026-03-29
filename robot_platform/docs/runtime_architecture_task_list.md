# Runtime Architecture Task List

这份文档只保留当前仍未完成的运行时任务，不再重复早期架构定稿阶段的内容。

## 当前任务

### T1. 收紧 `device` backend 装配

目标：

- 让 `device` 通过明确 backend profile 装配，而不是单一默认绑定器

当前要求：

- `hw` 与 `sitl` 装配入口分离
- `device_layer` 只暴露稳定 layer 接口
- backend 绑定作为内部实现细节存在

### T2. 清理 `control/internal`

目标：

- 让 `control/internal` 变成可维护的内部模型，而不是 legacy 状态结构集合

当前要求：

- 清理旧字段命名
- 减少 controller 对 legacy 语义的依赖
- 让正式 contracts 成为外部真相

### T3. 继续收缩过渡消息对象

目标：

- 继续减少 `app/control` 之间的项目私有消息对象

当前重点：

- `ins_data`
- `chassis_observe`

要求：

- 这些对象若保留，只允许作为内部中间消息
- 不再作为 app 配置层对象

### T4. 文档瘦身

目标：

- 让文档只表达当前主架构和当前待办

要求：

- 删掉阶段性、过程性、已过时口径
- 正式文档与当前代码状态一致

## 当前禁区

- 不再新增新的 runtime 中间层
- 不再把设备细节暴露回 `app`
- 不再以 legacy task 文件名驱动架构决策
- 不再为兼容旧对象保留主路径重复结构

## 当前交付节点

这轮运行时架构重构，达到下面五条时视为阶段交付完成：

1. `device` 稳定
   - 只保留 `device_layer + device_profile + device nodes`
   - 不再有驱动脏头或板级细节泄漏到设备语义层
2. `control` 稳定
   - 对外只围绕 `RobotState / RobotIntent / ActuatorCommand / DeviceFeedback`
   - `control/internal` 不再残留明显 legacy 命名和旧结构外泄
3. `app` 稳定
   - 只保留 bringup、intent/mode、orchestration
   - 不再承接设备细节和控制运行态
4. 文档稳定
   - 仓库只保留当前有效架构口径
   - 不再存在会误导实现的旧阶段文档
5. 构建恢复
   - 在结构收口后，`hw_elf` 和 `sitl` 至少恢复到可构建
