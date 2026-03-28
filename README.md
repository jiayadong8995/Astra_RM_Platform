# Astra RM Platform

这是一个嵌入式平台化迁移项目的仓库。

项目目标不是继续维护一份以 `CubeMX + Keil + legacy task` 为中心的单体固件，而是把原有 RoboMaster 底盘控制工程逐步收口到统一的平台体系中。

## 项目定义

这个项目可以理解成一个平台项目中的三类资产：

1. `Astra_RM2025_Balance/`
   历史基线资产。
   用于保存原始固件、`.ioc` 和行为参考，不作为主开发入口。
2. `robot_platform/`
   当前主工程。
   负责承接生成、构建、运行时分层和当前 `SITL` 验证主线。
3. `Platform_Refactor/`
   路线决策归档。
   用于解释为什么项目最终收敛到当前路线。

## 当前项目主线

当前项目围绕 `robot_platform/` 推进，主线是：

- 统一生成链
- 统一 GCC/CMake 构建链
- 逐步收紧运行时分层
- 以 `SITL` 作为当前验证方向
- 推进新消息总线体系落地

## 从哪里开始

1. 先读 [robot_platform/README.md](./robot_platform/README.md)
2. 再读 [robot_platform/docs/README.md](./robot_platform/docs/README.md)
3. 如需看路线背景，再读 [Platform_Refactor/README.md](./Platform_Refactor/README.md)

## 说明

这个 `README` 只负责项目级定义和导航。

架构细节、分工、环境约束和迁移边界，统一在 `robot_platform/docs/` 中维护。
