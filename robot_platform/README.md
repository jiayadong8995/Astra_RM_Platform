# robot_platform

`robot_platform` 是 `Astra_RM2025_Balance` 的新平台入口。

目标：

1. 脱离 `Keil uvprojx` 作为主构建入口
2. 把硬件生成、业务代码、验证链路分层
3. 为 `Chassis` 提供第一版迁移落点

当前阶段已落地：

1. 平台目录骨架
2. CMake 主入口
3. GCC toolchain 占位
4. CLI 协议入口
5. schema / project config 示例
6. `Chassis` 第二阶段导入清单
7. 官方 `STM32CubeMX` CLI 的 `generate code` 主路径
8. `generated_raw + HAL + seed BSP` 的 GCC 最小编译链

新的执行顺序：

1. 先跑通纯 `CubeMX` 代码生成链
2. 再导入 `Chassis` 到新平台

当前阶段未落地：

1. 完整的多项目/多板卡 codegen backend 抽象
2. 完整的 `Chassis` 固件 ELF 链接目标
3. 真正的 replay / sim 执行链

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

1. 先读 [docs/phase0_codegen_first.md](./docs/phase0_codegen_first.md)
2. 再读 [docs/phase1_bootstrap.md](./docs/phase1_bootstrap.md)
3. 再读 [docs/phase2_hw_seed.md](./docs/phase2_hw_seed.md)
4. 再看 [docs/chassis_import_manifest.md](./docs/chassis_import_manifest.md)
5. 然后开始做 `Chassis` 最小编译链接入

## 当前可用命令

```bash
python3 -m robot_platform.tools.platform_cli.main generate
python3 -m robot_platform.tools.platform_cli.main build
python3 -m robot_platform.tools.platform_cli.main build hw_seed
python3 -m robot_platform.tools.platform_cli.main build legacy_obj
```
