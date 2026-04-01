# Feature Landscape: v2 Platform Simplification

**Domain:** Embedded robot platform simplification for a wheeled-legged target
**Researched:** 2026-04-01
**Context:** v1 complete (24/24 requirements, 11 CTest targets, 6 safety gates). v2 goal is reducing over-engineered complexity while preserving verification capability.

## Current Complexity Baseline (Evidence)

Before defining features, here is what the simplification is working against:

| Metric | Current v1 | Reference (basic_framework) | Delta |
|--------|-----------|---------------------------|-------|
| Runtime layers | 5-6 (generated, bsp, device, control, module, app) | 4 (bsp, modules, application, generated) | 1-2 extra layers |
| Device layer files | 29 files, 2806 lines | 0 (devices live inside modules) | Entire layer is indirection |
| Device layer depth | 4 directory levels (device/actuator/motor/dm4310) | 1 level (modules/motor) | 3 extra nesting levels |
| Topic wrapper boilerplate | 10 files, 329 lines across 5 bus structs | 0 (inline PubRegister/SubGetMessage) | 329 lines of pure ceremony |
| Pub/sub topics (production) | 6 topics, 16 registrations across 5 wrapper files | 3-4 topics, inline in application code | Similar topic count, 5x more plumbing |
| Device profile indirection | device_profile.h + 2 profile .c files + device_layer.c (662 lines) with vtable dispatch, global singleton, test hooks | Compile-time #ifdef for board variants | Runtime vtable vs compile-time selection |
| Test support stubs for device layer | device_layer_stubs.c/h (86 lines) duplicating the device_layer API | N/A (no test infrastructure) | Stub maintenance cost |

## Table Stakes

Features that v2 must deliver. Missing any of these means the simplification failed.

| Feature | Why Required | Complexity | Evidence |
|---------|-------------|------------|---------|
| Flatten device layer into per-device-type modules | The device_layer/device_profile indirection adds 662 lines of vtable dispatch, global singleton management, and profile binding for 3 devices (IMU, remote, motor). basic_framework and StandardRobotpp both use per-device-type structs with compile-time backend selection. The indirection provides no runtime benefit for a single-target platform. | High | 29 files / 2806 lines in `runtime/device/`. Profile dispatch in `device_layer.c` lines 400-428 has redundant `#ifdef SITL_BUILD` branches. |
| Remove topic wrapper boilerplate, use message_center inline | 5 bus struct types and 10 wrapper files (329 lines) exist solely to hide `PubRegister`/`SubGetMessage` calls. basic_framework calls these directly in application code. The wrappers add indirection without type safety or runtime benefit. | Med | `chassis_topics.c` is 102 lines for 6 registrations + observation globals. `ins_topics.c` is 11 lines for 1 registration. The wrapper pattern scales linearly with topics. |
| Narrow message_center usage to genuinely cross-task scenarios | Current topology has 6 topics with 16 registrations. Some are genuinely cross-task (ins_data: published by INS task, consumed by 3 other tasks). Others could be direct struct passing within a single task's step function. Narrowing means keeping pub/sub only where tasks truly run on different FreeRTOS threads. | Med | `ins_data` has 1 publisher and 3 subscribers across 3 tasks — genuinely cross-task. `chassis_observe` has 1 pub and 1 sub — could be a direct function call if observe and chassis run in the same task. |
| Consolidate to 4 layers: bsp, modules, control, app | v1 has generated + bsp + device + control + module + app. The device layer should merge into modules (per-device structs). The module layer's algorithms already belong alongside control. Target: bsp (hardware), modules (devices + algorithms + message_center), control (controllers + state estimation), app (project-specific composition). | High | basic_framework uses bsp/modules/application. StandardRobotpp uses bsp/modules/app. XRobot uses a flat module structure. All reference projects converge on 3-4 layers. |
| Preserve all 11 CTest host-test targets with equivalent coverage | v1 verification is the project's core value. Every existing test must either survive the refactor unchanged, be refactored to test the simplified equivalent, or be replaced by a test that covers the same safety property. Zero net loss of SAFE-01..06 coverage. | High | 11 test targets in `runtime/tests/host/`. `balance_safety_harness` depends on current task runtime types. `device_layer_stubs` depends on the device_layer API being removed. Both need migration. |
| Preserve SITL/HW backend selection | The ability to build the same control code for both STM32 hardware and Linux SITL must survive. The mechanism can change (vtable dispatch -> compile-time selection), but the capability cannot be lost. | Med | Current: runtime vtable in `device_layer.c`. Target: `#ifdef SITL_BUILD` or CMake target-based source selection, matching basic_framework's `#ifdef CHASSIS_BOARD` pattern. |
| Maintain validate pipeline end-to-end | The `python3 -m robot_platform.tools.platform_cli.main validate` 5-stage pipeline must continue to work. Internal test target names may change, but the pipeline stages (freshness, build, host_tests, sim, closure) must remain functional. | Med | Pipeline defined in `main.py`. References CTest target names that will change if test files are renamed/restructured. |
| Simplify test hook mechanism | Current `platform_device_test_hooks_t` (5 function pointers + context) exists because the device_layer global singleton needs runtime injection for tests. If device access becomes direct per-device calls, test hooks can become simpler compile-time stubs or link-time substitution. | Med | `device_layer.h` lines 21-36 define 5 hook typedefs. `device_layer.c` lines 190-296 implement hook dispatch for every device operation. This entire mechanism exists only for testability of the singleton. |

## Differentiators

Features that make this simplification genuinely valuable beyond just "fewer files."

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Reduced cognitive load for new contributors | basic_framework is the most-forked RM framework because its 4-layer structure is immediately legible. Matching that structure means new team members can orient in minutes instead of tracing vtable dispatch chains. | Low | This is the primary human value of the simplification. Measured by: can a new developer trace "remote input -> controller -> motor output" without reading device_profile.h? |
| Direct struct-passing where pub/sub is unnecessary | Replacing `chassis_observe` topic (1 pub, 1 sub) with a direct function return eliminates a string-keyed runtime lookup, a memcpy, and a generation counter check per control cycle. For a 1kHz control loop, this matters. | Low | `observe_topics.c` exists for a single producer-consumer pair. Direct passing is both simpler and faster. |
| Test stubs become trivially simple | Without the device_layer singleton, test stubs become per-device mock files selected at link time. No hook registration, no global state reset, no 5-pointer struct. The `balance_safety_harness` can inject data by directly calling device read functions with test implementations. | Med | Current: 86 lines of stub code + hook registration ceremony in every test. Target: link-time substitution with ~20-line stub files per device type. |
| Contracts survive as the stable API boundary | The `runtime/control/contracts/` headers (device_input.h, robot_state.h, actuator_command.h, etc.) are already well-designed and layer-neutral. Making them the primary interface between modules and control — instead of routing through device_layer — elevates them from "internal contract" to "the API." | Low | 7 contract headers totaling ~200 lines. These are already the right abstraction; the simplification just removes the indirection that sits between them and their consumers. |
| Easier path to second robot target | Paradoxically, removing the over-engineered device_layer makes adding a second target easier. The current vtable dispatch is tightly coupled to balance_chassis's specific 3-device set. Per-device modules with compile-time selection are more composable for a different robot with different devices. | Med | basic_framework supports mecanum, balance, and steering chassis types through compile-time selection in `robot_def.h`, not runtime vtables. |

## Anti-Features

Things to deliberately NOT do during v2 simplification.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| Rewrite message_center itself | The message_center implementation (156 lines) is correct, tested, and adequate. The problem is not the pub/sub mechanism — it's the wrapper boilerplate and overuse. Rewriting the core would risk regressions in a proven module. | Keep message_center.c/.h unchanged. Remove the wrapper files. Call PubRegister/SubGetMessage directly where cross-task communication is needed. |
| Introduce a new abstraction layer to replace device_layer | The goal is fewer layers, not different layers. Adding a "hardware abstraction module" or "device manager" to replace device_layer would be the same mistake with a new name. | Dissolve device_layer into per-device modules (imu_module, motor_module, remote_module) that own their own HW/SITL backends via compile-time selection. |
| Remove contracts or merge them into device code | The contract headers are the cleanest part of the current architecture. They define the data shapes that flow between layers and are the foundation of the test harness. | Keep contracts as-is. They become more prominent when the device_layer indirection is removed. |
| Simplify by removing SITL capability | SITL is the foundation of the fake-link validation that v1 built. Removing it would destroy the project's core value proposition. | Simplify the SITL backend selection mechanism (compile-time instead of runtime vtable), but preserve the SITL build target and smoke pipeline. |
| Merge all tasks into a single super-loop | FreeRTOS task separation exists for real-time scheduling reasons (INS at realtime priority, chassis at above-normal). Collapsing to a super-loop would lose priority-based preemption. | Keep task separation where priority differences exist. Consider merging tasks that run at the same priority and period (e.g., observe + chassis if they share timing). |
| Add new features during simplification | v2 is a refactoring milestone, not a feature milestone. Adding new control modes, new device types, or new safety gates during the restructuring creates a moving target that makes verification impossible. | Pure structural change with test equivalence. New features belong in v3. |
| Over-flatten into a single directory | basic_framework's flat `modules/` directory works at small scale but becomes hard to navigate with 20+ modules. Some grouping is still valuable. | Use 4 top-level layers with 1 level of grouping inside modules (e.g., modules/imu/, modules/motor/, modules/algorithm/, modules/message_center/). No deeper nesting. |
| Remove observation/instrumentation from chassis_topics | The `g_actuator_command_observed` / `g_first_actuator_command` observation mechanism in `chassis_topics.c` supports SITL runtime output capture. This is verification infrastructure, not boilerplate. | Migrate the observation mechanism to the chassis control task or a dedicated observation module. Don't silently drop it. |
| Break the validate pipeline "temporarily" | Simplification will rename files and restructure tests. If the validate pipeline breaks during the process and is fixed "later," it will stay broken. | Refactor in phases where each phase ends with a green validate run. Never merge a phase that breaks the pipeline. |

## Feature Dependencies

```text
Flatten device_layer into per-device modules
  -> Migrate device_layer_stubs to per-device test stubs
  -> Update balance_safety_harness to use new device API
  -> Update SITL backend selection (compile-time)

Remove topic wrapper boilerplate
  -> Migrate observation mechanism from chassis_topics.c
  -> Update task runtime structs (remove bus struct members)
  -> Update test harness (direct topic registration in tests)

Consolidate to 4 layers
  -> Depends on: device_layer flattened + topic wrappers removed
  -> Move algorithm modules alongside control
  -> Update CMakeLists.txt source lists and include paths

Preserve test coverage
  -> Depends on: all structural changes above
  -> Each structural change must include test migration
  -> Validate pipeline must pass after each phase
```

## Simplification Outcome Metrics

How to measure whether v2 succeeded:

| Metric | v1 Current | v2 Target | How to Verify |
|--------|-----------|-----------|---------------|
| Runtime layer count | 5-6 | 4 | Count top-level directories under runtime/ |
| Device indirection files | 29 files, 2806 lines | ~12 files, ~1200 lines (drivers + per-device modules) | `find runtime/modules -path '*/device*' \| wc -l` |
| Topic wrapper files | 10 files, 329 lines | 0 | `find runtime -name '*topics*' \| wc -l` |
| device_layer.c + device_profile.h | 662 lines | 0 (deleted) | File should not exist |
| Test targets passing | 11/11 | 11/11 (may be renamed) | `ctest --test-dir build/sitl` |
| Safety gates (SAFE-01..06) | All pass | All pass | `validate` pipeline Stage 2 |
| Validate pipeline | 5 stages, all pass | 5 stages, all pass | `python3 -m robot_platform.tools.platform_cli.main validate` |
| Max directory nesting under runtime/ | 4 levels (device/actuator/motor/dm4310) | 2 levels (modules/motor/) | `find runtime -type d \| awk -F/ '{print NF}' \| sort -n \| tail -1` |

## Phase Ordering Recommendation

Based on dependency analysis:

1. **Flatten device_layer** first — it's the largest structural change and unblocks test migration
2. **Narrow message_center usage and remove topic wrappers** — depends on understanding which topics survive device_layer removal
3. **Consolidate directory structure to 4 layers** — mechanical restructuring after the semantic changes are done
4. **Test migration and validate pipeline update** — should happen incrementally within each phase, with a final verification pass

## Sources

- Codebase analysis: direct file reading and line counting of `robot_platform/runtime/` (HIGH confidence)
- Reference project: `references/external/basic_framework/` application and module structure (HIGH confidence — code is in-tree)
- v1 requirements and milestone audit: `.planning/milestones/v1-REQUIREMENTS.md`, `.planning/v1-MILESTONE-AUDIT.md` (HIGH confidence)
- v1 concerns: `.planning/codebase/CONCERNS.md` (HIGH confidence)
- basic_framework pub/sub pattern: `references/external/basic_framework/application/chassis/chassis.c`, `references/external/basic_framework/application/cmd/robot_cmd.c` (HIGH confidence — inline PubRegister/SubGetMessage usage observed)
