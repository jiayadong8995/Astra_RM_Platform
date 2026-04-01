# Deferred Items

## 2026-04-01

- `04-01-PLAN.md` hardware verification still uses host `cc` for `build/robot_platform_hw_make`, so Cortex-M flags like `-mthumb` and `-mfloat-abi=hard` fail before link. This blocked the plan's hardware build verify step, but it is a pre-existing toolchain/configuration issue rather than a regression introduced by startup-authority changes.
