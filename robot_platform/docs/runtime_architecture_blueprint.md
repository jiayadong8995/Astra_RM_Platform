# Runtime Architecture Blueprint

这份文档只固定当前运行时正式架构，不再记录过程性迁移讨论。

## 1. 目标

当前运行时架构需要同时满足：

1. 开闭原则
   - 新增设备、模式、控制器、后端时，不频繁改主链
2. 解耦设备与应用
   - `app` 不直接依赖驱动、句柄、板级细节
3. 解耦业务与通用能力
   - 机器人专属控制逻辑不混进通用算法库
4. 支持 `hw` 与 `sitl`
   - 核心控制链尽量共用，差异收敛在设备 profile 和 backend

## 2. 顶层结构

```text
robot_platform/
  runtime/
    generated/
    bsp/
    device/
    control/
    app/
  sim/
```

分层职责：

- `generated`
  - CubeMX 生成资产和启动代码
- `bsp`
  - 板级支持与底层后端桩
- `device`
  - 统一设备语义，按 profile 装配具体实现
- `control`
  - 状态形成、控制求解、约束、执行映射
- `app`
  - bringup、mode/intent、业务装配
- `sim`
  - 仿真后端和验证接入

## 3. 正式主数据流

```text
DeviceInput
  -> RobotState
  -> RobotIntent
  -> ActuatorCommand
  -> DeviceCommand
  -> Backend
  -> DeviceFeedback
  -> RobotState
```

正式对象是：

- `platform_device_input_t`
- `platform_robot_state_t`
- `platform_robot_intent_t`
- `platform_actuator_command_t`
- `platform_device_command_t`
- `platform_device_feedback_t`

旧对象已经退出主路径，不再作为正式边界。

## 4. 通信原则

- 主控制链使用直接接口和明确结构体
- 轻量 pub-sub 只用于广播和内部中间消息
- 不做“全量 topic 化”架构

## 5. 当前实现落点

### `runtime/device`

当前已经收成：

- `device_layer`
- `device_profile`
- `imu / remote / actuator` 设备节点

要求：

- `hw` 与 `sitl` 通过 profile 装配
- 上层不直接依赖驱动私有头

### `runtime/control`

当前已经收成：

- `contracts/`
- `state/`
- `controllers/`
- `constraints/`
- `execution/`
- `internal/`

要求：

- `contracts` 是唯一正式外部边界
- `internal` 只保留控制内部模型和参数
- 通用算法原件统一留在 `runtime/module`

### `runtime/app`

当前已经收成：

- `app_bringup/`
- `app_intent/`
- `app_io/`
- `app_config/`

要求：

- `app` 只保留任务组织、意图生成、业务装配
- 不再承接控制运行态和设备细节

## 6. 当前交付标准

这一轮架构重构达到下面几条，就视为阶段交付完成：

1. `device`
   - 稳定为 `device_layer + device_profile + device nodes`
2. `control`
   - 以 contracts 作为唯一正式外部边界
3. `app`
   - 退回业务装配层
4. 旧主路径对象
   - 彻底退出正式主链
5. 文档与代码
   - 口径一致
6. 构建
   - `hw_elf` 和 `sitl` 恢复可构建

## 3.6 `sim`

职责：

- 仿真后端
- 项目级适配
- 验证与观测

原则：

- 通过正式边界接入控制链
- 不直接依赖 app 内部实现细节

---

## 4. 正式数据流

当前建议的正式数据流如下：

```text
Device Input
  -> Robot State
  -> Robot Intent
  -> Actuator Command
  -> Device Command
  -> Backend / BSP
  -> Device Feedback
  -> Robot State
```

再展开成更具体的版本：

```text
raw device input
  - imu_sample
  - rc_input
  - device_feedback

        |
        v

robot_state
  - body state
  - wheel state
  - leg state
  - chassis state

        |
        +------------------+
        |                  |
        v                  |
robot_intent               |
  - mode                   |
  - motion target          |
  - jump/recover request   |
        |                  |
        +--------->--------+
                   |
                   v

actuator_command
  - joint target
  - wheel target
  - enable/start flags

                   |
                   v

device_command
  - backend-specific command

                   |
                   v

bsp / hardware / sitl backend

                   |
                   v

device_feedback
  - joint feedback
  - wheel feedback
  - sensor feedback
```

关键约束：

- `ActuatorCommand` 是控制层正式输出
- `DeviceCommand` 才是设备/后端相关命令
- `ActuatorCommand` 不能直接等价于 BSP 调用参数

所以主链应理解为：

`ActuatorCommand -> DeviceCommand -> BSP/Backend`

而不是：

`ActuatorCommand -> BSP`

---

## 5. 正式边界对象

当前建议先固定下面几个核心对象：

- `DeviceInput`
- `RobotState`
- `RobotIntent`
- `ActuatorCommand`
- `DeviceCommand`
- `DeviceFeedback`

其中最关键的是：

### `RobotState`

它应作为系统唯一正式主状态视图。

目标是避免多套平行真相长期并存，例如：

- 某一份 `INS`
- 某一份 `chassis_move`
- 某一份局部观察状态
- 某一份驱动反馈缓存

上层最终应面对统一的 `RobotState`。

### `RobotIntent`

它应作为业务层唯一正式目标视图。

目标是避免业务层到处直接写：

- `start_flag`
- `recover_flag`
- `jump_flag`
- `x_set`
- `v_set`
- `turn_set`
- `leg_set`

这些应被统一收敛为正式意图对象。

---

## 6. 通信原则

当前不建议把整个系统做成“全量发布订阅”。

需要区分：

- 数据流
- 通信机制

它们不是一回事。

### 6.1 主控制闭环

建议：

- 使用直接接口
- 使用明确结构体
- 使用周期调度

类似：

```text
device.read()
  -> state.update()
  -> app.intent.update()
  -> control.solve()
  -> execution.map()
  -> device.write()
```

原因：

- 强实时
- 所有权明确
- 数据依赖强
- 不适合在主链每一段都用异步广播

### 6.2 广播与附属消费

建议：

- 只在状态广播、监控、日志、sim 观测等场景使用轻量 pub-sub

例如：

- `RobotState` 给 sim / logger / ui
- fault / diagnostics 广播

### 6.3 请求式交互

建议：

- 显式调用
- 或 request-response 形式

例如：

- 切换模式
- 执行一次校准
- 触发某个生命周期动作

一句话：

- 主控制链不用全量 pub-sub
- 广播链可用轻量 pub-sub

---

## 7. 开闭原则落点

本架构必须保证：

### 设备扩展

新增：

- 新 IMU
- 新遥控器
- 新执行器

应主要发生在：

- `device/`
- `bsp/backends/`

主控制链不应频繁改动。

### 控制扩展

新增：

- 新状态估计器
- 新控制器
- 新约束器

应主要发生在：

- `control/state/`
- `control/controllers/`
- `control/constraints/`

`app` 和 `device` 不应频繁改动。

### 模式扩展

新增：

- 新模式
- 新业务策略

应主要发生在：

- `app/modes/`
- `app/orchestration/`

底层设备和控制基础设施不应频繁改动。

### 后端扩展

新增：

- 新 sim backend
- 新执行环境

应主要发生在：

- `bsp/backends/`
- `sim/`

控制主链不应频繁改动。

---

## 8. 当前阶段结论

当前认可的核心结论是：

1. 不再围绕 legacy task 组织架构
2. 不再把全量 topic 化当作目标
3. 不再让 `app` 承担状态形成、控制求解和设备 glue
4. `device`、`control`、`app` 是主骨架
5. `ActuatorCommand` 不是直接给 BSP 的数据，而是控制层正式输出
6. 真正落到 BSP 的应是 `DeviceCommand`

一句话总结：

当前项目的正式架构方向应当是：

`generated/bsp -> device -> control -> app -> sim`

其中：

- `device` 负责统一设备语义
- `control` 负责正式控制主链
- `app` 负责模式与装配
- `sim` 负责验证接入

而主数据流应当是：

`DeviceInput -> RobotState -> RobotIntent -> ActuatorCommand -> DeviceCommand -> Backend -> DeviceFeedback`
