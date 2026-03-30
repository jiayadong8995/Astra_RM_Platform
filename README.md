# Astra RM Platform

这是一个面向 RoboMaster 控制场景的嵌入式平台架构仓库。

项目当前的重点不是继续讨论“是否迁移”，而是围绕已有历史基线，持续定义、实现和收紧统一的平台架构。

## 项目定义

这个项目可以理解成一个平台项目中的三类资产：

1. `references/legacy/Astra_RM2025_Balance_legacy/`
   历史基线资产镜像。
   用于保存原始固件、`.ioc` 和行为参考，不作为主开发入口。
2. `robot_platform/`
   当前主工程。
   负责承接生成、构建、运行时分层和当前 `SITL` 验证主线。
3. `references/`
   只读参考仓库。
   分为 `external/` 外部开源参考和 `legacy/` 历史基线镜像，用于对照成熟机器人平台和旧工程的目录分工与控制组织方式。

## 当前项目主线

当前项目围绕 `robot_platform/` 推进，主线是：

- 统一生成链
- 统一 GCC/CMake 构建链
- 定义并收紧运行时分层
- 以 `SITL` 作为当前验证方向
- 推进新消息总线体系落地

## 从哪里开始

1. 先读 [robot_platform/README.md](./robot_platform/README.md)
2. 再读 [robot_platform/docs/README.md](./robot_platform/docs/README.md)
3. 如需看参考实现，再读 [robot_platform/docs/ros_robot_algorithm_placement_research.md](./robot_platform/docs/ros_robot_algorithm_placement_research.md)

## 说明

这个 `README` 只负责项目级定义和导航。

架构细节、分工、环境约束和平台边界，统一在 `robot_platform/docs/` 中维护。
