# Phase 2: Hardware Seed Build

## Goal

Phase 2 does not try to link the whole `Chassis` firmware yet.

The first hardware milestone is narrower:

- keep `STM32CubeMX` responsible for raw peripheral code generation
- compile the generated STM32H7 code with `arm-none-eabi-gcc`
- seed the new platform with a small BSP slice that does not drag the full app stack in

## What Is Included

The current seed target is built from:

- `runtime/generated/stm32h7_ctrl_board_raw/Src/*.c` except `main.c` and `freertos.c`
- `Chassis/Core/Src/system_stm32h7xx.c`
- the minimal STM32H7 HAL source set needed by the generated code
- `Chassis/User/Bsp/bsp_dwt.c`
- `Chassis/User/Bsp/bsp_PWM.c`

## Why `main.c` And `freertos.c` Are Excluded

The current repository only carries the FreeRTOS portable layer under:

- `Chassis/Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM4F`

That is a Keil/RVDS CM4F port, not a GCC Cortex-M7 port. Until the correct GCC/CM7
portable layer is added, a full firmware ELF target would be misleading.

Phase 2 therefore focuses on a truthful milestone:

- generated code compiles under GCC
- HAL include paths and CPU flags are correct
- the new platform can start absorbing BSP code in controlled slices

## Current Targets

- `balance_chassis_generated_raw`
- `balance_chassis_bsp_seed`
- `balance_chassis_hw_seed.elf`

## Validation

Typical validation flow:

```bash
cmake -S robot_platform -B build/robot_platform \
  -DCMAKE_TOOLCHAIN_FILE=robot_platform/cmake/toolchains/arm-none-eabi-gcc.cmake

cmake --build build/robot_platform --target balance_chassis_bsp_seed
cmake --build build/robot_platform --target balance_chassis_hw_seed.elf
```

## Next Step

After this seed build is stable, Phase 2 should continue in this order:

1. keep the ELF target linkable after each generated-code refresh
2. import the next BSP devices
3. replace the seed app with migrated app/module code
4. start wiring flash/debug commands
