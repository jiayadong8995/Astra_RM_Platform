# Deferred Items

## 2026-04-01

- `04-01-PLAN.md` hardware verification still uses host `cc` for `build/robot_platform_hw_make`, so Cortex-M flags like `-mthumb` and `-mfloat-abi=hard` fail before link. This blocked the plan's hardware build verify step, but it is a pre-existing toolchain/configuration issue rather than a regression introduced by startup-authority changes.
- `04-03-PLAN.md` stub scan found the pre-existing `print(f"command placeholder: {cmd}")` fallback in `robot_platform/tools/platform_cli/main.py`. It is unrelated to authoritative bring-up metadata and docs, so it was left unchanged for a later CLI-hardening pass.
