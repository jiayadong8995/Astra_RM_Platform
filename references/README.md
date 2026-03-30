# references

这个目录存放只读参考代码，包括外部开源项目，以及本仓库自己的历史基线镜像。

用途：

- 参考平台分层
- 参考应用层拆分
- 参考硬件接口、控制器、bringup 和仿真组织方式
- 参考平台架构设计，不直接决定当前实现复杂度

这里的内容：

- 不属于 `robot_platform` 运行资产
- 不参与当前仓库的构建
- 不参与当前仓库的测试
- 不作为当前项目实现代码的直接依赖
- 不在这些目录中继续演进当前主线实现

## 当前参考项目

- `Astra_RM2025_Balance_legacy`
  本仓库历史基线镜像，重点看 `Chassis / Gimbal` 中原始固件、`.ioc` 和旧分层组织
- `basic_framework`
  RoboMaster 嵌入式框架参考，重点看 `bsp / modules / application`
- `PX4-Autopilot`
  飞控 SITL 与仿真基础设施参考，重点看 `Tools/simulation` 与 `src/modules/simulation`
- `ardupilot`
  SITL 与外部 physics backend 接口参考，重点看 `libraries/SITL`
- `betaflight`
  轻量 SITL target 参考，重点看 `src/platform/SIMULATOR/target/SITL`
- `ros2_control_demos`
  ROS 2 控制框架参考，重点看 `bringup / description / hardware / controllers`
- `iiwa_ros2`
  工业机器人 ROS 2 栈参考，重点看 `bringup / controllers / description / hardware`
- `nav2_bringup`
  ROS 2 应用级 bringup 参考，重点看 `launch / params / urdf / rviz`
- `StandardRobotpp`
  RoboMaster 嵌入式电控框架参考，重点看 `application / components / bsp / application/robot_param*.h`

## 使用原则

- 这里只做架构和实现参考
- 不在这些目录上继续开发当前项目功能
- 需要引用时，应在文档中说明参考来源和参考点
