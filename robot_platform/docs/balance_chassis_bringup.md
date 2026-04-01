# Balance Chassis Authoritative Bring-Up

This document is the single authoritative bring-up reference for `balance_chassis`.

## Authoritative Paths

- Hardware: `main.c -> MX_FREERTOS_Init() -> balance_chassis_app_startup() -> scheduler`
- SITL: `main_sitl.c -> balance_chassis_app_startup() -> scheduler`
- Shared startup API: `balance_chassis_app_startup()`

Both hardware and SITL keep their own host entrypoints, but they converge on the same project-owned startup seam before the scheduler takes over. That shared seam is the blessed `balance_chassis` bring-up path.

## Ownership Split

- `app` owns project composition, remote intent ingress, mode/business assembly, and startup wiring.
- `control` owns state observation, chassis control, runtime execution output, and the main observe -> control -> execution chain.
- `device` remains the semantic adapter layer between backend-specific access and control/runtime consumers.

This split keeps `balance_chassis` as the proving path for the reusable platform instead of turning it into a bypass around `runtime/device` or `runtime/control`.

## Legacy Demotion

`runtime/app/balance_chassis/app_bringup/freertos_app.c` is a legacy compatibility surface. `freertos_app.c` is not an authoritative startup owner anymore and should not be used as the source of truth for current bring-up.

## Developer Guidance

- Use the `verify phase3` JSON artifact as the machine-readable proof surface for authoritative bring-up metadata.
- Use this document when you need the human-readable statement of which startup path is blessed and which legacy path is demoted.
