# Astra RM Platform

当前仓库已经收口为两条主线：

1. `Astra_RM2025_Balance/`
   保留的历史整机工程归档，用于追溯原始 `CubeMX + Keil` 资产。
2. `robot_platform/`
   当前继续演进的平台主入口，负责代码生成、GCC 构建和 SITL 验证。

## 当前判断

- 硬件生成链和 GCC 构建链已经可用。
- SITL 已经具备 Linux 构建与进程启动能力。
- 过时的路线草稿、占位入口和 IDE 残留已开始清理。

## 现在应该从哪里开始

1. 阅读 [robot_platform/README.md](./robot_platform/README.md)
2. 如需看方案背景，再看 [Platform_Refactor/README.md](./Platform_Refactor/README.md)

## 非目标

- 不再把仓库主入口放在 `Keil uvprojx`
- 不再保留多个互相冲突的“主验证路线”说明
