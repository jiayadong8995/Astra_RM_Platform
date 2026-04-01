# platform_cli

统一命令入口建议如下：

```text
platform generate
platform build
platform flash
platform debug
platform replay
platform sim
platform test
```

当前仓库收口后的重点命令只有两类：

- `generate`: 生成底层代码
- `build`: 构建硬件或 SITL 目标

当前已经落地：

- `generate`: 使用 `references/legacy/Astra_RM2025_Balance_legacy/Chassis/CtrlBoard-H7_IMU.ioc` 调用官方 `STM32CubeMX` CLI 生成 `runtime/bsp/boards/stm32h7_ctrl_board/generated`
- `build hw_elf`: 构建 `balance_chassis_hw_seed.elf`
- `build hw_seed`: 构建底层静态库种子 `balance_chassis_bsp_seed`
- `build sitl`: 构建 Linux 上运行的 `balance_chassis_sitl`
- `sim`: 自动构建并执行最小 SITL smoke run，输出 `build/sim_reports/sitl_smoke.json`
  并在终端打印一行 smoke summary
  支持 `--duration <seconds>` 和 `--skip-build`
- `test sim`: 运行 sim 侧最小单元回归，锁住 bridge 事件解析和 smoke report 汇总逻辑

当前未收口：

- `flash`
- `debug`
- `replay`

当前确认延后：

- `physics_sim` 场景化闭环
