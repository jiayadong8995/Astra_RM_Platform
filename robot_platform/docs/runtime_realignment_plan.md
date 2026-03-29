# Runtime Realignment Plan

这份文档回答三个问题：

1. 参考项目的分层共性是什么
2. 本项目预期的目标结构应该是什么
3. 当前代码应怎样分批拆回正确层次

它不是细节实现文档，而是运行时结构回正方案。

## 1. 参考项目给出的共性

本项目当前已经在 [references](/home/xbd/workspace/codes/Astra_RM_Platform/references) 中保留多组架构参考。

从这些参考项目里，能抽出来的共性不是某个具体目录名，而是稳定的职责边界。

### 1.1 `basic_framework`

关键结构：

- `bsp/`
- `modules/`
- `application/`

说明：

- 板级与驱动在 `bsp`
- 公共能力在 `modules`
- 机器人业务装配在 `application`

可借鉴点：

- 公共能力与业务装配分开
- 业务层按角色组织，不按单个 legacy task 文件组织

### 1.2 `ros2_control_demos`

关键结构：

- `bringup`
- `description`
- `hardware`
- `controllers`

说明：

- 启动与运行组织独立
- 硬件接口独立
- 控制器独立
- 配置与描述独立

可借鉴点：

- `bringup`、`hardware`、`controllers` 不应混成一层

### 1.3 `iiwa_ros2`

关键结构：

- `iiwa_bringup`
- `iiwa_hardware`
- `iiwa_controllers`
- `iiwa_description`

说明：

- 真实机器人项目里，运行组织、硬件、控制器、配置是不同职责

可借鉴点：

- 项目层并不是“什么都放进去”，它只承接装配

### 1.4 `PX4`

关键结构：

- `boards/`
- `src/modules/`
- `src/modules/simulation/`
- `Tools/simulation/`

说明：

- 板级入口、模块层、仿真接口层、仿真启动脚本层相互分离

可借鉴点：

- 仿真不应散落在业务代码里
- 平台公共层和后端适配层应分开

### 1.5 `ArduPilot`

关键结构：

- `libraries/AP_HAL_*`
- `libraries/SITL`
- `vehicle code`

说明：

- HAL/SITL/业务载具层是分开的

可借鉴点：

- 外部环境后端可以通过正式接口接入
- 运行时与载具业务不应互相吞掉

## 2. 本项目应借鉴什么

本项目当前应借鉴的是：

- 板级与驱动分层
- 设备与运行时分层
- 控制模块与业务装配分层
- 启动/运行组织单独成层
- 验证框架单独成层

本项目当前不应照搬的是：

- ROS 级系统复杂度
- Linux 级完整驱动框架
- 过度插件化和过度泛化接口

一句话：

借鉴职责分层，不照搬生态复杂度。

## 3. 本项目目标结构

本项目更合适的运行时结构应收成：

```text
robot_platform/
  cmake/
  docs/
  projects/
  runtime/
    generated/
    bsp/
    device/
    runtime_service/
    module/
    app/
  sim/
  tools/
```

## 4. 各层职责

### 4.1 `runtime/generated`

职责：

- 承接 `CubeMX` 生成代码
- 保存芯片启动、HAL 初始化、外设初始化

不负责：

- 长期人工维护的业务逻辑

### 4.2 `runtime/bsp`

职责：

- 板级支持
- 具体驱动实现
- 芯片、总线、句柄、寄存器级访问

当前可保留内容：

- `boards/stm32h7_ctrl_board/`
- `devices/bmi088/`
- `devices/dm_motor/`
- `devices/remote_control/`
- `sitl/` 下的 BSP 桩

### 4.3 `runtime/device`

职责：

- 提供统一设备节点抽象
- 隔离具体驱动差异
- 向上只暴露设备语义

建议先建立：

- `sensor/imu`
- `input/remote_control`
- `actuator/joint_motor`
- `actuator/wheel_motor`

### 4.4 `runtime/runtime_service`

职责：

- 把设备层接入控制链
- 承接输入整理、反馈采集、控制运行时 glue

建议至少拆成：

- `sensing/`
- `observe/`
- `actuator/`
- `control_runtime/`

### 4.5 `runtime/module`

职责：

- 算法
- 控制器
- 公共模块
- 消息总线

当前可保留方向：

- `algorithm/`
- `controller/`
- `message_center/`

后续要求：

- 继续减少对 legacy runtime 类型和板级细节的依赖

### 4.6 `runtime/app`

职责：

- 项目业务装配
- 模式管理
- 任务编排
- app 边界契约

当前建议结构：

```text
runtime/app/balance_chassis/
  app_bringup/
  app_io/
  app_flow/
  app_config/
  legacy/
```

但要明确：

- `app` 只是平台中的一层
- 它不应继续承接 actuator runtime、设备反馈整理和硬件句柄访问

### 4.7 `sim`

职责：

- 平台公共验证框架
- `SITL`
- 项目级 sim 适配
- 后续可扩展 backend

建议继续收成：

```text
sim/
  core/
  projects/<robot>/
  backends/
```

## 5. 当前代码与目标结构的偏差

当前项目主方向没有错，偏的是运行时分层。

最主要的偏差有四个。

### 5.1 `app` 吃进了执行层

当前不应长期留在 `app` 的内容包括：

- `app_flow/actuator_runtime.*`
- `app_io/actuator_topics.*`

原因：

- 这部分承接了执行器命令和反馈
- 它更像 actuator service，不像业务装配

### 5.2 `app` 吃进了 control runtime

当前不应长期留在 `app` 的内容包括：

- `app_flow/chassis_control_support.*`
- `app_flow/observe_orchestration.*`

原因：

- 它们在做反馈整理、状态拼装和控制运行态更新
- 这属于 control runtime/observe service

### 5.3 `device` 层缺失

当前上层仍然容易通过运行态间接碰到：

- 具体电机对象
- 具体驱动函数
- 具体硬件句柄

这说明设备抽象没有立住。

### 5.4 `legacy task` 仍然承载过多运行时 glue

当前 `legacy/` 虽然已经承认了迁移期存在，但很多 task 还没有完全退化成 shell。

## 6. 当前目录映射建议

### 6.1 保留在 `runtime/bsp`

- `boards/stm32h7_ctrl_board/*`
- `devices/bmi088/*`
- `devices/dm_motor/*`
- `devices/remote_control/*`
- `sitl/*`

### 6.2 新增 `runtime/device`

建议新增：

```text
runtime/device/
  core/
  sensor/
  input/
  actuator/
```

初始建议承接的对象：

- `imu_device`
- `remote_control_device`
- `joint_actuator_device`
- `wheel_actuator_device`

### 6.3 新增 `runtime/runtime_service`

建议新增：

```text
runtime/runtime_service/
  sensing/
  observe/
  actuator/
  control_runtime/
```

建议迁入的内容：

- `app_flow/actuator_runtime.*` -> `runtime_service/actuator/`
- `app_io/actuator_topics.*` -> `runtime_service/actuator/`
- `app_flow/chassis_control_support.*` -> `runtime_service/control_runtime/`
- `app_flow/observe_orchestration.*` -> `runtime_service/observe/`
- `app_flow/ins_runtime.*` -> `runtime_service/sensing/` 或 `runtime_service/observe/`

### 6.4 保留在 `runtime/module`

继续保留：

- `algorithm/EKF/*`
- `algorithm/PID/*`
- `algorithm/VMC/*`
- `algorithm/kalman/*`
- `algorithm/mahony/*`
- `controller/*`
- `message_center/*`

但后续继续去掉：

- 对 `legacy` 头和 app runtime 类型的直接依赖

### 6.5 保留在 `runtime/app`

建议长期保留在 app 层的内容：

- `app_bringup/freertos_app.c`
- `app_bringup/task_registry.*`
- `app_io/topic_contract.h`
- `app_io/chassis_topics.*`
- `app_io/remote_topics.*`
- `app_config/*`

说明：

- `app_io` 长期只保留业务边界 topic
- `actuator_cmd/feedback` 这类执行链 topic 不应长期停在 app 层

### 6.6 暂留在 `legacy/`

当前先保留：

- `legacy/INS_task.*`
- `legacy/chassis_task.*`
- `legacy/remote_task.*`
- `legacy/observe_task.*`
- `legacy/motor_control_task.*`

原则：

- 当前先承认迁移期存在
- 后续逐步压成 shell

## 7. 拆分批次

不建议一次性做大改。

建议按下面四个批次推进。

### Batch 1. 先立空层

目标：

- 创建 `runtime/device/`
- 创建 `runtime/runtime_service/`
- 先把职责边界立住

不要求：

- 第一批就把所有实现全部迁完

### Batch 2. 优先拆 actuator 线

目标：

- 把执行器命令下发和反馈采集从 `app` 里拔出去

优先迁移：

- `app_flow/actuator_runtime.*`
- `app_io/actuator_topics.*`
- `legacy/motor_control_task.*`

### Batch 3. 再拆 observe/control runtime

目标：

- 把反馈整理、控制运行态拼装从 `app_flow` 中移走

优先迁移：

- `app_flow/chassis_control_support.*`
- `app_flow/observe_orchestration.*`
- `legacy/observe_task.*`
- `legacy/chassis_task.*`

### Batch 4. 最后瘦 app 和 legacy

目标：

- 让 app 真正只剩业务装配
- 让 legacy task 退化为薄 shell

## 8. 当前一句话结论

本项目接下来不应继续围绕 `app` 做局部细拆，而应先按参考项目抽出的稳定分层，把结构重新摆正为：

`generated + bsp + device + runtime_service + module + app + sim`

其中当前最优先要补的是：

- `device`
- `runtime_service`
