---
phase: v2-04
plan: "01"
subsystem: control
tags: [pub-sub, topics, message-center, refactor]
dependency-graph:
  requires: [v2-03]
  provides: [centralized-topics, inline-pub-sub]
  affects: [v2-04-02, v2-04-03]
tech-stack:
  added: []
  patterns: [centralized-topic-constants, direct-pub-sub-in-tasks]
key-files:
  created:
    - robot_platform/runtime/control/topics.h
  modified:
    - robot_platform/CMakeLists.txt
    - robot_platform/runtime/control/state/ins_task.c
    - robot_platform/runtime/control/state/observe_task.c
    - robot_platform/runtime/control/state/observe_task.h
    - robot_platform/runtime/control/execution/motor_control_task.c
    - robot_platform/runtime/control/execution/motor_control_task.h
    - robot_platform/runtime/control/controllers/chassis_control_task.c
    - robot_platform/runtime/control/controllers/chassis_control_task.h
    - robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c
    - robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.h
    - robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c
    - robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.h
    - robot_platform/runtime/tests/host/test_support/balance_safety_harness.c
  deleted:
    - robot_platform/runtime/control/state/ins_topics.c
    - robot_platform/runtime/control/state/ins_topics.h
    - robot_platform/runtime/control/state/observe_topics.c
    - robot_platform/runtime/control/state/observe_topics.h
    - robot_platform/runtime/control/execution/actuator_topics.c
    - robot_platform/runtime/control/execution/actuator_topics.h
    - robot_platform/runtime/app/balance_chassis/app_io/remote_topics.c
    - robot_platform/runtime/app/balance_chassis/app_io/remote_topics.h
decisions:
  - id: D-v2-04-01-01
    description: "Preserve chassis_topics.c as observation module with new chassis_observation_on_publish() entry point"
  - id: D-v2-04-01-02
    description: "Inline bus structs into task runtime structs — no intermediate bus abstraction"
  - id: D-v2-04-01-03
    description: "Use relative includes for topics.h from task files rather than adding CMake include directory"
metrics:
  duration: ~7min
  completed: 2026-04-02
---

# Phase v2-04 Plan 01: Topic Wrapper Removal Summary

Centralized topic name constants into `control/topics.h` and inlined all PubRegister/SubRegister/PubPushMessage/SubGetMessage calls directly into task init/step functions, eliminating 8 topic wrapper files and 5 bus wrapper structs.

## What Changed

Each task previously delegated pub/sub registration and messaging to a dedicated `*_topics.c` wrapper file with its own bus struct. Now each task owns its `Publisher_t*`/`Subscriber_t*` pointers directly in its runtime struct and calls message_center APIs inline.

The observation instrumentation in `chassis_topics.c` (RuntimeOutput printf, first/latest command capture, observation count) was preserved by extracting it into a `chassis_observation_on_publish()` function called from `chassis_control_task.c` after each actuator command publish.

## Files Deleted (8)

- `ins_topics.c/h` — INS publisher wrapper
- `observe_topics.c/h` — observe bus wrapper (pub + 3 subs + readiness)
- `actuator_topics.c/h` — actuator bus wrapper (2 subs + pub + readiness)
- `remote_topics.c/h` — remote intent bus wrapper (pub + sub)

## Decisions Made

| ID | Decision | Rationale |
|----|----------|-----------|
| D-v2-04-01-01 | Keep chassis_topics.c as observation module | Tests and SITL runtime-output logging depend on observation instrumentation |
| D-v2-04-01-02 | Inline bus structs into task runtime structs | Removes indirection layer; each task directly owns its pub/sub handles |
| D-v2-04-01-03 | Relative includes for topics.h | Avoids adding new CMake include directory; consistent with existing include style |

## Deviations from Plan

None — plan executed exactly as written.

## Verification

- 10/10 CTest targets pass (including test_balance_safety_path)
- 40/40 Python CLI tests pass
- Clean build from scratch with no warnings
- Net deletion: 285 lines removed, 101 added (184 lines net reduction)
