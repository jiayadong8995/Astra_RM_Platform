# Phase 0 Codegen First

在导入现有项目之前，必须先独立跑通 `CubeMX` 代码生成链。

## 为什么必须先做这一步

如果不先证明“配置 -> `.ioc/.ioc2` -> generated code”这条链是通的，后续把 `Chassis` 导入平台只会把旧工程复制一遍，不能证明新平台真的掌握了生成入口。

## 当前仓库可用输入

目前仓库内可直接用于验证的输入文件：

- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Gimbal/standard_tpye_c.ioc`

建议优先使用：

- `Chassis/CtrlBoard-H7_IMU.ioc`

原因：

- 它是后续第一版平台要迁移的主目标

## Phase 0 最小目标

先不要求 yaml 驱动生成 `.ioc2`，只要求先跑通：

1. 选择一个现有 `.ioc`
2. 用 `CubeMX` CLI 执行代码生成
3. 输出到一个固定的 generated 目录
4. 能重复生成
5. 能与原工程目录做结构对比

## 平台里的输出位置

建议固定为：

```text
robot_platform/runtime/generated/stm32h7_ctrl_board_raw/
```

说明：

- `_raw` 表示这是第一阶段“直接由 CubeMX 生成”的原始产物
- 第二阶段导入 `Chassis` 时再整理为正式 `generated/stm32h7_ctrl_board/`

## 当前阻塞

本机当前未在 `PATH` 中发现可用 CLI：

- `STM32CubeMX`
- `stm32cubemx`

当前机器已经安装并验证可启动的 `STM32CubeMX 6.17.0` 路径为：

- `/home/xbd/.local/stm32cubemx/6.17.0/STM32CubeMX`

因此当前的阻塞点已经不是“没有 CLI”。

当前已验证成功的主路径是：

1. `config load "/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc"`
2. `generate code "<output dir>"`
3. `exit`

当前不作为平台主线的路径是：

- `project generate`

原因：

- 在当前工程上会触发 `ProjectManager/ThirdParty/FREERTOS` 相关异常
- 平台本来也不需要依赖 CubeMX 生成最终完整工程

## 当前验证结果

已验证生成出的典型文件包括：

- `Src/main.c`
- `Src/freertos.c`
- `Src/gpio.c`
- `Src/fdcan.c`
- `Inc/main.h`
- `Inc/FreeRTOSConfig.h`

## 完成标准

满足以下条件后，才进入 `Chassis` 导入：

1. `CubeMX` CLI 可调用
2. 至少一份 `.ioc` 可稳定生成
3. generated 输出目录固定
4. 平台里已经有 `generate` 命令能驱动这件事
