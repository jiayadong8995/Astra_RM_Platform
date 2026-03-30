# Technology Stack

**Analysis Date:** 2026-03-30

## Languages

**Primary:**
- C (C11 in toolchain) - Main runtime, hardware firmware, BSP, device, control, and SITL executable code under `robot_platform/runtime/` and `robot_platform/CMakeLists.txt`
- Python (version not pinned in repo) - Platform CLI, STM32CubeMX backend, SITL orchestration, smoke reporting, and tests under `robot_platform/tools/` and `robot_platform/sim/`

**Secondary:**
- CMake (minimum 3.22) - Top-level build definition in `robot_platform/CMakeLists.txt`
- YAML - Declarative board and project configuration in `robot_platform/projects/balance_chassis/project.yaml`, `robot_platform/projects/balance_chassis/board.yaml`, `robot_platform/tools/schema/project.schema.yaml`, and `robot_platform/tools/schema/board.schema.yaml`
- ASM - STM32 startup code compiled into the hardware ELF from `robot_platform/CMakeLists.txt`
- IOC project format - STM32CubeMX source input in `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc`

## Runtime

**Environment:**
- Embedded ARM Cortex-M7 firmware on STM32H723 hardware, configured in `robot_platform/projects/balance_chassis/board.yaml`
- Linux x86_64 SITL runtime, configured by `robot_platform/cmake/toolchains/linux-gcc.cmake`
- Python CLI execution environment for tooling and tests, invoked from `robot_platform/README.md` and implemented in `robot_platform/tools/platform_cli/main.py`

**Package Manager:**
- None detected for the active project surface under `robot_platform/`
- Lockfile: missing for the active project surface

## Frameworks

**Core:**
- STM32 HAL (vendored) - MCU peripheral access via `robot_platform/third_party/stm32_cube/Drivers/STM32H7xx_HAL_Driver`
- CMSIS and CMSIS-DSP (vendored) - Core and DSP support via `robot_platform/third_party/stm32_cube/Drivers/CMSIS`
- FreeRTOS (vendored) - Tasking and RTOS primitives via `robot_platform/third_party/stm32_cube/Middlewares/Third_Party/FreeRTOS/Source`

**Testing:**
- Python `unittest` - Minimal regression coverage for CLI and sim runner in `robot_platform/tools/platform_cli/tests/test_main.py` and `robot_platform/sim/tests/test_runner.py`

**Build/Dev:**
- CMake 3.22+ - Primary build system in `robot_platform/CMakeLists.txt`
- Ninja - Hardware build generator selected in `robot_platform/tools/platform_cli/main.py`
- Unix Makefiles - SITL build generator selected in `robot_platform/tools/platform_cli/main.py`
- `arm-none-eabi-gcc` - Cross-compiler in `robot_platform/cmake/toolchains/arm-none-eabi-gcc.cmake`
- GCC/G++ - Linux SITL compiler in `robot_platform/cmake/toolchains/linux-gcc.cmake`
- STM32CubeMX 6.17.0 - Code generation backend discovered in `robot_platform/tools/cubemx_backend/main.py`

## Key Dependencies

**Critical:**
- `robot_platform/third_party/stm32_cube` - Vendored STM32Cube assets: HAL, CMSIS, FreeRTOS, and generated-code dependencies used by `robot_platform/CMakeLists.txt`
- `robot_platform/third_party/freertos_port_gcc_cm7_r0p1` - Cortex-M7 FreeRTOS GCC port used by the hardware target in `robot_platform/CMakeLists.txt`
- `robot_platform/third_party/freertos_port_linux` - Linux FreeRTOS port used by the SITL target in `robot_platform/CMakeLists.txt`
- `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc` - Historical board definition consumed by the current `generate` flow in `robot_platform/tools/platform_cli/main.py`

**Infrastructure:**
- `robot_platform/tools/platform_cli/main.py` - Unified command entry point for `generate`, `build`, `sim`, and `test`
- `robot_platform/tools/cubemx_backend/main.py` - Wrapper around STM32CubeMX CLI, HOME override, and headless handling
- `robot_platform/sim/core/runner.py` - SITL session orchestration, subprocess launching, and smoke report generation
- `robot_platform/sim/reports/report_writer.py` - JSON report output to `build/sim_reports/`

## Configuration

**Environment:**
- No `.env` files detected at repository root or within the active project scan
- `STM32CUBEMX_BIN` can override the STM32CubeMX binary path in `robot_platform/tools/cubemx_backend/main.py`
- `DISPLAY` and `WAYLAND_DISPLAY` are checked to decide headless CubeMX behavior in `robot_platform/tools/cubemx_backend/main.py`
- `HOME` and `JAVA_TOOL_OPTIONS` are set by the CubeMX backend to isolate runtime state in `.cache/stm32_user_home`

**Build:**
- Top-level build graph: `robot_platform/CMakeLists.txt`
- Toolchains: `robot_platform/cmake/toolchains/arm-none-eabi-gcc.cmake` and `robot_platform/cmake/toolchains/linux-gcc.cmake`
- Board/project descriptors: `robot_platform/projects/balance_chassis/board.yaml` and `robot_platform/projects/balance_chassis/project.yaml`
- Config schemas: `robot_platform/tools/schema/board.schema.yaml` and `robot_platform/tools/schema/project.schema.yaml`
- Historical generated baseline: `Astra_RM2025_Balance/Chassis/Core/` and `Astra_RM2025_Balance/Chassis/Drivers/`

## Platform Requirements

**Development:**
- `python3`, `cmake`, `ninja`, `arm-none-eabi-gcc`, `java`, and `STM32CubeMX` are required by `robot_platform/docs/wsl_environment_setup.md`
- Linux or WSL environment for current documented workflows in `robot_platform/docs/wsl_environment_setup.md`
- Writable repository-local cache directories such as `.cache/stm32_user_home` and generated output under `robot_platform/runtime/generated/`

**Production:**
- STM32H723 target board defined by `robot_platform/projects/balance_chassis/board.yaml`
- Cortex-M7 floating-point settings and linker script configured in `robot_platform/CMakeLists.txt` and `robot_platform/cmake/linker/stm32h723_flash.ld`

---

*Stack analysis: 2026-03-30*
