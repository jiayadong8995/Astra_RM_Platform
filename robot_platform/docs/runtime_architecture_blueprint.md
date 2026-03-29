# Runtime Architecture Blueprint

这份文档用于固定当前阶段认可的运行时架构草案。

它不讨论遗产代码如何迁移，也不绑定当前仓库里某个具体实现。
它只回答四个问题：

1. 我们到底想解决什么问题
2. 运行时应该分成哪几层
3. 正式数据流应该怎么走
4. 各层之间应该用什么通信方式

---

## 1. 设计目标

当前运行时架构的设计必须满足下面几个约束：

1. 满足开闭原则
   - 新增设备、新增模式、新增控制器、新增后端时，不应频繁修改主控制链
2. 解耦设备与应用
   - 应用层不直接依赖驱动、句柄、板级细节
3. 解耦业务与通用能力
   - 业务模式与机器人专属控制逻辑，不应直接混入公共算法库
4. 支持 `hw` 与 `sitl`
   - 核心控制链尽量共用，环境差异收敛在设备与后端适配层
5. 控制主链保持清晰
   - 不再围绕 legacy task 文件组织架构
   - 不把“全量发布订阅”误当成系统分层

一句话：

当前架构的目标是让主控制链稳定、边界清晰、扩展点明确，而不是把旧代码搬进新目录。

---

## 2. 顶层架构

当前建议的顶层结构如下：

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

其中：

- `generated`
  - CubeMX 生成资产
- `bsp`
  - 板级支持与底层后端实现
- `device`
  - 统一设备语义
- `control`
  - 状态形成与控制求解主链
- `app`
  - 模式管理、任务组织、业务装配
- `sim`
  - 仿真后端与验证适配

注意：

- 不单独引入一个厚重的 `runtime_service` 层作为长期目标
- 不把 `module` 作为机器人专属控制逻辑的归属层
- 通用算法原件收敛到 `runtime/module`

---

## 3. 分层职责

## 3.1 `runtime/generated`

职责：

- 启动代码
- 时钟初始化
- HAL 初始化
- CubeMX 生成的外设初始化

原则：

- 不承接人工业务逻辑
- 不承接控制策略

## 3.2 `runtime/bsp`

职责：

- 板级支持
- 底层总线访问
- 板级后端适配

建议结构：

```text
runtime/bsp/
  boards/
  backends/
```

其中：

- `boards/`
  - 真实硬件板级支持
- `backends/`
  - `sitl` 等后端底层桩实现

原则：

- 只负责底层访问能力
- 不形成业务状态
- 不承接控制求解

## 3.3 `runtime/device`

职责：

- 提供统一设备语义
- 屏蔽驱动和硬件差异

当前建议的设备类型：

- `imu`
- `remote`
- `actuator`

注意：

- `actuator` 是长期语义名
- 当前第一版只实现 `motor`
- 后续可以在不修改主架构的前提下扩展其他执行器
- 统一的是设备接口，不是 concrete adapter 文件
- `hw` 与 `sitl` 的设备实现必须按 backend 分离

建议结构：

```text
runtime/device/
  device_types.h
  imu/
  remote/
  actuator/
    actuator_device.h
    motor/
```

职责边界：

- 上层看到的是设备能力
- 上层不应该直接碰：
  - `FDCAN_HandleTypeDef`
  - UART DMA buffer
  - 具体寄存器和驱动私有结构

实现原则：

- 统一 `device` 接口
- 分离 `hw/sitl` concrete adapter
- 不允许一份设备实现同时硬依赖真实板级驱动和仿真 backend

## 3.4 `runtime/control`

职责：

- 正式状态形成
- 观测与状态整理
- 控制求解
- 安全约束与输出裁剪
- 执行命令映射

这是系统的核心主链层。

它不负责：

- 业务模式管理
- 板级初始化
- 具体驱动访问

建议结构：

```text
runtime/control/
  contracts/
  state/
  controllers/
  constraints/
  execution/
```

各子目录职责：

- `contracts/`
  - 正式边界数据定义
- `state/`
  - 状态形成与观测整理
- `controllers/`
  - 机器人专属控制组合逻辑
- `constraints/`
  - 限幅、安全、保护
- `execution/`
  - `ActuatorCommand -> DeviceCommand` 映射

通用算法、数学工具和控制小原件统一留在 `runtime/module`，不再和 `control` 重叠。
- 基础滤波器

这些内容统一留在 `runtime/module`，不再在 `control` 内重复设一层目录。

## 3.5 `runtime/app`

职责：

- 启动和任务组织
- 生命周期管理
- 模式管理

## 4. 阶段交付标准

当前这轮架构重构不是无限进行，达到下面几条就应进入“收尾与构建恢复”：

1. `device` 形成稳定的 profile + nodes 模型
2. `control` 以 contracts 作为唯一正式外部边界
3. `app` 退回业务装配层
4. 旧主路径对象彻底退出
5. 文档口径与代码一致

到这一步，后续工作应从“继续改架构”切换到“恢复构建、补验证、开始功能迁移”。
- 机器人意图生成
- 业务装配

它回答的问题是：

- 机器人要做什么
- 系统怎么组织运行

它不该负责：

- 传感器原始数据整理
- 观测量形成
- 控制求解
- 执行器命令翻译
- 硬件句柄访问

建议结构：

```text
runtime/app/
  <robot>/
    bringup/
    modes/
    orchestration/
    config/
    interfaces/
```

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
