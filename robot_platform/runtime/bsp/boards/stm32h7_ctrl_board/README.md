# stm32h7_ctrl_board BSP

当前板级层只保留真实底层访问能力：

- `bsp_dwt.[ch]`
- `bsp_PWM.[ch]`
- `bsp_uart.[ch]`
- `can_bsp.[ch]`

这里不再承接：

- 控制逻辑
- 业务状态
- app 级编排
