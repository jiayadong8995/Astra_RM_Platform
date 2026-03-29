# Runtime File Migration Map

这份文档是在目标结构已经明确后，对当前代码做的文件级映射。

它只回答两个问题：

1. 当前文件应该长期留在哪一层
2. 当前文件下一步应该怎么迁

它的用途是让后续拆分不再靠感觉推进。

## 1. 目标结构回顾

当前确认的目标结构是：

```text
runtime/
  generated/
  bsp/
  device/
  runtime_service/
  module/
  app/
```

其中：

- `generated`：生成资产
- `bsp`：板级与具体驱动
- `device`：统一设备节点抽象
- `runtime_service`：运行时服务层
- `module`：算法、控制器、消息总线
- `app`：业务装配、模式管理、任务编排

## 2. 当前映射规则

当前所有文件按四类处理：

1. `Keep`
   当前目录方向正确，保留在该层继续演进
2. `Move`
   当前目录明显不对，应迁到目标层
3. `Transitional`
   过渡资产，短期保留，长期要压薄
4. `Split`
   当前文件职责不纯，后续要拆成多部分再分别迁移

## 3. `runtime/bsp` 映射

### 3.1 板级文件

`Keep`

- `runtime/bsp/boards/stm32h7_ctrl_board/bsp_PWM.*`
- `runtime/bsp/boards/stm32h7_ctrl_board/bsp_dwt.*`
- `runtime/bsp/boards/stm32h7_ctrl_board/bsp_uart.*`
- `runtime/bsp/boards/stm32h7_ctrl_board/can_bsp.*`

说明：

- 这些文件本身就是板级支持
- 它们不应上移到 app 或 module

### 3.2 具体驱动

`Keep`

- `runtime/bsp/devices/bmi088/*`
- `runtime/bsp/devices/dm_motor/*`
- `runtime/bsp/devices/remote_control/*`

说明：

- 这些文件本身就是具体驱动实现
- 它们未来只需要被 `device` 层包装，不应直接搬到 app

### 3.3 SITL 侧 BSP 桩

`Keep`

- `runtime/bsp/sitl/BMI088driver_sitl.c`
- `runtime/bsp/sitl/bsp_PWM_sitl.c`
- `runtime/bsp/sitl/bsp_dwt_sitl.c`
- `runtime/bsp/sitl/can_bsp_sitl.c`
- `runtime/bsp/sitl/dm4310_drv_sitl.c`
- `runtime/bsp/sitl/hal_stub.c`
- `runtime/bsp/sitl/main_sitl.c`

说明：

- 这些仍然属于 BSP/SITL 后端
- 不应和 `sim` 项目适配层混在一起

## 4. `runtime/module` 映射

### 4.1 算法模块

`Keep`

- `runtime/module/algorithm/EKF/*`
- `runtime/module/algorithm/PID/*`
- `runtime/module/algorithm/VMC/*`
- `runtime/module/algorithm/kalman/*`
- `runtime/module/algorithm/mahony/*`

说明：

- 方向对
- 但后续要继续减少对 app runtime 类型和 legacy 头的依赖

### 4.2 控制器与消息总线

`Keep`

- `runtime/module/controller/*`
- `runtime/module/message_center/*`
- `runtime/module/lib/*`

说明：

- 这些是公共模块层
- 后续应服务 `hw` 与 `sitl`

## 5. `runtime/app/balance_chassis` 映射

## 5.1 `app_bringup`

`Keep`

- `runtime/app/balance_chassis/app_bringup/freertos_app.c`
- `runtime/app/balance_chassis/app_bringup/task_registry.c`
- `runtime/app/balance_chassis/app_bringup/task_registry.h`

说明：

- 这些文件承担启动入口和任务注册
- 它们应继续留在 app 层

注意：

- 这里只负责启动和装配
- 不应继续吃进执行器细节或反馈整理

## 5.2 `app_config`

`Keep`

- `runtime/app/balance_chassis/app_config/app_params.h`
- `runtime/app/balance_chassis/app_config/robot_def.h`

`Transitional`

- `runtime/app/balance_chassis/app_config/runtime_state.h`

说明：

- `app_params.h`、`robot_def.h` 的方向是对的
- `runtime_state.h` 当前仍混有较多控制运行态事实，后续要继续瘦身

建议：

- app 配置层长期只保留项目参数、业务边界数据、app 级 profile
- 不长期持有大量 actuator/control runtime 内部状态

## 5.3 `app_io`

### 长期保留在 app 的部分

`Keep`

- `runtime/app/balance_chassis/app_io/topic_contract.h`
- `runtime/app/balance_chassis/app_io/chassis_topics.*`
- `runtime/app/balance_chassis/app_io/remote_topics.*`

说明：

- 这些文件仍属于业务边界 topic 适配

### 不应长期留在 app 的部分

`Move -> runtime/runtime_service/actuator/`

- `runtime/app/balance_chassis/app_io/actuator_topics.c`
- `runtime/app/balance_chassis/app_io/actuator_topics.h`

原因：

- 它们服务的是 actuator command / feedback 运行时链路
- 这不是业务 app I/O，而是执行层运行时 I/O

`Move -> runtime/runtime_service/observe/`

- `runtime/app/balance_chassis/app_io/observe_topics.c`
- `runtime/app/balance_chassis/app_io/observe_topics.h`

原因：

- 它们接的是观测运行时输入输出，不是纯业务边界

`Move -> runtime/runtime_service/sensing/`

- `runtime/app/balance_chassis/app_io/ins_topics.c`
- `runtime/app/balance_chassis/app_io/ins_topics.h`

原因：

- `ins_data` 更像传感器运行时输入
- 长期更适合放在 sensing service，而不是业务 app I/O

## 5.4 `app_flow`

### 可留在 app 的部分

`Keep`

- `runtime/app/balance_chassis/app_flow/remote_orchestration.c`
- `runtime/app/balance_chassis/app_flow/remote_orchestration.h`
- `runtime/app/balance_chassis/app_flow/remote_runtime.h`
- `runtime/app/balance_chassis/app_flow/chassis_orchestration.c`
- `runtime/app/balance_chassis/app_flow/chassis_orchestration.h`

说明：

- 这些文件更接近业务装配和流程组织
- 但后续要继续减少其对内部 runtime state 的直接操作

### 不应长期留在 app 的部分

`Move -> runtime/runtime_service/actuator/`

- `runtime/app/balance_chassis/app_flow/actuator_runtime.c`
- `runtime/app/balance_chassis/app_flow/actuator_runtime.h`

原因：

- 直接依赖 `fdcan.h`
- 直接调用 `mit_ctrl`、`CAN_cmd_chassis`
- 直接持有具体电机对象和硬件句柄
- 这是 actuator service，不是 app flow

`Move -> runtime/runtime_service/control_runtime/`

- `runtime/app/balance_chassis/app_flow/chassis_control_support.c`
- `runtime/app/balance_chassis/app_flow/chassis_control_support.h`

原因：

- 它承担了 feedback snapshot、姿态拼装、轮反馈整理、输出限幅
- 这是控制运行时 glue，不是业务装配

`Move -> runtime/runtime_service/observe/`

- `runtime/app/balance_chassis/app_flow/observe_orchestration.c`
- `runtime/app/balance_chassis/app_flow/observe_orchestration.h`
- `runtime/app/balance_chassis/app_flow/observe_runtime.h`

原因：

- 它们服务观测链，不是业务层的模式装配

`Move -> runtime/runtime_service/sensing/` 或 `runtime/runtime_service/observe/`

- `runtime/app/balance_chassis/app_flow/ins_runtime.c`
- `runtime/app/balance_chassis/app_flow/ins_runtime.h`

原因：

- 它们更接近传感器运行时整理，而不是 app flow

## 5.5 `legacy`

### 已基本接近 shell 的

`Transitional`

- `runtime/app/balance_chassis/legacy/remote_task.c`
- `runtime/app/balance_chassis/legacy/observe_task.c`
- `runtime/app/balance_chassis/legacy/motor_control_task.c`

说明：

- 这些文件已经在转向 task shell
- 但仍绑定当前过渡目录，需要配合上层迁移继续收薄

### 仍然偏重的

`Transitional / Split`

- `runtime/app/balance_chassis/legacy/chassis_task.c`
- `runtime/app/balance_chassis/legacy/INS_task.c`

说明：

- 它们仍承担较多运行时 glue 和状态装配
- 后续需要继续拆成 task shell + runtime service 调用

### 兼容头

`Transitional`

- `runtime/app/balance_chassis/legacy/*.h`

说明：

- 当前先保留，避免迁移过程中大面积破坏 include
- 长期要继续瘦身

## 6. `runtime/device` 初始承接建议

当前仓库里还没有正式的 `runtime/device/`，但建议第一版至少建立以下设备节点。

### 6.1 传感器设备

建议新增：

- `runtime/device/sensor/imu_device.*`

底层绑定：

- `runtime/bsp/devices/bmi088/*`
- `runtime/bsp/sitl/BMI088driver_sitl.c`

### 6.2 输入设备

建议新增：

- `runtime/device/input/remote_control_device.*`

底层绑定：

- `runtime/bsp/devices/remote_control/*`

### 6.3 执行器设备

建议新增：

- `runtime/device/actuator/joint_actuator_device.*`
- `runtime/device/actuator/wheel_actuator_device.*`

底层绑定：

- `runtime/bsp/devices/dm_motor/*`
- `runtime/bsp/sitl/dm4310_drv_sitl.c`

## 7. `runtime/runtime_service` 初始承接建议

### 7.1 `runtime_service/actuator`

第一批承接：

- `app_flow/actuator_runtime.*`
- `app_io/actuator_topics.*`
- `legacy/motor_control_task.*`

最终职责：

- actuator command / feedback 运行时
- 执行器命令下发
- 反馈采集
- 启停、保护、限幅

### 7.2 `runtime_service/observe`

第一批承接：

- `app_flow/observe_orchestration.*`
- `app_io/observe_topics.*`
- `legacy/observe_task.*`

最终职责：

- 观测输入整理
- 状态估计运行时组织
- 观测输出构建

### 7.3 `runtime_service/sensing`

第一批承接：

- `app_flow/ins_runtime.*`
- `app_io/ins_topics.*`
- `legacy/INS_task.*`

最终职责：

- 传感器输入整理
- `ins_data` 形成
- 对上提供统一 sensing 输入

### 7.4 `runtime_service/control_runtime`

第一批承接：

- `app_flow/chassis_control_support.*`
- 部分 `legacy/chassis_task.*` 内的运行态 glue

最终职责：

- feedback snapshot
- 控制运行态更新
- 控制输出限幅
- 执行反馈到控制状态的映射

## 8. 拆分优先级

### P0

- 先建 `runtime/device/`
- 先建 `runtime/runtime_service/`

### P1

- 先拆 actuator 线
- 把 `actuator_runtime.*` 和 `actuator_topics.*` 从 app 拔出去

### P2

- 再拆 observe / sensing / control_runtime

### P3

- 最后瘦 `legacy/` 和 `app/runtime_state`

## 9. 当前一句话结论

当前代码不是“不能用”，而是层次落点不对。

下一步不应继续围着 `app` 做细拆，而应先按这张映射表，把文件逐步迁到：

- `device`
- `runtime_service`

等这两层站住后，`app` 才能真正回到业务装配层。
