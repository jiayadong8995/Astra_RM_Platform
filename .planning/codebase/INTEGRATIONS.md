# External Integrations

**Analysis Date:** 2026-03-30

## APIs & External Services

**Code Generation Tooling:**
- STM32CubeMX CLI - Generates board code from `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc` into `robot_platform/runtime/generated/stm32h7_ctrl_board_raw`
  - SDK/Client: Local binary invoked by `robot_platform/tools/cubemx_backend/main.py`
  - Auth: None

**Simulation Transport:**
- Local SITL UDP bridge - Exchanges IMU samples, motor commands, and motor feedback over localhost for the sim profile defined in `robot_platform/sim/projects/balance_chassis/profile.py`
  - SDK/Client: Python standard-library `socket` in `robot_platform/sim/backends/sitl_bridge.py`
  - Auth: None

**Hardware Interfaces:**
- STM32 peripheral buses - Board configuration declares `fdcan1`, `fdcan2`, `fdcan3`, `spi2`, `tim3`, `uart5`, and `usart1` in `robot_platform/projects/balance_chassis/board.yaml`
  - SDK/Client: STM32 HAL and CMSIS from `robot_platform/third_party/stm32_cube`
  - Auth: Not applicable

## Data Storage

**Databases:**
- None detected in the active project surface under `robot_platform/`
  - Connection: Not applicable
  - Client: Not applicable

**File Storage:**
- Local filesystem only
- Generated code is written to `robot_platform/runtime/generated/stm32h7_ctrl_board_raw`
- Smoke reports are written to `build/sim_reports/sitl_smoke.json` by `robot_platform/sim/reports/report_writer.py`
- CubeMX runtime state is written under `.cache/stm32_user_home` by `robot_platform/tools/cubemx_backend/main.py`

**Caching:**
- Local filesystem cache only via `.cache/stm32_user_home`

## Authentication & Identity

**Auth Provider:**
- None
  - Implementation: No user, service, token, or identity-provider flow is implemented in `robot_platform/` or its active tooling

## Monitoring & Observability

**Error Tracking:**
- None detected

**Logs:**
- CLI and sim processes log to stdout/stderr using `print(...)` in `robot_platform/tools/platform_cli/main.py`, `robot_platform/tools/cubemx_backend/main.py`, and `robot_platform/sim/backends/sitl_bridge.py`
- Bridge events are emitted as JSON log lines prefixed with `[BridgeEvent]` in `robot_platform/sim/backends/sitl_bridge.py`
- Smoke-session summaries are serialized to JSON by `robot_platform/sim/reports/report_writer.py`

## CI/CD & Deployment

**Hosting:**
- Embedded deployment target is the STM32H723 control board from `robot_platform/projects/balance_chassis/board.yaml`
- SITL execution target is a local Linux process built as `balance_chassis_sitl` from `robot_platform/CMakeLists.txt`

**CI Pipeline:**
- None detected for the active project surface

## Environment Configuration

**Required env vars:**
- `STM32CUBEMX_BIN` - Optional override for CubeMX binary discovery in `robot_platform/tools/cubemx_backend/main.py`
- `DISPLAY` / `WAYLAND_DISPLAY` - Checked to determine whether CubeMX should run in headless mode in `robot_platform/tools/cubemx_backend/main.py`
- `HOME` and `JAVA_TOOL_OPTIONS` - Set internally by the CubeMX backend when launching the generator from `robot_platform/tools/cubemx_backend/main.py`

**Secrets location:**
- No secrets store detected
- No `.env` files detected in the active repository scan

## Webhooks & Callbacks

**Incoming:**
- None

**Outgoing:**
- None

## Integration Notes

**Local Network Ports:**
- `127.0.0.1:9001` - IMU stream target from `robot_platform/sim/projects/balance_chassis/profile.py`
- `127.0.0.1:9002` - Motor feedback target from `robot_platform/sim/projects/balance_chassis/profile.py`
- `127.0.0.1:9003` - Motor command listener from `robot_platform/sim/projects/balance_chassis/profile.py`

**Concrete Device Integrations:**
- BMI088 IMU device integration is implemented under `robot_platform/runtime/device/imu/bmi088/` and `robot_platform/runtime/device/imu/bmi088_device_hw.c`
- DM4310 motor integration is implemented under `robot_platform/runtime/device/actuator/motor/dm4310/` and `robot_platform/runtime/device/actuator/motor/motor_actuator_device_hw.c`
- DBUS remote integration is implemented under `robot_platform/runtime/device/remote/dbus/` and `robot_platform/runtime/device/remote/dbus_remote_device_hw.c`

**Historical Baseline Dependency:**
- The active code generation path depends on the legacy baseline asset `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc`
- The repository root `README.md` states that `Astra_RM2025_Balance/` is historical baseline material and `robot_platform/` is the current engineering entry point

---

*Integration audit: 2026-03-30*
