# Sim Architecture Research

这份文档用于回答当前项目关于 `sim` 的三个关键问题：

1. `sim` 应该是平台公共能力，还是每个机器人各做一套
2. 当前阶段最适合采用什么类型的 `sim/SITL` 方案
3. 如果后续要接入物理仿真与视觉，当前架构怎样保持可扩展

本文档基于以下本地参考代码与对应项目结构调研形成：

- `references/PX4-Autopilot`
- `references/ardupilot`
- `references/betaflight`
- `references/ros2_control_demos`
- `references/iiwa_ros2`
- `references/nav2_bringup`

## 1. 当前问题

当前项目已经确定：

- `sim` 应服务于平台验证
- 当前主线是 `SITL`
- `replay` 暂不作为当前交付项
- `physics_sim` 当前不进入执行面

但后续仍要考虑：

- 不同机器人接入 `sim`
- 外部传感器扩展，例如视觉
- 未来可能接入简化物理仿真

因此本轮调研的重点不是“立刻做什么”，而是：

当前 `sim` 架构怎样设计，才能在后续扩展时符合开闭原则。

## 2. 调研结果

## 2.1 PX4

参考目录：

- [references/PX4-Autopilot](/home/xbd/workspace/codes/Astra_RM_Platform/references/PX4-Autopilot)

关键结构：

- `Tools/simulation/`
- `src/modules/simulation/`
- `boards/px4/sitl/`

其中：

- `Tools/simulation/` 负责 simulator 启动与连接脚本
- `src/modules/simulation/` 提供 PX4 内部的仿真接口层
- 同时支持多种 simulator/backend：
  - `simulator_mavlink`
  - `gz_bridge`
  - `sensor_*_sim`
  - `pwm_out_sim`

### 结论

PX4 的重点不是某个具体 simulator，而是：

- 平台有统一仿真入口
- 具体 simulator/backend 可替换
- 仿真接口在平台内部是正式模块，不是零散脚本

对本项目的启发是：

`sim` 应该有公共框架层，而不是继续把所有逻辑堆在单个 bridge 脚本里。

## 2.2 ArduPilot

参考目录：

- [references/ardupilot](/home/xbd/workspace/codes/Astra_RM_Platform/references/ardupilot)

关键结构：

- `libraries/AP_HAL_SITL`
- `libraries/SITL`
- `libraries/SITL/examples/JSON`

其中：

- `libraries/SITL/` 是统一仿真层
- vehicle、sensor、backend 在统一 SITL 层内扩展
- `examples/JSON` 提供了一个很轻的外部 simulator 接口

`JSON` backend 的核心特征：

- 使用 UDP 连接
- SITL 输出 actuator / servo 数据
- 外部 physics backend 回传：
  - IMU
  - position
  - attitude / quaternion
  - velocity
- 可选追加：
  - rangefinder
  - wind
  - battery
  - RC input

### 结论

ArduPilot 最值得本项目借鉴的不是它的高保真仿真，而是：

- 定义正式仿真输入输出协议
- 允许外部 simulator/physics backend 通过统一接口接入
- optional sensor fields 可渐进扩展

这非常适合本项目未来的方向：

- 先做 `SITL`
- 后续再接 `physics`
- 再后续接 `vision`

而无需推翻核心结构。

## 2.3 Betaflight

参考目录：

- [references/betaflight](/home/xbd/workspace/codes/Astra_RM_Platform/references/betaflight)

关键结构：

- `src/platform/SIMULATOR/target/SITL`

特点：

- 直接提供 `SITL` target
- 固件在 PC 上运行
- 通过 UDP 与 Gazebo 交互
- 结构轻量，强调先把固件跑起来

### 结论

Betaflight 的启发是：

- `SITL` 的第一阶段可以很轻
- 不需要一开始就建设完整 physics/world framework
- 只要 actuator/sensor 接口清楚，验证平台就已经成立

这与本项目当前阶段高度一致。

## 2.4 ROS 相关参考

参考目录：

- [references/ros2_control_demos](/home/xbd/workspace/codes/Astra_RM_Platform/references/ros2_control_demos)
- [references/iiwa_ros2](/home/xbd/workspace/codes/Astra_RM_Platform/references/iiwa_ros2)
- [references/nav2_bringup](/home/xbd/workspace/codes/Astra_RM_Platform/references/nav2_bringup)

这些项目的价值不在于 `sim` 细节，而在于：

- `bringup`
- `config`
- `hardware`
- `controllers`

这些职责如何拆分。

### 结论

这些项目说明：

- 运行组织层是独立职责
- 配置和硬件/控制器应分开
- 但它们的复杂度主要服务于 ROS 的上层系统需求

因此本项目可以借鉴其组织思想，但不应引入 ROS 级仿真复杂度。

## 3. 总判断

这轮调研后的核心结论如下。

### 3.1 `sim` 应该是平台公共验证架构

`sim` 不应该变成“每个机器人一套独立工程”。

更合理的方式是：

- 平台提供公共 `sim core`
- 每个机器人提供自己的 `profile/adapter`

换句话说：

不是每个机器人重写一套 `sim`，而是每个机器人补自己的业务适配层。

### 3.2 当前主线应继续保持为 `SITL`

当前阶段最适合本项目的不是高保真物理仿真，而是：

- 固件在 PC/Linux 中运行
- 外部 bridge 或 backend 提供输入
- 保证主控制链与 `hw` 共用

也就是：

- `SITL`
- 轻量 bridge
- 最小 smoke / validation

### 3.3 后续物理仿真应作为新增 backend 接入

如果未来要接 `physics`，不应修改 `sim core` 主流程来堆功能。

更合理的方式是：

- 新增 physics backend
- 新增对应 adapter
- 复用已有 runner/session/report/profile

这就是本项目在 `sim` 上保持开闭原则的关键。

### 3.4 视觉应作为项目层的可选传感器扩展

后续如需接入视觉，不应直接做进 `sim core`。

更合理的方式是：

- `sim core` 提供统一 sensor injection 能力
- 具体机器人在项目层挂接 `vision adapter`

这样没有视觉需求的机器人不会被强迫理解这套能力。

## 4. 建议的 `sim` 结构

结合本项目现状与调研结论，建议 `sim` 最终演进为：

```text
sim/
  core/
    runner
    session
    transport
    report
    bridge_base
    profile_base
  projects/
    balance_chassis/
      profile
      runtime_boundary
      bridge_adapter
      smoke
      sensors/
      actuators/
      optional_vision/
  backends/
    sitl_bridge
    replay_source
    physics_backend
```

## 4.1 `sim/core`

职责：

- 进程拉起
- session 生命周期
- transport/protocol
- report
- 通用 bridge 框架
- profile 接口

原则：

- 不认具体机器人
- 不认具体 physics 实现
- 只认 profile 与 backend 接口

## 4.2 `sim/projects/<robot>`

职责：

- 机器人 runtime boundary
- command/state/topic 映射
- sensor/actuator contract
- smoke 配置
- 可选视觉适配

原则：

- 机器人差异只放这一层
- 不污染 `sim core`

## 4.3 `sim/backends`

职责：

- `sitl_bridge`
- `replay_source`
- `physics_backend`

原则：

- 新增验证模式时，尽量通过新增 backend 解决
- 而不是修改 runner 核心流程

## 5. 为什么这套结构符合开闭原则

后续如果新增：

- 一个新机器人
- 一套物理仿真
- 一个视觉输入源
- 一个 replay backend

原则上应主要新增：

- 新 profile
- 新 adapter
- 新 backend

而不是反复修改：

- `sim core`
- 现有机器人实现
- 现有 session/report 主流程

一句话：

新增能力应当主要通过“扩展文件”完成，而不是“反复修改核心流程”。

## 6. 当前阶段的落地建议

考虑到本项目不追求一版到位，当前更稳妥的落地顺序是：

1. 继续把现有 `sim/core + sim/projects/balance_chassis` 走通
2. 先把 `SITL` 公共框架和项目适配层边界收干净
3. 暂不立即上 `physics`
4. 先确保未来新增 `physics` 时是“加 backend”，不是“重写 bridge”

## 7. 最终结论

本轮调研后的正式结论如下：

1. `sim` 应当是平台公共验证架构
2. 每个机器人应只补自己的业务适配层，而不是各自重写一套 `sim`
3. 当前最适合本项目的主线仍然是 `SITL`
4. `physics` 应作为未来新增 backend 接入
5. `vision` 应作为项目层可选传感器扩展
6. `sim` 当前最重要的不是“做复杂”，而是“把扩展点留对”

## 8. 参考来源

- PX4-Autopilot
  https://github.com/PX4/PX4-Autopilot
- ArduPilot
  https://github.com/ArduPilot/ardupilot
- Betaflight
  https://github.com/betaflight/betaflight
- ros2_control_demos
  https://github.com/ros-controls/ros2_control_demos
- iiwa_ros2
  https://github.com/ICube-Robotics/iiwa_ros2
- nav2_bringup
  https://github.com/MonashNovaRover/nav2_bringup
