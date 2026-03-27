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

当前阶段只落协议和占位实现。

当前优先级：

1. `generate`
2. `build`
3. 其他命令后续补齐

当前已经落地：

- `generate`: 使用现有 `Chassis/CtrlBoard-H7_IMU.ioc` 调用官方 `STM32CubeMX` CLI 生成 `runtime/generated/stm32h7_ctrl_board_raw`
- `build`: 默认构建 `balance_chassis_hw_seed.elf`
- `build hw_seed`: 构建底层静态库种子 `balance_chassis_bsp_seed`
- `build legacy_obj`: 编译迁移后的整套 `Chassis/User` 对象库 `balance_chassis_legacy_full_obj`
