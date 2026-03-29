# Runtime Capability Boundaries

这份文档用于定义系统能力边界。

它不按遗产 task 文件名拆系统，而按“系统真正需要哪些能力”来拆。

---

## 1. 目标

这份文档回答三个问题：

1. 系统有哪些核心能力
2. 每类能力应该归属哪一层
3. 哪些能力绝对不应该跨层混放

目的：

- 避免后续继续按 `INS_task`、`chassis_task`、`remote_task` 组织架构
- 避免设备、状态、控制、业务混在一起
- 为迁移顺序和目录设计提供依据

---

## 2. 能力总览

当前建议将系统能力分为五类：

1. 平台与底层访问能力
2. 设备语义能力
3. 状态形成能力
4. 控制求解能力
5. 应用装配能力

---

## 3. 能力边界表

| 能力域 | 主要输入 | 主要输出 | 应归属层 | 不该负责 |
|---|---|---|---|---|
| 平台启动 | 系统入口、生成代码 | 时钟/中断/外设可用 | `generated` | 业务状态、控制逻辑 |
| 板级总线访问 | CAN/UART/SPI/TIM/DMA 请求 | 原始访问能力 | `bsp` | 模式管理、控制求解 |
| IMU 设备语义 | SPI/I2C 驱动输出 | `imu_sample` | `device` | 姿态解算、业务判断 |
| 遥控设备语义 | UART/DBUS 驱动输出 | `rc_input` | `device` | 模式决策、控制求解 |
| 执行器设备语义 | 电机驱动输入输出 | `device_feedback` / 可写命令接口 | `device` | 业务模式、状态估计 |
| 状态形成 | `DeviceInput` | `RobotState` | `control/state` | 模式管理、BSP 访问 |
| 观测整理 | 轮/腿/姿态反馈 | `RobotState` 子状态 | `control/state` | 业务模式、底层句柄 |
| 控制求解 | `RobotState + RobotIntent` | `ActuatorCommand` | `control/controllers` | 底层发送细节 |
| 安全约束 | 控制输出、状态 | 裁剪后的 `ActuatorCommand` | `control/constraints` | 业务模式 |
| 执行映射 | `ActuatorCommand` | `DeviceCommand` | `control/execution` | 模式管理、设备驱动实现 |
| 模式管理 | `rc_input + RobotState` | `RobotIntent.mode` | `app/modes` | 底层驱动、控制算法实现 |
| 意图装配 | 用户输入、模式状态 | `RobotIntent` | `app/orchestration` | BSP 访问、设备反馈解析 |
| 生命周期与任务组织 | 系统配置、运行入口 | 任务/循环启动 | `app/bringup` | 控制算法、设备语义 |
| 仿真接入 | 仿真 backend 输入输出 | 正式边界对象映射 | `sim` | app 内部状态结构 |

---

## 4. 各层应承接的能力

## 4.1 `generated`

只承接：

- 启动
- 时钟
- 中断
- HAL 初始化
- 外设初始化

不承接：

- 人工业务逻辑
- 控制策略
- 设备语义封装

## 4.2 `bsp`

只承接：

- 板级访问
- 总线收发
- 硬件后端与 SITL 底层桩

不承接：

- 机器人状态形成
- 控制输出求解
- 业务模式判断

## 4.3 `device`

只承接：

- `imu`
- `remote`
- `actuator`

这层的目标是：

- 把驱动包装成统一设备语义
- 向上提供设备输入、反馈、命令入口

不承接：

- 观测量整理
- 姿态解算
- 业务模式
- 控制求解

## 4.4 `control`

承接三组核心能力：

1. `state`
2. `controllers`
3. `execution`

### `control/state`

负责：

- 姿态形成
- 状态融合
- 观测量整理
- `RobotState` 输出

### `control/controllers`

负责：

- 平衡控制
- 跳跃控制
- 恢复控制
- 机器人专属控制组合逻辑

### `control/constraints`

负责：

- 限幅
- 保护
- 安全裁剪

### `control/execution`

负责：

- `ActuatorCommand -> DeviceCommand`

`control` 不承接：

- 模式管理
- 任务组织
- 设备驱动访问

## 4.5 `app`

只承接：

- 生命周期
- 模式管理
- 意图生成
- 任务与循环组织
- 业务装配

不承接：

- 设备输入解析
- 观测量形成
- 控制器内部算法
- 底层执行命令映射

## 4.6 `sim`

只承接：

- backend 适配
- 验证
- 观测输出

不承接：

- app 内部结构依赖
- legacy 状态对象依赖

---

## 5. 当前最容易混淆的边界

## 5.1 `device` 和 `control/state`

必须分开：

- `device` 只输出设备语义
- `control/state` 才负责形成 `RobotState`

也就是说：

- IMU 驱动不负责姿态解算
- 电机反馈不直接等于机器人状态

## 5.2 `control/controllers` 和 `app`

必须分开：

- `app` 决定机器人要做什么
- `control/controllers` 决定为了实现这个目标该输出什么控制命令

也就是说：

- 模式管理不是控制器
- 控制器也不是业务编排器

## 5.3 `ActuatorCommand` 和 `DeviceCommand`

必须分开：

- `ActuatorCommand` 是控制层输出
- `DeviceCommand` 是设备/后端命令

所以：

```text
ActuatorCommand -> DeviceCommand -> device/bsp/backend
```

---

## 6. 当前阶段结论

当前架构设计应先围绕以下能力划分推进：

- `device`：统一设备语义
- `control/state`：形成正式状态
- `control/controllers`：生成正式控制输出
- `control/execution`：映射到设备命令
- `app`：管理模式、意图和运行组织

后续任何迁移、目录设计和代码重构，都应优先服从这份能力边界表，而不是服从历史 task 边界。
