# Domain Pitfalls: v2 Platform Simplification

**Domain:** Simplifying an over-engineered embedded robot platform from 5-6 layers to 4 layers
**Researched:** 2026-04-01
**Context:** v1 shipped 24/24 requirements. v2 targets complexity reduction while preserving safety gates, testability, and HW/SITL backend distinction.
**Overall confidence:** HIGH (based on direct codebase analysis of all affected files, call sites, and test dependencies)

---

## Critical Pitfalls

Mistakes that cause safety regressions, broken test suites, or forced rewrites.

---

### Pitfall 1: Removing device_layer test hooks destroys the entire safety test suite

**What goes wrong:** The `platform_device_test_hooks_t` mechanism in `device_layer.c` is the sole injection seam that 7 out of 11 host CTest targets depend on. Removing or restructuring `device_layer` without preserving an equivalent hook surface breaks every safety test simultaneously.

**Why it happens:** The device_layer/device_profile indirection looks like unnecessary abstraction when you read it in isolation. The temptation is to inline device calls directly into tasks. But the test hooks (`platform_device_set_test_hooks` / `platform_device_reset_test_hooks`) are not part of the "indirection" — they are the testability seam.

**Concrete evidence:**
- `test_balance_safety_path.c` — uses `platform_device_set_test_hooks` (line 113)
- `test_device_profile_safety_seams.c` — uses all 5 hook functions (lines 73-99)
- `test_safety_arming.c` — uses hooks (line 72)
- `test_safety_mapping.c` — uses hooks (line 71)
- `test_safety_saturation.c` — uses hooks (line 71)
- `test_safety_sensor_faults.c` — uses hooks (line 73)
- `test_safety_wheel_leg.c` — uses hooks (line 60)
- `test_actuator_gateway.c` — uses `device_layer_stubs.c` which replaces the same default API functions

**Consequences:** All safety tests fail. CI goes red. Team either reverts the simplification or (worse) deletes the tests to make CI green, losing the v1 safety coverage.

**Prevention:**
1. Before touching device_layer, catalog every test that calls `platform_device_set_test_hooks`, `platform_device_reset_test_hooks`, or links against `device_layer_stubs.c`
2. Design the replacement injection seam first, migrate one test to it, verify it passes, then proceed
3. The new seam can be simpler (direct function pointers in a struct passed to tasks, or compile-time weak symbols) but it must support the same 5 operations: init, read_remote, read_imu, read_feedback, write_command

**Detection:** Any refactoring PR that modifies `device_layer.h` or `device_layer.c` without updating all 7+ test files is a red flag.

**Which phase should address it:** Must be the very first simplification phase. Test seam migration before any structural changes.

---

### Pitfall 2: Collapsing layers breaks the HW/SITL backend distinction

**What goes wrong:** The current `device_profile_t` + `device_layer_t` pattern is what allows the same runtime code to bind to either `device_profile_hw()` or `device_profile_sitl()` at init time. Removing this indirection without a replacement means the `#ifdef SITL_BUILD` conditionals must spread into every call site, or the SITL target stops compiling.

**Concrete evidence:**
- `device_profile.h` declares `platform_device_profile_hw()` and `platform_device_profile_sitl()` (lines 15-16)
- `device_layer.c:platform_select_profile()` uses `#ifdef SITL_BUILD` in exactly one place (lines 400-428) to route to the correct profile
- `device_profile_sitl.c` binds IMU/remote/motor via `bind_sitl_imu`, `bind_sitl_remote`, `bind_sitl_motor`
- `device_profile_hw.c` does the same for hardware
- CMakeLists.txt compiles different device source sets for `PLATFORM_TARGET_HW` vs `PLATFORM_TARGET_SIM` (lines 520-538)
- The SITL target links `balance_chassis_device` which includes `device_profile_sitl.c` + `device_layer.c`

**Why it happens:** The profile/layer split feels like over-engineering because there are only 2 profiles and 3 devices. But the split is doing real work: it keeps `#ifdef` out of task code and lets tests bind without any `#ifdef` at all.

**Consequences:** Either SITL stops building, or `#ifdef SITL_BUILD` proliferates into `ins_task.c`, `remote_task.c`, `actuator_gateway.c`, and `motor_control_task.c`. The latter makes the code harder to test and harder to read — the opposite of simplification.

**Prevention:**
1. The replacement for device_profile does not need to be a vtable pattern. It can be as simple as: compile different `.c` files that implement the same function signatures (link-time polymorphism). This is what `device_layer_stubs.c` already does for tests.
2. Keep the "one place that decides HW vs SITL" principle. Move it from runtime profile selection to CMake link-time selection if desired, but do not scatter it.
3. Verify both `build hw_elf` and `build sitl` compile and link after every structural change.

**Detection:** Any PR that removes `device_profile.h` without showing that both HW and SITL targets still build and pass tests.

**Which phase should address it:** Same phase as Pitfall 1. Design the replacement backend-selection mechanism before removing the current one.

---

### Pitfall 3: Removing message_center topics that the balance_safety_harness depends on

**What goes wrong:** The `balance_safety_harness.c` test infrastructure manually orchestrates the full task pipeline (remote -> observe -> chassis -> motor_control) by calling `_init`, `_prepare`, and `_step` functions in sequence. This harness depends on message_center topics being registered and functional. Removing topics that seem "unnecessary" breaks the harness and all 6 safety tests that use it.

**Concrete evidence — the topic dependency graph:**

| Topic | Publisher | Subscribers | Cross-task? |
|-------|-----------|-------------|-------------|
| `ins_data` | INS task (ins_topics.c) | observe_topics, chassis_topics, actuator_topics, balance_safety_harness | YES — 1 producer, 4 consumers |
| `robot_intent` | remote_topics.c | observe_topics, chassis_topics | YES — 1 producer, 2 consumers |
| `chassis_observe` | observe_topics.c | chassis_topics | YES — 1 producer, 1 consumer |
| `device_feedback` | actuator_topics.c | observe_topics, chassis_topics | YES — 1 producer, 2 consumers |
| `robot_state` | chassis_topics.c | remote_topics.c, test harnesses | YES — 1 producer, 1+ consumers |
| `actuator_command` | chassis_topics.c | actuator_topics.c | YES — 1 producer, 1 consumer |

**Why it happens:** When simplifying, you look at topics like `chassis_observe` (1 producer, 1 consumer) and think "this could be a direct function call." That's true in production. But the harness needs to step tasks independently and observe intermediate state, which requires the decoupled pub/sub.

**Consequences:** Replacing pub/sub with direct calls for some topics means the harness can no longer step tasks independently. Either the harness must be rewritten (expensive, error-prone) or the tests lose their ability to inject faults at intermediate points.

**Prevention:**
1. Map the full topic graph (above) before deciding which topics to remove
2. For each topic candidate for removal, check: does `balance_safety_harness.c` or any test depend on subscribing to or publishing this topic independently?
3. Topics with fan-out > 1 (`ins_data`, `robot_intent`, `device_feedback`) are genuinely cross-task and should stay
4. Topics with fan-out = 1 (`chassis_observe`, `actuator_command`) are candidates for direct calls, but only if the harness is updated to handle the tighter coupling

**Detection:** Removing a topic registration from any `*_topics.c` file without grep-checking all `SubRegister`/`PubRegister` call sites including test files.

**Which phase should address it:** After test seam migration (Pitfall 1). Narrow message_center only after the test infrastructure is stable on the new seam.

---

### Pitfall 4: Simplification turns into a rewrite — scope creep through "while we're at it"

**What goes wrong:** The team starts removing device_layer indirection, then notices the actuator_gateway command mapping is verbose (354 lines of field-by-field copy), then decides to restructure the contract types, then realizes the balance_controller needs updating too, and suddenly the entire runtime is being rewritten in one branch.

**Concrete evidence of coupling that invites scope creep:**
- `device_layer.c:platform_map_device_command_to_motor_set` (lines 298-354) — 56 lines of manual field mapping that begs to be simplified
- `actuator_gateway.c:platform_map_contract_command` (lines 30-106) — another 76 lines of field mapping
- `balance_controller.c` — large file that touches contracts, constraints, and motor output shapes
- Contract headers in `control/contracts/` — 7 header files defining the data shapes everything depends on

**Why it happens:** Embedded C codebases have high coupling through struct layouts. Changing one struct ripples through every file that touches it. The "simplify device_layer" goal naturally pulls you into "simplify the command mapping" which pulls you into "simplify the contracts."

**Consequences:** A multi-week branch that touches 40+ files, cannot be reviewed incrementally, and has a high probability of introducing subtle control bugs that only manifest on hardware.

**Prevention:**
1. Define a strict scope boundary for each simplification phase: "this phase changes these files and no others"
2. Use the existing test suite as a regression gate — if a change requires modifying tests beyond updating the injection seam, it's scope creep
3. Resist the urge to clean up command mapping or contract shapes in the same phase as layer removal
4. Each PR should be reviewable in under 30 minutes

**Detection:** A branch that has been open for more than a week, touches more than 15 files, or requires contract header changes.

**Which phase should address it:** This is a process discipline that applies to every phase. Define scope boundaries in the phase plan and enforce them.

---

### Pitfall 5: Losing the "default safe" startup guarantee

**What goes wrong:** The current startup sequence has explicit readiness gates: `chassis_runtime_bus_wait_ready` blocks until INS is ready and feedback is valid. `platform_actuator_bus_wait_ready` blocks until INS publishes. These gates exist because the balance controller will compute garbage outputs if fed zero-initialized state. During simplification, if tasks are restructured or topics are replaced with direct calls, these gates can be accidentally removed or short-circuited.

**Concrete evidence:**
- `chassis_topics.c:chassis_runtime_bus_wait_ready` (lines 23-34) — blocks on `ins->ready` AND `feedback->actuator_feedback.valid`
- `actuator_topics.c:platform_actuator_bus_wait_ready` (lines 12-25) — blocks on `ins_msg->ready`
- `observe_topics.c:platform_observe_bus_wait_ready` (lines 13-20) — blocks on `ins_msg->ready`
- `test_balance_safety_path.c` — the primary test that verifies the startup path produces safe (zeroed, disabled) outputs

**Why it happens:** Readiness gates are embedded in the topic bus wrappers. If you remove the topic bus wrappers (because you're simplifying message_center usage), the gates go with them unless explicitly preserved.

**Consequences:** The balance controller runs on zero-initialized IMU data, computes non-zero torque outputs, and the robot moves unpredictably on first power-up. This is the exact "crazy robot" scenario the v1 safety gates were designed to prevent.

**Prevention:**
1. Extract readiness gates from topic wrappers into explicit, named functions before simplifying the topic layer
2. `test_balance_safety_path` must pass at every intermediate step of the simplification
3. Add a new test that verifies: "if INS has not published, chassis_task_step produces zero actuator commands with control_enable=false and actuator_enable=false"

**Detection:** Any change to `*_topics.c` files that removes `wait_ready` functions without relocating the logic.

**Which phase should address it:** Must be verified in every phase. The `test_balance_safety_path` test is the canary — it must never go red during simplification.

---

## Moderate Pitfalls

Mistakes that cause delays or technical debt.

---

### Pitfall 6: CMakeLists.txt becomes unmaintainable during structural changes

**What goes wrong:** The current CMakeLists.txt is 989 lines with explicit source file lists, per-target include directories, and duplicated sanitizer configuration for each of the 11 test targets. Moving or renaming files during simplification requires updating multiple places in this file, and missing one causes cryptic linker errors.

**Concrete evidence:**
- `BALANCE_SAFETY_HOST_RUNTIME_SOURCES` (lines 218-250) lists 30+ source files explicitly
- Each test target repeats sanitizer options (8 test targets x ~10 lines each = 80 lines of duplication)
- `balance_safety_host_runtime` include directories (lines 297-337) lists 36 directories

**Prevention:**
1. Consider introducing a CMake helper function/macro for test targets to reduce duplication before the structural changes
2. When moving files, update CMakeLists.txt in the same commit — never in a follow-up
3. Run both `build host_tests` and `build hw_elf` after every CMakeLists.txt change

**Detection:** Linker errors mentioning undefined references to functions that clearly exist in the source tree.

**Which phase should address it:** First phase, as a preparatory step before structural changes.

---

### Pitfall 7: Removing the device_layer "default" singleton pattern without a replacement

**What goes wrong:** The `g_platform_default_layer` singleton in `device_layer.c` is what allows task code to call `platform_device_read_default_imu()` without passing a device handle. This is a convenience pattern that keeps task code clean. Removing it means either: (a) every task must receive and store a device handle, or (b) a new global access pattern must be introduced.

**Concrete call sites in production code (not tests):**
- `ins_task.c:31` — `platform_device_read_default_imu(&sample)`
- `remote_task.c:44` — `platform_device_read_default_remote(&runtime->rc_input)`
- `actuator_gateway.c:12` — `platform_device_init_default_profile()`
- `actuator_gateway.c:17` — `platform_device_read_default_feedback(feedback_msg)`
- `actuator_gateway.c:27` — `platform_device_write_default_command(&device_command)`

**Prevention:**
1. If replacing with explicit handle passing: update all 3 task files + actuator_gateway simultaneously
2. If replacing with link-time polymorphism: create the replacement `.c` files (one for HW, one for SITL, one for test stubs) that implement the same function signatures without the singleton
3. Option 2 is simpler and preserves the current call-site signatures

**Detection:** Compilation errors in task files after device_layer changes.

**Which phase should address it:** Core simplification phase, after test seam migration.

---

### Pitfall 8: Test harness becomes the bottleneck for all changes

**What goes wrong:** The `balance_safety_harness` is a monolithic test fixture that initializes all 4 task runtimes, seeds INS, and steps the entire pipeline. If the simplification changes any task's init/prepare/step signature, the harness and all 6 tests using it must be updated atomically.

**Concrete evidence:**
- `balance_safety_harness.h` includes headers from `app_bringup`, `control/controllers`, `control/execution`, and `control/state` (lines 4-8)
- `balance_safety_harness.c:platform_balance_safety_harness_init` calls `remote_task_init`, `observe_task_init`, `motor_control_task_init`, `chassis_task_init`, `observe_task_prepare`, `motor_control_task_prepare`, `motor_control_task_step`, `chassis_task_prepare` (lines 19-44)
- Tests using it: `test_balance_safety_path`, `test_safety_arming`, `test_safety_mapping`, `test_safety_saturation`, `test_safety_sensor_faults`, `test_safety_wheel_leg`

**Prevention:**
1. Update the harness in a dedicated commit before changing task signatures
2. Consider splitting the harness into smaller composable fixtures if the simplification changes task boundaries
3. Never change a task's public API and the harness in the same commit — it makes failures ambiguous

**Detection:** Multiple test failures that all trace back to `balance_safety_harness_init` or `balance_safety_harness_step_once`.

**Which phase should address it:** Addressed incrementally as task signatures change.

---

### Pitfall 9: Narrowing message_center without understanding the fan-out pattern

**What goes wrong:** Someone looks at `MSG_MAX_TOPICS = 8` and `MSG_MAX_SUBSCRIBERS = 16` and decides to reduce these limits as part of simplification. But the current topic graph uses 6 topics with some having 3-4 subscribers, and the test harness registers additional publishers. Reducing limits causes silent `NULL` returns from `PubRegister`/`SubRegister` at runtime.

**Concrete topic count:**
- `ins_data` — 1 pub, 3 subs (observe, chassis, actuator) + test harness pub
- `robot_intent` — 1 pub, 2 subs (observe, chassis)
- `chassis_observe` — 1 pub, 1 sub (chassis)
- `device_feedback` — 1 pub, 2 subs (observe, chassis)
- `robot_state` — 1 pub, 1 sub (remote) + test pubs
- `actuator_command` — 1 pub, 1 sub (actuator)
- Total: 6 topics, 10+ subscribers in production, more in tests

**Prevention:**
1. If removing topics, update the capacity constants to match
2. Add a registration-failure counter or assert in debug builds
3. Run all tests after any message_center capacity change

**Detection:** Tasks silently not receiving data, `SubGetMessage` always returning 0, or `PubRegister` returning NULL.

**Which phase should address it:** Message_center narrowing phase.

---

## Minor Pitfalls

Mistakes that cause annoyance but are fixable.

---

### Pitfall 10: Include path breakage from directory restructuring

**What goes wrong:** Moving files between directories breaks relative includes like `../../device/device_layer.h` in `ins_task.c` and `../../../control/task_registry/control_task_registry.h` in `task_registry.c`. The codebase uses relative paths extensively.

**Prevention:** When moving files, grep for all includes that reference the old path. Consider switching to CMake include-directory-based includes (e.g., `#include "device/device_layer.h"`) during the simplification.

**Which phase should address it:** Any phase that moves files.

---

### Pitfall 11: Observation/instrumentation code in chassis_topics.c gets lost

**What goes wrong:** `chassis_topics.c` contains runtime observation logic (`g_actuator_command_observed`, `chassis_runtime_bus_observation_count`, `chassis_runtime_bus_get_first_observation`, `chassis_runtime_bus_get_latest_observation`) that is used by both SITL smoke validation and host tests. This is not obvious from the file name. Simplifying "topics" files risks removing this instrumentation.

**Prevention:** Before modifying `chassis_topics.c`, note that it serves dual duty: pub/sub wiring AND runtime observation. If splitting or removing the pub/sub part, preserve the observation functions in a dedicated file.

**Which phase should address it:** Message_center narrowing phase.

---

### Pitfall 12: Python CLI validate/verify commands break silently

**What goes wrong:** The Python CLI (`platform_cli/main.py`) has `validate` and `verify phase1/2/3` commands that depend on specific build artifacts, test names, and SITL behaviors. Structural changes to the C runtime can cause these commands to pass vacuously (e.g., a test binary that no longer exists is silently skipped).

**Prevention:** After each simplification phase, run the full `validate` command and verify it still exercises the expected number of tests. Check that CTest target names in CMakeLists.txt match what the CLI expects.

**Which phase should address it:** Every phase, as a final verification step.

---

## Phase-Specific Warnings for v2 Simplification

| Phase | Likely Pitfall | Mitigation | Severity |
|-------|---------------|------------|----------|
| Test seam migration | Pitfall 1: Breaking 7 safety tests by changing device_layer hooks | Design replacement seam first, migrate one test, then batch-migrate | CRITICAL |
| Backend selection redesign | Pitfall 2: SITL target stops compiling | Keep "one place decides HW vs SITL" principle, verify both targets build | CRITICAL |
| Message_center narrowing | Pitfall 3: Breaking harness topic dependencies | Map full topic graph, check fan-out before removing any topic | CRITICAL |
| Layer removal | Pitfall 4: Scope creep into contract restructuring | Strict file-boundary scope per phase, no contract header changes | HIGH |
| Layer removal | Pitfall 5: Losing startup readiness gates | Extract gates before removing topic wrappers, keep test_balance_safety_path green | CRITICAL |
| CMake cleanup | Pitfall 6: Linker errors from stale source lists | Update CMakeLists.txt in same commit as file moves | MODERATE |
| Device access simplification | Pitfall 7: Breaking task call sites | Use link-time polymorphism to preserve function signatures | MODERATE |
| Any structural change | Pitfall 8: Harness becomes bottleneck | Update harness in dedicated commit before changing task APIs | MODERATE |
| Message_center config | Pitfall 9: Silent registration failures from reduced limits | Verify topic/subscriber counts, add debug asserts | MODERATE |
| File moves | Pitfall 10: Include path breakage | Grep all relative includes before moving | LOW |
| Topic file changes | Pitfall 11: Losing observation instrumentation | Preserve observation functions in dedicated file | LOW |
| Every phase | Pitfall 12: CLI validate commands pass vacuously | Run full validate after each phase | LOW |

---

## Recommended Simplification Order (Risk-Minimizing)

Based on the pitfall analysis, the safest order is:

1. **Migrate test seams** — Replace device_layer test hooks with a simpler mechanism (link-time stubs or direct struct injection). All 11 tests must pass before proceeding. This is the foundation everything else depends on.

2. **Simplify backend selection** — Replace device_profile vtable with link-time polymorphism (compile different `.c` files per target). Verify HW, SITL, and host_tests all build and pass.

3. **Extract readiness gates** — Move `wait_ready` logic out of topic wrapper files into explicit named functions. Verify `test_balance_safety_path` still passes.

4. **Narrow message_center usage** — Remove topics with fan-out = 1 where direct calls are cleaner (`chassis_observe`, `actuator_command` are candidates). Update harness. Keep fan-out > 1 topics (`ins_data`, `robot_intent`, `device_feedback`, `robot_state`).

5. **Remove device_layer/device_profile files** — Now that tests use the new seam and backend selection is link-time, the old indirection files can be deleted. Task code keeps calling the same function signatures.

6. **CMake cleanup** — Deduplicate test target boilerplate, update source lists to reflect new structure.

This order ensures that at every step, the full test suite remains green and both build targets compile.

---

## Sources

- `robot_platform/runtime/device/device_layer.h` and `device_layer.c` — test hook mechanism, default singleton, profile selection (HIGH, direct codebase analysis)
- `robot_platform/runtime/device/device_profile.h`, `device_profile_sitl.c`, `device_profile_hw.c` — backend binding pattern (HIGH)
- `robot_platform/runtime/tests/host/test_*.c` (all 11 test files) — test hook dependencies (HIGH)
- `robot_platform/runtime/tests/host/test_support/balance_safety_harness.c` — harness task orchestration (HIGH)
- `robot_platform/runtime/tests/host/test_support/device_layer_stubs.c` — link-time stub pattern already in use (HIGH)
- `robot_platform/runtime/control/state/ins_topics.c`, `observe_topics.c`, `actuator_topics.c`, `app_io/chassis_topics.c`, `app_io/remote_topics.c` — full topic graph (HIGH)
- `robot_platform/runtime/module/message_center/message_center.h` — capacity limits and API (HIGH)
- `robot_platform/CMakeLists.txt` — build structure, source lists, target definitions (HIGH)
- `robot_platform/runtime/control/execution/actuator_gateway.c` — device API call sites in production (HIGH)
- `robot_platform/runtime/control/state/ins_task.c`, `app_bringup/remote_task.c` — device API call sites in tasks (HIGH)
