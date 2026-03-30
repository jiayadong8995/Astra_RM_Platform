# ROS Robot Algorithm Placement Research

这份文档只回答一个问题：

在 ROS 风格、且带控制算法的机器人平台中，算法通常放在哪里。

结论先写在前面：

- 不放在 `bringup`
- 不放在 `hardware`
- 不放在 `description`
- 通用算法原件放公共库
- 机器人专属控制算法放 `controllers` 或机器人专属 `control` 包
- 状态估计常单独放 `estimation` 包

## 1. 调研目标

这轮调研用于校验 `robot_platform` 当前分层是否合理，重点看：

1. 机器人专属控制算法应该归属哪一层
2. 状态估计应不应该和控制器分开
3. `hardware/device` 层是否应该承接算法
4. `bringup/app` 层应不应该承接算法

## 2. 调研样本

本次主要参考 4 组质量较高的项目：

1. `ros2_control_demos`
   - 上游：https://github.com/ros-controls/ros2_control_demos
   - 本地：`references/external/ros2_control_demos`
2. `iiwa_ros2`
   - 上游：https://github.com/ICube-Robotics/iiwa_ros2
   - 本地：`references/external/iiwa_ros2`
3. `legged_control`
   - 上游：https://github.com/qiayuanl/legged_control
   - 本地：`references/external/legged_control`
4. `basic_framework`
   - 上游：https://gitee.com/hnuyuelurm/basic_framework
   - 本地：`references/external/basic_framework`

另外参考了 `ros2_control` 官方文档：

- Controller Manager
  - https://docs.ros.org/en/ros2_packages/humble/api/controller_manager/doc/userdoc.html
- hardware_interface
  - https://docs.ros.org/en/iron/p/hardware_interface/

## 3. 观察结果

## 3.1 `ros2_control_demos`

目录长期稳定收成：

- `bringup`
- `description`
- `hardware`
- `controllers`

例如：

- `references/external/ros2_control_demos/example_12/bringup`
- `references/external/ros2_control_demos/example_12/hardware`
- `references/external/ros2_control_demos/example_12/controllers`

说明：

- `bringup` 负责启动和运行组织
- `hardware` 负责硬件接口
- `controllers` 负责控制器实现

这里没有把控制算法放进 `hardware` 或 `bringup`。

## 3.2 `iiwa_ros2`

目录直接拆成：

- `iiwa_bringup`
- `iiwa_controllers`
- `iiwa_hardware`
- `iiwa_description`

例如：

- `references/external/iiwa_ros2/iiwa_bringup`
- `references/external/iiwa_ros2/iiwa_controllers/impedance_controller/src/impedance_controller.cpp`
- `references/external/iiwa_ros2/iiwa_hardware`

从 [impedance_controller.cpp](/home/xbd/workspace/codes/Astra_RM_Platform/references/external/iiwa_ros2/iiwa_controllers/impedance_controller/src/impedance_controller.cpp) 可以直接看出：

- 控制算法写在 controller plugin 里
- controller 通过 `hardware_interface` 暴露的 state/command interface 工作
- controller 不直接落到硬件实现目录里

这说明：

- 机器人专属控制器放 `controllers`
- `hardware` 只提供接口和状态命令面

## 3.3 `legged_control`

这是这轮最有价值的参考。

它的目录明确拆成：

- `legged_controllers`
- `legged_estimation`
- `legged_hw`
- `legged_interface`
- `legged_wbc`

例如：

- `references/external/legged_control/legged_controllers/src/LeggedController.cpp`
- `references/external/legged_control/legged_estimation/src/StateEstimateBase.cpp`
- `references/external/legged_control/legged_hw`

从 [LeggedController.cpp](/home/xbd/workspace/codes/Astra_RM_Platform/references/external/legged_control/legged_controllers/src/LeggedController.cpp) 可以看到：

- controller 里组装 MPC、WBC、state estimate
- 控制器通过 hardware interface 拿 joint/contact/imu handle
- 主控制算法明确属于 controller 包，不属于 hardware 包

从 [StateEstimateBase.cpp](/home/xbd/workspace/codes/Astra_RM_Platform/references/external/legged_control/legged_estimation/src/StateEstimateBase.cpp) 可以看到：

- 状态估计被单独放在 `legged_estimation`
- 估计器不混在 hardware，也不混在 bringup

这说明在“算法很多”的项目里，成熟做法通常是：

- 控制器单独成包
- 状态估计单独成包
- 硬件接口单独成包

## 3.4 `basic_framework`

虽然不是 ROS，但对嵌入式平台有很强参考价值。

它的结构是：

- `bsp`
- `modules`
- `application`

其中 `application` 继续按业务角色拆：

- `cmd`
- `chassis`
- `gimbal`
- `shoot`

这说明：

- 应用层更适合按业务角色/装配职责组织
- 通用算法和中间能力收在 `modules`
- 不围绕 task 文件名组织整个系统

## 4. 总结规律

这几组参考有非常稳定的共同点：

1. 算法不放在 `bringup`
   - `bringup` 只负责启动、launch、参数和运行组织
2. 算法不放在 `hardware/device`
   - `hardware` 或 `device` 只负责设备语义和接口
3. 机器人专属控制算法放在 `controllers/control`
   - 阻抗、MPC、WBC、平衡控制都属于这一类
4. 状态估计经常单独成模块
   - 特别是算法复杂时，估计器和控制器并列
5. 通用算法原件不和机器人专属控制混在一起
   - PID、滤波器、数学工具是公共原件
   - 机器人平衡/步态/跳跃逻辑是机器人专属控制

## 5. 对 `robot_platform` 的直接启发

结合当前项目，我建议把算法归属定成下面这版：

- `runtime/device`
  - 设备语义
  - backend profile
  - 不放控制算法

- `runtime/control/state`
  - 状态估计
  - 观测
  - 状态融合

- `runtime/control/controllers`
  - 平衡控制
  - 跳跃/恢复控制
  - 机器人专属控制组合逻辑

- `runtime/control/constraints`
  - 限幅
  - 安全约束
  - 输出裁剪

- `runtime/module`
  - PID
  - 滤波器
  - 数学工具
  - 通用原件

- `runtime/app`
  - bringup
  - intent/mode
  - orchestration
  - 不放控制算法实现

## 6. 当前判断

所以从这轮调研回看，`robot_platform` 当前已经收出来的方向是对的：

- `device`
  - 不应继续承接算法
- `control`
  - 应该成为机器人专属控制与状态主链
- `app`
  - 应继续保持装配层身份

如果后面要继续对齐成熟模式，最合理的深化方向不是新增层，而是：

1. 继续纯化 `control/state`
2. 继续纯化 `control/controllers`
3. 让 `module` 只保留公共原件
4. 保持 `app` 不回流控制逻辑

## 7. 结论

一句话总结：

在 ROS 风格且带算法的机器人平台里，算法通常放在机器人专属 `controllers/control` 层，状态估计常单独成 `estimation/state` 层，`hardware/device` 和 `bringup/app` 一般不承接算法实现。
