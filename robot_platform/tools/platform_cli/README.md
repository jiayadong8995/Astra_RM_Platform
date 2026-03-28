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

- `generate`: 使用 `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc` 调用官方 `STM32CubeMX` CLI 生成 `runtime/generated/stm32h7_ctrl_board_raw`
- `build hw_elf`: 构建 `balance_chassis_hw_seed.elf`
- `build hw_seed`: 构建底层静态库种子 `balance_chassis_bsp_seed`
- `build legacy_obj`: 编译迁移后的整套 `Chassis/User` 对象库 `balance_chassis_legacy_full_obj`
- `build legacy_full`: 构建迁移后的整套 legacy 固件 `balance_chassis_legacy_full.elf`
- `build sitl`: 构建 Linux 上运行的 `balance_chassis_sitl`

当前未收口：

- `flash`
- `debug`
- `replay`
- `test`

当前确认延后：

- `physics_sim` 场景化闭环
