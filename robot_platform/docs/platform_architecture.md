# Platform Architecture

这份文档只描述当前有效的平台口径，不保留历史迁移讨论。

## 1. 当前主工程

当前继续演进的工程只有 `robot_platform/`。

仓库中其他目录的定位是：

- `references/legacy/Astra_RM2025_Balance_legacy/`
  历史基线与行为参考，不作为当前架构入口
- `references/`
  外部参考仓库，不作为当前执行文档

## 2. 目标分层

当前运行时主架构固定为：

```text
generated -> bsp -> device -> control -> app -> sim
```

各层职责如下：

- `generated`
  - CubeMX 生成资产
  - 芯片启动、HAL 初始化、外设初始化
- `bsp`
  - 板级访问
  - 硬件与 SITL 的底层后端实现
- `device`
  - 统一设备语义
  - 承接具体驱动与 backend profile 装配
- `control`
  - 状态形成
  - 控制求解
  - 约束
  - 执行命令映射
- `app`
  - 生命周期
  - 模式管理
  - 任务组织
  - 业务装配
- `sim`
  - 仿真接入
  - bridge / backend / validation

## 3. 当前主数据流

当前正式主链固定为：

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

当前正式对象为：

- `platform_rc_input_t`
- `platform_imu_sample_t`
- `platform_device_feedback_t`
- `platform_robot_state_t`
- `platform_robot_intent_t`
- `platform_actuator_command_t`
- `platform_device_command_t`

旧对象已经退出主路径：

- `RC_Data_t`
- `Chassis_Cmd_t`
- `Chassis_State_t`
- `Leg_Output_t`
- `Actuator_Cmd_t`
- `Actuator_Feedback_t`

## 4. 通信原则

当前只接受下面这套通信原则：

- 主控制链使用直接接口和明确结构体
- 广播、观测、调试场景才使用轻量 pub-sub
- 不做“全量 topic 化”

这意味着：

- `ActuatorCommand` 不是 BSP 参数
- `ActuatorCommand -> DeviceCommand -> Backend` 才是正确链路
- `app` 不再直接碰驱动头和板级句柄

## 5. 当前代码状态

当前已经完成的结构纠偏包括：

- `runtime_service` 已移除
- `runtime/bsp/devices` 已移除
- `runtime/control/primitives` 已并回 `runtime/module`
- `legacy/` 目录已移除
- `app` 主路径已切到 `platform_robot_intent_t`
- `control` 主反馈已切到 `platform_device_feedback_t`
- `execution` 主输出已切到 `platform_actuator_command_t`

## 6. 当前剩余工作

当前后续工作的重点只有四件事：

1. 收紧 `device` 的 backend profile / registration
2. 清理 `control/internal` 的 legacy 内部模型
3. 继续减少 `app/control` 之间的过渡消息对象
4. 持续瘦身文档，避免旧阶段口径回流

## 7. 当前不做的事

当前明确不做：

- 再引入新的 runtime 中间层
- 继续按 legacy task 文件名组织架构
- 把 `app` 重新做成控制与设备 glue 层
- 为兼容旧结构保留大量重复对象
