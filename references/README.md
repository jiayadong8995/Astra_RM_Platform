# references

这个目录只存放外部开源项目的只读参考代码。

用途：

- 参考平台分层
- 参考应用层拆分
- 参考硬件接口、控制器、bringup 和仿真组织方式

这里的内容：

- 不属于 `robot_platform` 运行资产
- 不参与当前仓库的构建
- 不参与当前仓库的测试
- 不作为当前项目实现代码的直接依赖

## 当前参考项目

- `basic_framework`
  RoboMaster 嵌入式框架参考，重点看 `bsp / modules / application`
- `ros2_control_demos`
  ROS 2 控制框架参考，重点看 `bringup / description / hardware / controllers`
- `iiwa_ros2`
  工业机器人 ROS 2 栈参考，重点看 `bringup / controllers / description / hardware`
- `nav2_bringup`
  ROS 2 应用级 bringup 参考，重点看 `launch / params / urdf / rviz`

## 使用原则

- 这里只做架构和实现参考
- 不在这些目录上继续开发当前项目功能
- 需要引用时，应在文档中说明参考来源和参考点
