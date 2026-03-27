# stm32h7_ctrl_board BSP

This directory is the first real BSP landing zone inside `robot_platform`.

Current imported seed files:

- `bsp_dwt.[ch]`
- `bsp_PWM.[ch]`
- `bsp_uart.[ch]`
- `can_bsp.[ch]`

These files are intentionally copied into the platform tree so later BSP
refactors can happen without editing the legacy `Chassis/` project in place.
