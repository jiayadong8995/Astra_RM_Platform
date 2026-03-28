# robot_platform

`robot_platform` 是当前仓库的主平台入口。

目标：

1. 脱离 `Keil uvprojx` 作为主构建入口
2. 让底层生成、平台运行时、验证链路在同一仓库中收口
3. 以 `balance_chassis` 为第一条平台化样板链路

当前阶段已落地：

1. `STM32CubeMX` CLI 代码生成链
2. `CMake + arm-none-eabi-gcc` 硬件构建链
3. `balance_chassis_legacy_full.elf` 硬件链接目标
4. `linux-gcc + FreeRTOS POSIX port` 的 SITL 构建目标
5. `runtime/bsp/sitl` 的硬件桩与 UDP bridge 骨架

当前阶段未落地：

1. `flash / debug / replay / test` 的统一 CLI 落地
2. SITL 场景、回放、报告的完整闭环
3. runtime 边界进一步收紧，减少 legacy 头文件直连

## 目录

```text
robot_platform/
  CMakeLists.txt
  cmake/
  docs/
  projects/
  runtime/
  sim/
  tools/
```

## 建议执行顺序

1. 先读 [../README.md](../README.md)
2. 再读 [docs/chassis_import_manifest.md](./docs/chassis_import_manifest.md)
3. 如需看方案背景，再读 [../Platform_Refactor/05_SITL_Roadmap.md](../Platform_Refactor/05_SITL_Roadmap.md)

## 当前可用命令

```bash
python3 -m robot_platform.tools.platform_cli.main generate
python3 -m robot_platform.tools.platform_cli.main build hw_elf
python3 -m robot_platform.tools.platform_cli.main build legacy_full
python3 -m robot_platform.tools.platform_cli.main build sitl
```
