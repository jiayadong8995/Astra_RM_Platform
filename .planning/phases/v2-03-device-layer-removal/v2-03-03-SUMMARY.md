---
phase: v2-03
plan: "03"
subsystem: control-readiness
tags: [readiness-gates, wait-ready, startup-safety, extraction]
dependency-graph:
  requires: [v2-03-02]
  provides: [standalone-readiness-gates]
  affects: [v2-04]
tech-stack:
  added: []
  patterns: [subscriber-based-readiness-gate]
key-files:
  created:
    - robot_platform/runtime/control/readiness.h
    - robot_platform/runtime/control/readiness.c
  modified:
    - robot_platform/runtime/control/execution/actuator_topics.c
    - robot_platform/runtime/control/state/observe_topics.c
    - robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c
    - robot_platform/CMakeLists.txt
decisions:
  - id: D-v2-03-03-01
    summary: "Standalone readiness gates take raw Subscriber_t* pointers — no dependency on bus wrapper structs"
  - id: D-v2-03-03-02
    summary: "Original wait_ready functions kept as thin wrappers delegating to readiness.c — nothing breaks now, Phase 4 deletes wrappers"
metrics:
  duration: "5 minutes"
  completed: "2026-04-02"
---

# Phase v2-03 Plan 03: Extract wait_ready Startup Gates Summary

Standalone readiness gate functions extracted from topic wrapper files into `runtime/control/readiness.c/h`, taking raw `Subscriber_t*` pointers instead of bus structs. Original wrapper functions now delegate to the new standalone versions.

## Task 1: Extract wait_ready gates

Created two standalone gate functions:

- `platform_readiness_wait_ins` — busy-waits on an `ins_data` subscriber until `ins_msg->ready != 0`
- `platform_readiness_wait_ins_and_feedback` — busy-waits on both `ins_data` and `device_feedback` subscribers until INS ready AND feedback valid

Converted three topic wrapper `wait_ready` functions to thin delegating wrappers:

- `platform_actuator_bus_wait_ready` -> `platform_readiness_wait_ins`
- `platform_observe_bus_wait_ready` -> `platform_readiness_wait_ins`
- `chassis_runtime_bus_wait_ready` -> `platform_readiness_wait_ins_and_feedback`

Added `readiness.c` to both `BALANCE_CHASSIS_APP_SOURCES` and `BALANCE_SAFETY_HOST_RUNTIME_SOURCES` in CMakeLists.txt.

Commit: `8c536d2e`

## Deviations from Plan

None — plan executed exactly as written.

## Verification

All 12 CTest targets pass:

```
100% tests passed, 0 tests failed out of 12
```

Critical canary `test_balance_safety_path` green throughout.

## Next Phase Readiness

- Readiness gates are now standalone — no dependency on topic wrapper bus structs
- Topic wrapper `wait_ready` functions are thin wrappers ready for deletion in Phase 4
- Phase 4 can remove topic wrappers without losing startup safety gates
