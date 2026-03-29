# Runtime Contracts Draft

这份文档定义运行时主链的正式契约草案。

它不描述当前遗产代码长什么样，而只描述未来重构应围绕哪些正式对象展开。

---

## 1. 契约目标

这些契约用于解决三个问题：

1. 固定主控制链的数据边界
2. 让 `device / control / app / sim` 围绕统一对象协作
3. 避免继续依赖分散、隐式、与具体实现绑定的状态结构

当前正式对象为：

- `DeviceInput`
- `RobotState`
- `RobotIntent`
- `ActuatorCommand`
- `DeviceCommand`
- `DeviceFeedback`

---

## 2. 主链关系

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

说明：

- `RobotState` 是系统唯一正式主状态视图
- `RobotIntent` 是系统唯一正式业务目标视图
- `ActuatorCommand` 是控制层正式输出
- `DeviceCommand` 是设备/后端相关命令

---

## 3. DeviceInput

职责：

- 表达来自设备层的原始输入事实
- 作为状态形成阶段的输入

建议字段：

```text
DeviceInput
  timestamp
  imu_sample
  rc_input
  actuator_feedback_snapshot
```

### 3.1 imu_sample

建议字段：

```text
ImuSample
  accel[3]
  gyro[3]
  temperature
  sample_time
  valid
```

### 3.2 rc_input

建议字段：

```text
RcInput
  channels[]
  switches[]
  mouse
  keyboard
  source
  sample_time
  valid
```

### 3.3 actuator_feedback_snapshot

建议字段：

```text
ActuatorFeedbackSnapshot
  motors[]
  sample_time
  valid
```

---

## 4. RobotState

职责：

- 表达机器人当前状态
- 作为 `app` 和 `control` 共同依赖的正式主状态对象

原则：

- 系统里不再存在多份平行“真相”
- 上层不直接依赖散落的姿态、观测、腿、轮状态结构
- `RobotState` 优先表达“控制真正需要的状态”，而不是设备原始回显
- 每个子状态都必须带清晰语义，避免把临时中间量固化成正式字段

建议结构：

```text
RobotState
  timestamp
  sequence
  body
  chassis
  legs
  wheels
  contact
  health
```

字段约束：

- `timestamp`
  - 单一时间基准
  - 用于整个主链的时间对齐
- `sequence`
  - 单调递增
  - 用于调试和 replay 对齐
- 所有子状态字段默认表达“当前时刻估计状态”
- 不把驱动原始 buffer、板级句柄或协议字段暴露进 `RobotState`

### 4.1 body

```text
BodyState
  roll
  pitch
  yaw
  gyro[3]
  accel[3]
  orientation_valid
```

### 4.2 chassis

```text
ChassisState
  x
  v
  vx
  vy
  yaw_total
  turn_rate
  state_valid
```

说明：

- `x / v` 主要保留当前平衡控制链直接依赖的状态
- `vx / vy` 为后续扩展保留，避免未来重新改主结构

### 4.3 legs

```text
LegState
  left:
    length
    joint_pos[]
    joint_vel[]
    joint_torque_est[]
    leg_angle
    support_phase
  right:
    length
    joint_pos[]
    joint_vel[]
    joint_torque_est[]
    leg_angle
    support_phase
```

说明：

- `leg_angle` 属于控制直接使用的组合状态
- `support_phase` 表达腿部当前接地/摆动等阶段语义

### 4.4 wheels

```text
WheelState
  left:
    speed
    position
    torque_est
    online
  right:
    speed
    position
    torque_est
    online
```

### 4.5 contact

```text
ContactState
  grounded
  left_support
  right_support
  land_confidence
```

### 4.6 health

```text
HealthState
  imu_ok
  remote_ok
  actuator_ok
  state_valid
  degraded_mode
```

### 4.7 RobotState 字段分层原则

`RobotState` 字段应分为三层：

1. 正式控制状态
   - 控制器稳定依赖的状态
2. 组合观测状态
   - 由设备反馈和估计拼装得到
3. 健康与有效性状态
   - 用于判定当前状态是否可信

不应进入 `RobotState` 的内容：

- 板级驱动原始对象
- 临时调试变量
- 某个单独算法内部缓存
- 只服务某一版 legacy 实现的过渡字段

---

## 5. RobotIntent

职责：

- 表达业务层希望机器人完成的目标
- 是 `app` 对 `control` 的正式输出

原则：

- 不再散落为一组松散 flag 和 setpoint
- 用统一对象表达模式和目标
- `RobotIntent` 只表达“机器人想做什么”，不表达“怎么实现”

建议结构：

```text
RobotIntent
  timestamp
  sequence
  mode
  motion_target
  posture_target
  behavior_request
  enable
```

字段约束：

- `RobotIntent` 必须由 `app` 生成
- `control` 可以消费 `RobotIntent`，但不应反向修改其业务语义
- `RobotIntent` 不应包含底层设备命令字段

### 5.1 mode

```text
RobotMode
  idle
  active
  recover
  jump
  safe
```

说明：

- 当前第一版先用单层模式枚举
- 若后续复杂度增加，可扩展为“主模式 + 子模式”，但不改对象角色

### 5.2 motion_target

```text
MotionTarget
  vx
  x
  yaw_rate
  yaw_hold
  velocity_frame
```

说明：

- `velocity_frame` 用于明确速度目标所处坐标系
- `yaw_hold` 用于表达是否锁定朝向

### 5.3 posture_target

```text
PostureTarget
  leg_length
  body_pitch_ref
  stance_height
```

### 5.4 behavior_request

```text
BehaviorRequest
  jump_request
  recover_request
  stand_request
  emergency_stop
```

### 5.5 enable

```text
EnableState
  start
  control_enable
  actuator_enable
```

### 5.6 RobotIntent 边界原则

`RobotIntent` 不应直接承载以下内容：

- 关节力矩目标
- 轮电流目标
- CAN 帧参数
- 某个控制器内部状态机缓存

也就是说：

- `RobotIntent` 是业务目标
- `ActuatorCommand` 才是控制输出

---

## 6. ActuatorCommand

职责：

- 表达控制层输出的正式执行目标
- 仍保持设备无关或弱设备相关

原则：

- 不直接包含 BSP 句柄
- 不直接等于底层发送帧
- 不直接暴露具体驱动调用参数
- 只表达执行器目标，不表达发送实现

建议结构：

```text
ActuatorCommand
  timestamp
  sequence
  enable
  motors
```

字段约束：

- `ActuatorCommand` 允许弱设备语义
- `ActuatorCommand` 不允许具体 backend 协议字段
- 所有执行目标应在此层保持可测试和可 replay

### 6.1 motors

当前第一版只实现电机类执行器，因此建议先抽象为：

```text
MotorCommandSet
  left_leg_joint[]
  right_leg_joint[]
  left_wheel
  right_wheel
```

说明：

- 当前先围绕 motor 实现
- 未来若引入其他 actuator，可扩展 `ActuatorCommand` 顶层而不破坏现有 motor 子结构

### 6.2 单个电机命令

```text
MotorCommand
  control_mode
  torque_target
  velocity_target
  position_target
  current_target
  kp
  kd
  valid
```

说明：

- 不是所有字段同时有效
- 由 `control_mode` 决定实际生效字段
- `kp / kd` 仅在需要阻抗或位置速度混合控制时有效

### 6.3 control_mode 建议枚举

建议统一为：

```text
MotorControlMode
  disabled
  current
  torque
  velocity
  position
  impedance
```

### 6.4 ActuatorCommand 边界原则

`ActuatorCommand` 不应进入以下内容：

- `can_id`
- `fdcan_handle`
- `uart_port`
- `mit packet raw bytes`
- `sitl transport packet`

---

## 7. DeviceCommand

职责：

- 表达发送给具体设备/后端的命令
- 是 `execution` 层的输出

原则：

- 允许与具体执行环境绑定
- 允许按 backend 区分

建议结构：

```text
DeviceCommand
  timestamp
  actuator_packets[]
  backend_meta
```

说明：

- 对硬件 backend，它可能对应 CAN/MIT 等发送语义
- 对 SITL backend，它可能对应仿真协议对象
- `DeviceCommand` 是第一层允许出现 backend-specific 字段的对象

所以：

```text
ActuatorCommand -> DeviceCommand -> BSP/Backend
```

而不是：

```text
ActuatorCommand -> BSP
```

---

## 8. DeviceFeedback

职责：

- 表达设备或后端返回的反馈结果
- 是状态形成层重新进入闭环的输入之一

建议结构：

```text
DeviceFeedback
  timestamp
  motors[]
  sensors[]
  backend_meta
```

### 8.1 motor feedback

```text
MotorFeedback
  id
  kind
  position
  velocity
  torque_est
  temperature
  online
  sample_time
```

### 8.2 DeviceFeedback 约束

- `DeviceFeedback` 可以带设备语义
- `DeviceFeedback` 不应直接等于 `RobotState`
- `DeviceFeedback` 应作为状态形成的输入，而不是业务层直接依赖对象

---

## 9. 对各层的使用约束

## 9.1 device

允许：

- 生成 `DeviceInput`
- 消费 `DeviceCommand`
- 产出 `DeviceFeedback`

不允许：

- 直接形成 `RobotIntent`
- 直接依赖业务模式

## 9.2 control

允许：

- `DeviceInput -> RobotState`
- `RobotState + RobotIntent -> ActuatorCommand`
- `ActuatorCommand -> DeviceCommand`

不允许：

- 依赖 BSP 句柄
- 做业务模式管理

## 9.3 app

允许：

- 消费 `RobotState`
- 生成 `RobotIntent`
- 组织生命周期和任务

不允许：

- 直接访问设备驱动结构
- 直接生成 `DeviceCommand`

## 9.4 sim

允许：

- 通过正式契约接入主链

不允许：

- 直接依赖 app 内部实现细节
- 直接依赖遗产状态结构

---

## 10. 当前阶段待定项

以下内容仍需要下一轮细化：

- `RobotState` 是否需要拆分观测状态与控制状态
- `RobotIntent.mode` 的枚举集合是否需要层次化
- `MotorCommand` 是否拆分 `joint` / `wheel` 两种语义类型
- `DeviceCommand` 的 backend 统一抽象形状
- `DeviceFeedback` 是否需要区分原始反馈和整理后的反馈
- `RobotState.sequence` 与时间基准的正式约束
- `RobotIntent.velocity_frame` 的枚举集合
- `MotorCommandSet` 是否需要统一 actuator 索引映射

---

## 11. 当前阶段结论

当前阶段先固定两个判断：

1. 系统必须围绕正式契约对象迁移，而不是围绕 legacy task 迁移
2. `RobotState`、`RobotIntent`、`ActuatorCommand` 是后续重构最关键的三个主对象

后续所有目录设计和代码迁移，都应优先服从这份契约草案。
