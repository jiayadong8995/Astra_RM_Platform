# App Layer Architecture Research

这份文档用于回答一个具体问题：

`runtime/app` 这一层应该怎么拆，参考哪些开源项目，哪些思想应该借鉴，哪些复杂度不应该引入。

本文档基于以下本地参考代码与官方项目结构调研形成：

- `references/basic_framework`
- `references/ros2_control_demos`
- `references/iiwa_ros2`
- `references/nav2_bringup`

## 1. 调研目标

当前项目正在把 legacy 底盘控制代码迁入 `robot_platform`。

在这个过程中，`runtime/app` 暴露出两个问题：

1. 当前目录仍然主要按 legacy task 组织
2. 新增的 `io/`、`control/` 目录虽然开始收口，但职责表达还不够稳定

因此本轮调研的目标不是“照搬外部框架”，而是回答：

- app 层到底要承担什么职责
- app 层应该拆成几层
- 是否需要参考 ROS/大型机器人项目的分工方式
- 参考时应该借哪些思想，避免哪些复杂度

## 2. 当前项目现状

当前 [runtime/app/balance_chassis](/home/xbd/workspace/codes/Astra_RM_Platform/robot_platform/runtime/app/balance_chassis) 还处在平台化迁移的强过渡期。

主要问题有：

1. 启动入口、线程创建和 `hw/sitl` 差异仍混在 legacy 启动方式里
2. `io/` 当前仍主要承担 `topic -> legacy state` 的过渡胶水职责
3. `control/` 当前更像业务编排 helper，不像平台级 `controller` 层
4. app 目录仍然主要按 `INS_task / chassis_task / observe_task / motor_control_task / remote_task` 组织

这意味着当前 `app` 还没有真正成为“业务装配层”。

## 3. 参考项目调研结果

## 3.1 basic_framework

参考目录：

- [references/basic_framework](/home/xbd/workspace/codes/Astra_RM_Platform/references/basic_framework)

核心结构：

- `bsp/`
- `modules/`
- `application/`

其中 `application/` 继续按业务角色拆成：

- `robot.c`
- `cmd/robot_cmd.c`
- `chassis/chassis.c`
- `gimbal/gimbal.c`
- `shoot/shoot.c`

而 `message_center` 位于：

- `modules/message_center/`

### 可借鉴点

- 公共能力在模块层
- 机器人业务差异在应用层
- 应用层按业务角色组织，而不是按旧 task 文件组织

### 不应照搬点

- 它的目录结构更适合单仓单控制工程
- 不适合直接作为 `robot_platform` 的目录模板

## 3.2 ros2_control_demos

参考目录：

- [references/ros2_control_demos](/home/xbd/workspace/codes/Astra_RM_Platform/references/ros2_control_demos)

典型例子结构，例如 `example_12`：

- `bringup/`
- `description/`
- `hardware/`
- `controllers/`

README 还明确说明，真实机器人通常应把这些拆成独立 package。

### 可借鉴点

- 启动/运行组织是独立职责
- 机器人描述/配置是独立职责
- 硬件接口和控制器实现是独立职责

### 对本项目的启发

`app_main` 不够表达这一层职责，真正缺的是：

- `bringup`
- 或 `runtime orchestration`

## 3.3 iiwa_ros2

参考目录：

- [references/iiwa_ros2](/home/xbd/workspace/codes/Astra_RM_Platform/references/iiwa_ros2)

核心结构：

- `iiwa_bringup`
- `iiwa_controllers`
- `iiwa_description`
- `iiwa_hardware`

### 可借鉴点

- 真实机器人项目会显式把 bringup 单独拉出来
- config/description 与 hardware/controllers 分离
- 运行组织层和实现层不会混在一起

### 对本项目的启发

`app` 层如果没有单独的 bringup/orchestration 概念，后面所有启动、任务管理、模式切换都会重新堆回 task 和 helper 文件。

## 3.4 nav2_bringup

参考目录：

- [references/nav2_bringup](/home/xbd/workspace/codes/Astra_RM_Platform/references/nav2_bringup)

核心结构：

- `launch/`
- `params/`
- `urdf/`
- `maps/`
- `rviz/`

它本质上是一个应用级 bringup 包。

### 可借鉴点

- bringup 是应用层的重要组成部分
- config 和 launch/runtime organization 经常是单独一层

### 需要明确的边界

`nav2_bringup` 代表的是上层机器人系统复杂度，不代表 STM32 底层控制平台需要采用同级别复杂架构。

## 4. 调研后的总判断

参考这些项目后，可以得到一个稳定结论：

1. app 层必须承担“业务装配”职责
2. app 层不能继续只是 legacy task 的落位目录
3. app 层至少要有：
   - 配置
   - 输入输出边界
   - 启动/运行组织
   - 业务编排

因此，单纯的：

- `app_main`
- `app_io`
- `app_config`

是不够的。

同时，直接引入一个大而泛的：

- `app_logic`

也不理想，因为它容易变成新的杂物间目录。

## 5. 为什么不引入 ROS 级复杂度

这是本轮调研最重要的边界结论。

ROS/ROS 2 大型机器人项目的很多复杂度，来自这些需求：

- 建图
- 导航
- 多进程节点系统
- 动态参数
- 大规模上层集成
- 可视化与仿真生态

而当前项目解决的问题是：

- STM32 底层控制平台化
- legacy 控制链迁移
- `hw + sitl`
- 新消息总线体系
- app 层收口

因此，本项目只应借鉴 ROS 的：

- 职责分离思想
- bringup 概念
- config 与实现分离
- hardware / controller / application 分离

不应引入：

- ROS 级中间件复杂度
- 动态参数体系
- 建图/导航导向的系统结构
- 过度节点化

一句话说：

本项目参考 ROS 的组织原则，但不采用 ROS 的系统复杂度。

## 6. 建议的 app 层正式结构

结合当前项目实际与参考项目结论，建议 `runtime/app/balance_chassis` 最终收成：

```text
runtime/app/balance_chassis/
  app_bringup/
  app_config/
  app_io/
  app_flow/
  legacy/
```

## 6.1 `app_bringup`

职责：

- 任务创建
- 生命周期
- 启动顺序
- `hw/sitl` 运行入口差异管理

这一层对应参考项目里的：

- `bringup`
- `launch`
- runtime orchestration entry

## 6.2 `app_config`

职责：

- 项目参数
- app 级 topic/command/state 定义
- profile

这一层对应参考项目里的：

- `config`
- `description` 中和项目差异直接相关的部分

## 6.3 `app_io`

职责：

- app 边界输入输出适配
- topic 与 app 输入/输出之间的转换

注意：

它的目标不应是长期维持 `topic -> legacy state` 的直接胶水，而应逐步演进成真正的边界适配层。

## 6.4 `app_flow`

职责：

- 业务编排
- 模式切换
- 角色协作
- 流程装配

这里不建议命名为 `app_logic`。

原因：

- `logic` 语义过宽
- 很容易重新变成杂物间目录

而 `flow` / `orchestration` 更能表达真实职责：它负责装配和编排，不负责承载公共算法实现。

## 6.5 `legacy`

职责：

- 存放尚未拆完的 legacy task 与兼容资产

引入这一层的原因不是鼓励长期保留 legacy，而是为了明确：

- 当前仍处在迁移期
- 不是所有旧代码都已经成为最终平台结构的一部分

## 7. 对当前代码的映射建议

### 7.1 应长期保留并演进的

- [freertos_legacy.c](/home/xbd/workspace/codes/Astra_RM_Platform/robot_platform/runtime/app/balance_chassis/freertos_legacy.c)
  后续演进为 `app_bringup` 入口
- [robot_def.h](/home/xbd/workspace/codes/Astra_RM_Platform/robot_platform/runtime/app/balance_chassis/robot_def.h)
  后续进入 `app_config`
- `io/`
  后续演进为 `app_io`

### 7.2 应重命名并重新定位的

- `control/`
  后续应演进为 `app_flow`
  不建议长期保留 `control` 这个名字，以避免与 `runtime/module/controller` 冲突

### 7.3 应明确视为过渡资产的

- `INS_task.c`
- `chassis_task.c`
- `observe_task.c`
- `motor_control_task.c`
- `remote_task.c`
- `INS_task.h / ins_task.h`

这些当前仍有价值，但更适合认知上进入 `legacy/`，再逐步拆薄。

## 8. 最终建议

这轮调研后的正式建议如下：

1. 不采用 ROS 级复杂系统架构
2. app 层正式目标不定为 3 层，而定为 5 层
3. 推荐采用：

- `app_bringup`
- `app_config`
- `app_io`
- `app_flow`
- `legacy`

4. 后续重构方向不是继续围绕 `*_task.c` 做局部修补，而是把 `runtime/app` 真正升级为“机器人业务装配层”

## 9. 参考来源

- PX4 architecture documentation
  https://docs.px4.io/v1.14/en/concept/architecture
- basic_framework
  https://gitee.com/hnuyuelurm/basic_framework
- ros2_control_demos
  https://github.com/ros-controls/ros2_control_demos
- iiwa_ros2
  https://github.com/ICube-Robotics/iiwa_ros2
- nav2_bringup
  https://github.com/MonashNovaRover/nav2_bringup
