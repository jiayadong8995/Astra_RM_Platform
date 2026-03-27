# Chassis Import Manifest

本文档为第二阶段准备，定义 `Chassis` 如何导入到 `robot_platform`。

## 1. 导入目标

先导入最小编译链，不做大规模行为重写。

## 2. 分层映射

### generated

来源目录：

- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/Core`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/Drivers`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/Middlewares`

目标目录：

```text
runtime/generated/stm32h7_ctrl_board/
```

### bsp

来源目录：

- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/User/Bsp`

目标目录：

```text
runtime/bsp/boards/stm32h7_ctrl_board/
```

### module

来源目录：

- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/User/Algorithm`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/User/Controller`
- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/User/modules`

目标目录：

```text
runtime/module/
```

### app

来源目录：

- `/home/xbd/worspcae/code/Astra_RM2025_Balance/Chassis/User/APP`

目标目录：

```text
runtime/app/balance_chassis/
```

## 3. 第二阶段优先导入文件

### generated 最小集合

- `Core/Inc/*`
- `Core/Src/main.c`
- `Core/Src/freertos.c`
- `Core/Src/gpio.c`
- `Core/Src/dma.c`
- `Core/Src/fdcan.c`
- `Core/Src/spi.c`
- `Core/Src/tim.c`
- `Core/Src/usart.c`
- `Core/Src/stm32h7xx_it.c`

### bsp 最小集合

- `User/Bsp/bsp_dwt.*`
- `User/Bsp/bsp_PWM.*`
- `User/Bsp/bsp_uart.*`
- `User/Bsp/can_bsp.*`

### app 最小集合

- `User/APP/robot_def.h`
- `User/APP/INS_task.*`
- `User/APP/remote_task.*`
- `User/APP/observe_task.*`
- `User/APP/chassis_task.*`
- `User/APP/motor_control_task.*`

## 4. 导入阶段策略

### Phase 2A

只保证能编译，不改逻辑边界。

当前已完成的种子导入：

- `bsp_dwt.*`
- `bsp_PWM.*`
- `bsp_uart.*`
- `remote_control.*`
- `can_bsp.*`
- `dm4310_drv.*`

当前已完成的整套代码落位：

- `runtime/app/balance_chassis/` 已承接原 `Chassis/User/APP`
- `runtime/module/algorithm/` 已承接原 `Chassis/User/Algorithm`
- `runtime/module/controller/` 已承接原 `Chassis/User/Controller`
- `runtime/module/lib/` 已承接原 `Chassis/User/Lib`
- `runtime/module/message_center/` 已承接原 `Chassis/User/modules/message_center`
- `runtime/bsp/devices/bmi088/` 已承接原 `Chassis/User/Devices/BMI088`

当前编译验证：

- `balance_chassis_legacy_full_obj` 可完成整套 `Chassis/User` 迁移代码的对象级编译
- `balance_chassis_legacy_full.elf` 已可完成整套 legacy `Chassis` 迁移代码的最终固件链接
- `balance_chassis_legacy_full.bin` 与 `balance_chassis_legacy_full.map` 已同步产出

### Phase 2B

把 `Bsp`、`Algorithm`、`APP` 逐步改成 `generated / bsp / module / app` 明确依赖。

### Phase 2C

接入 replay。
