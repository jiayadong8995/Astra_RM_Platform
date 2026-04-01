# Phase 4: authoritative-platform-composition - Research

**Researched:** 2026-04-01
**Domain:** Runtime startup authority, platform composition, and ownership boundaries for `balance_chassis`
**Confidence:** HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
### Startup and Bring-Up Authority
- **D-01:** The hardware host startup path must remain `main.c -> MX_FREERTOS_Init() -> scheduler`.
- **D-02:** `MX_FREERTOS_Init()` host-entry ownership and symbol authority belong to generated `freertos.c`; project code should no longer provide a parallel authoritative implementation of that function.
- **D-03:** Project business startup must be converged behind one stable app startup API that is called from generated startup rather than embedded directly inside generated code.
- **D-04:** SITL may keep a different host entry, but it must reuse the same app startup API as hardware and must not continue to own a parallel business bring-up path through `freertos_app.c`.

### App Startup API Boundary
- **D-05:** The app startup API should own project task registration and project-level runtime wiring.
- **D-06:** The app startup API should not own OS startup, scheduler startup, board initialization, or device-profile-selection policy itself.
- **D-07:** Generated startup should stay thin: initialize RTOS host concerns and then call the app startup API, rather than absorb more project business logic.

### Ownership Boundary Between App and Control
- **D-08:** Phase 4 should enforce a strong ownership split: `app` owns project composition, lifecycle, mode/business assembly, and startup wiring; `control` owns the main runtime chain for state observation, control, and execution output.
- **D-09:** `remote input` plus `intent parsing / mode constraints` remain the most natural application-side responsibilities for the current `balance_chassis` path.
- **D-10:** `state observation -> chassis control -> execution output` should be treated as control-owned runtime responsibilities rather than long-term app-owned task logic.
- **D-11:** `safety protection` and `runtime observability / diagnostics` remain cross-cutting capabilities; planning may place their concrete seams where they best preserve the ownership split above.

### Legacy Path Removal and Composition Cleanup
- **D-12:** Phase 4 should not stop at naming or documentation cleanup; it is allowed to perform real implementation and organization changes needed to eliminate parallel bring-up truth.
- **D-13:** The project should aggressively retire or demote legacy-parallel startup paths when they still present themselves as authoritative paths.
- **D-14:** This phase accepts substantive ownership and implementation migration when needed so that code structure matches the authoritative composition model, not just the docs.
- **D-15:** Cleanup should focus first on bring-up, ownership, and composition seams rather than on broad algorithm rewrites or unrelated platform expansion.

### Reusable Platform Direction
- **D-16:** `balance_chassis` remains the proving path for the platform, but Phase 4 must not collapse the platform into a one-off robot shortcut.
- **D-17:** The right abstraction pressure for this phase is to make the reusable platform direction more explicit and testable through one real composition path, not to over-generalize for hypothetical future robots.
- **D-18:** Project metadata and project configuration should remain the description/selection surface, while runtime/app/control code continues to hold the implementation.

### Claude's Discretion
- The exact app startup API name, file placement, and call shape may be chosen during planning as long as it is clearly the one authoritative project-business bring-up seam shared by hardware and SITL.
- The exact migration sequence from current task/file placement to the stronger `app` versus `control` ownership model may be chosen during planning as long as the end state leaves one blessed bring-up path and removes parallel authority.
- Planner/research may choose which existing files become compatibility shells versus which are removed outright, provided the final codebase no longer leaves developers guessing which startup path is authoritative.

### Deferred Ideas (OUT OF SCOPE)
- Broad new robot capabilities or a richer application-responsibility expansion remain out of scope for this phase.
- High-fidelity simulation, replay, or broader verification-surface growth remain outside Phase 4 unless they are directly needed to support the authoritative bring-up path.
- Deep algorithm rewrites or generalized multi-robot abstraction work beyond what `balance_chassis` needs as the proving path should be deferred to later phases.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| ARCH-01 | The platform defines one authoritative ownership boundary between orchestration, device adapters, control logic, and robot-project composition | Ownership split below makes `app` the composition seam, `device` the semantic adapter seam, and `control` the runtime chain owner. |
| ARCH-03 | The current platform architecture is reviewed for unnecessary coupling and overdesign, with v1 changes focused on making the existing reusable direction testable rather than broader | Research identifies specific coupling to remove: duplicate startup authority, app-owned controller task placement, control dependence on app config, and stale validation assumptions. |
| ARCH-04 | `balance_chassis` remains the proving path for the reusable platform, without collapsing the platform into one-off robot-specific shortcuts | Recommended migration keeps project metadata in `projects/`, shared runtime layers in `runtime/`, and only project composition in `app/balance_chassis`. |
| OBS-03 | The authoritative `balance_chassis` bring-up path is documented well enough that developers know which runtime path is blessed and which legacy paths are not | Research recommends one named blessed path plus explicit demotion/removal of `freertos_app.c` and other legacy-parallel surfaces in docs and compatibility shells. |
</phase_requirements>

## Project Constraints (from CLAUDE.md)

- Preserve the reusable robot-platform direction; do not solve Phase 4 by collapsing into a one-off robot app.
- Keep pre-hardware verification as the gate for runtime changes; fake-link and host tests are evidence, not physical proof.
- Preserve existing runtime layering where it provides leverage, but actively remove implementation-level coupling and overdesign.
- Work within the current toolchain reality: generated STM32Cube assets, CMake, host tests, SITL, and local environment setup are all part of the implementation surface.
- Use `balance_chassis` as the proving path for platform decisions without turning it into a bypass around the platform.

## Summary

Phase 4 should be planned as an architecture-convergence phase, not a naming cleanup. The repo currently has split startup authority: hardware builds compile generated `freertos.c`, SITL compiles `app_bringup/freertos_app.c`, and both files define `MX_FREERTOS_Init()`. At the same time, the checked-in generated `main.c` does not currently call `MX_FREERTOS_Init()` or start the scheduler, so the desired hardware truth (`main.c -> MX_FREERTOS_Init() -> scheduler`) is a target state that must be made real in generated/user-code seams, not merely documented.

The clean planning direction is: generated startup owns host/RTOS entry, one project app-startup API owns task registration and project-level composition, and the main runtime chain migrates toward `control` ownership. `remote_task` and `remote_intent` remain application responsibilities; observation, controller, and execution logic should stop presenting as app-owned logic over time.

The planner should treat build wiring, ownership migration, validation, and documentation as one connected workstream. A doc-only answer will fail because the current source tree and CMake target composition still encode parallel truth.

**Primary recommendation:** Plan Phase 4 around a thin generated/SITL startup shim that both call one new app-startup API, while migrating `observe/chassis/motor` runtime ownership out of app-facing bring-up surfaces and preserving existing host/SITL verification entrypoints.

## Standard Stack

### Core
| Library / Surface | Version | Purpose | Why Standard Here |
|---------|---------|---------|--------------|
| Vendored FreeRTOS + CMSIS-RTOS | vendored in `third_party/stm32_cube` | RTOS task creation and scheduler startup | Existing authoritative runtime substrate for both hardware and SITL. |
| STM32 generated startup (`main.c`, `freertos.c`) | checked-in generated sources | Hardware host entry and RTOS host init | Locked decision says generated startup must own `MX_FREERTOS_Init()`. |
| `runtime/app/balance_chassis` composition layer | repo-local | Project composition, lifecycle, remote intent, startup seam | Already holds task registration and intent logic; best seed for one app API. |
| `runtime/control` | repo-local | State observation, control solve, execution output | Already contains most of the true runtime chain implementation. |
| `robot_platform/CMakeLists.txt` | CMake 3.22+ in repo; local `cmake 3.28.3` | Authoritative build composition for hw, SITL, and host tests | Phase 4 changes must be reflected in compiled target truth, not docs alone. |

### Supporting
| Surface | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| Host C tests under `runtime/tests/host` | repo-local | Fast regression on composition/runtime seams | Use for startup API and ownership migration without requiring SITL. |
| Python `unittest` suites | stdlib; local `Python 3.12.3` | CLI/report regression coverage | Use when docs/verification surfaces or CLI routing change. |
| Existing `verify phase3` artifact flow | repo-local | Machine-readable blessed-path proof for SITL path | Reuse for OBS-03 and migration safety rather than creating a new report path. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Thin generated startup calling one app API | Keep `freertos_app.c` as the business startup owner | Reject. Leaves parallel startup authority and violates D-02 through D-04. |
| Moving runtime chain toward `control` ownership | Keep `chassis_task.c` as the long-term app-owned controller task | Reject. Preserves the exact ownership ambiguity ARCH-01 is trying to remove. |
| Reusing CTest/unittest/verify surfaces | Invent a new Phase 4-only validation harness | Reject for v1. Adds more orchestration truth instead of simplifying it. |

## Architecture Patterns

### Recommended Project Structure

```text
runtime/
├── generated/                      # Host/RTOS-generated startup authority
├── bsp/                            # Board and SITL host entry surfaces
├── device/                         # Semantic device adapters and backend binding
├── control/                        # Runtime chain: observe -> control -> execution
├── app/
│   └── balance_chassis/
│       ├── app_startup/            # Single project startup seam
│       ├── app_intent/             # Remote input + mode/intent shaping
│       └── app_config/             # Project task periods/priorities/config
└── module/                         # Reusable primitives
```

### Pattern 1: Thin Startup Shim
**What:** Generated hardware startup and SITL host entry both call one project startup API. The generated layer owns host concerns; app startup owns task registration and project wiring.
**When to use:** Immediately. This is the central convergence seam for D-01 through D-07.
**Example:**
```c
// Source: robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/freertos.c
void MX_FREERTOS_Init(void) {
    /* generated RTOS host init */
    balance_chassis_app_startup();
}
```

### Pattern 2: App Composes, Control Executes
**What:** `app` owns remote ingress, intent/mode shaping, lifecycle, and project composition. `control` owns observation, control solve, and execution output.
**When to use:** During file/task migration. Do not move remote intent out of app; do move long-term runtime-chain ownership toward `control`.
**Example:**
```c
// Source: current split across remote_task.c, observe_task.c, chassis_task.c, motor_control_task.c
remote_task_step(&remote);
observe_task_step(&observe);
chassis_task_step(&chassis);
motor_control_task_step(&motor);
```

### Pattern 3: Compatibility Shell Migration
**What:** Keep temporary wrappers for renamed/moved files only when they reduce migration risk, but make them visibly non-authoritative.
**When to use:** If includes, tests, or CMake targets need a short transition period.
**Example:**
```c
// Temporary compatibility shell
void balance_chassis_start_tasks(void) {
    balance_chassis_app_startup_register_tasks();
}
```

### Anti-Patterns to Avoid
- **Parallel startup symbols:** Do not leave both generated code and project code defining `MX_FREERTOS_Init()`.
- **Generated-code business ownership:** Do not push task registration or project logic deeper into CubeMX-generated files.
- **App-owned control forever:** Do not keep `observe/chassis/motor` as app-facing ownership merely because current file paths say so.
- **Robot-specific bypasses:** Do not solve Phase 4 by bypassing `device` or `control` with `balance_chassis`-only shortcuts.
- **Doc-only blessing:** Do not publish a blessed path before CMake, source ownership, and test surfaces agree on it.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Startup convergence | A second custom host bootstrap layer | Generated `freertos.c` plus one app-startup API | Prevents duplicate authority and preserves CubeMX ownership. |
| Runtime-chain migration | A new simulator-only controller path | Existing task init/prepare/step helpers and host harness | Current helpers already support composition testing without shadow logic. |
| Verification of blessed path | A Phase 4-only report stack | Existing CTest, Python unittest, and `verify phase3` reporting surfaces | Reuses current proof surfaces and keeps evidence comparable across phases. |
| Project identity | Robot-specific implementation in `projects/` | Keep `projects/` as metadata/config only | Preserves reusable platform direction and ARCH-04 intent. |

**Key insight:** The repo already has the primitives needed for Phase 4. The work is to converge authority and remove ambiguity, not to invent a new framework.

## Common Pitfalls

### Pitfall 1: Treating `freertos_app.c` as a harmless legacy shim
**What goes wrong:** It remains the de facto business startup owner for SITL while generated `freertos.c` remains the hardware owner.
**Why it happens:** Both paths compile different startup files today, and both expose `MX_FREERTOS_Init()`.
**How to avoid:** Introduce one app-startup API and demote/remove `freertos_app.c` as an authoritative entry surface.
**Warning signs:** Developers must ask whether hardware and SITL use the same bring-up path.

### Pitfall 2: Blessing a hardware path that is not expressed in code
**What goes wrong:** Docs say `main.c -> MX_FREERTOS_Init() -> scheduler`, but checked-in generated `main.c` does not call that flow.
**Why it happens:** The intended path exists conceptually, but generated/user-code seams have not been reconciled.
**How to avoid:** Make generated startup truth explicit in source and document exactly which generated user blocks carry the call.
**Warning signs:** `main.c` contains no `MX_FREERTOS_Init()` or scheduler call.

### Pitfall 3: Leaving app config coupled into control forever
**What goes wrong:** `control/state` and `control/execution` continue including `app/balance_chassis/app_config/app_params.h`.
**Why it happens:** Current task periods and startup delays are project-scoped, but control files consume them directly.
**How to avoid:** Decide whether the config remains project-owned but injected through composition, or whether shared control timing constants move to a neutral seam.
**Warning signs:** `runtime/control/*` still includes project app headers after ownership cleanup.

### Pitfall 4: Assuming the existing build tree is a stable validation baseline
**What goes wrong:** Plans rely on host tests that are registered in CTest but not currently built in `build/robot_platform_host_tests`.
**Why it happens:** The build directory is partially materialized; some tests exist, some executables do not.
**How to avoid:** Include a Wave 0 configure/build refresh before using host tests as phase gates.
**Warning signs:** `ctest` lists tests, but selected executables are missing at runtime.

### Pitfall 5: Moving everything out of `app` in one shot
**What goes wrong:** Phase 4 turns into a broad rewrite and destabilizes validated Phase 2/3 behavior.
**Why it happens:** Ownership cleanup is conflated with algorithm refactoring.
**How to avoid:** Migrate only the seams needed to make authority clear: startup API, task ownership, config dependencies, and docs.
**Warning signs:** Planned tasks start touching controller algorithms or broad multi-robot abstractions.

## Code Examples

Verified patterns from repository sources:

### Single Task Registration Seed
```c
// Source: robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c
void balance_chassis_start_tasks(void)
{
    osThreadDef(INS_TASK, INS_Task, APP_INS_TASK_PRIORITY, 0, APP_INS_TASK_STACK_BYTES);
    INS_TASKHandle = osThreadCreate(osThread(INS_TASK), NULL);
    /* ... other task registrations ... */
}
```

### Stepwise Host Harness for Runtime Composition
```c
// Source: robot_platform/runtime/tests/host/test_support/balance_safety_harness.c
remote_task_step(&harness->remote);
observe_task_step(&harness->observe);
chassis_task_step(&harness->chassis);
motor_control_task_step(&harness->motor);
```

### Current SITL Host Entry Shape
```c
// Source: robot_platform/runtime/bsp/sitl/main_sitl.c
MX_FREERTOS_Init();
vTaskStartScheduler();
```

## State of the Art

| Old Approach | Current Recommended Approach | When Changed | Impact |
|--------------|------------------------------|--------------|--------|
| Parallel startup ownership across generated and app code | Generated host startup + one app-startup API | Phase 4 | Removes ambiguity and makes docs/test plans enforceable. |
| App layer owns mixed business and control runtime tasks | App owns composition/intent; control owns observe/control/execution chain | Phase 4 | Sharpens ARCH-01 boundary and reduces coupling. |
| Build/docs infer the blessed path indirectly | Explicit blessed `balance_chassis` path documented and reflected in CMake/source ownership | Phase 4 | Satisfies OBS-03 and lowers onboarding/debug cost. |

**Deprecated/outdated:**
- `runtime/app/balance_chassis/app_bringup/freertos_app.c` as an authoritative startup owner: should be removed or converted to a clearly non-authoritative compatibility shell.
- `task/file location == ownership`: current file placement is historical and should not be treated as the target architecture.

## Open Questions

1. **Where should the new app-startup API live?**
   - What we know: it must be shared by hardware and SITL and own task registration/project wiring only.
   - What's unclear: whether to keep it under `app_bringup/`, create `app_startup/`, or place it elsewhere under `app/balance_chassis/`.
   - Recommendation: choose a new explicit `app_startup/` seam if planner wants clarity; otherwise keep `app_bringup/` only if legacy naming would slow delivery.

2. **How far should `chassis_task.c` migrate in Phase 4?**
   - What we know: long-term ownership belongs with `control`, not `app`.
   - What's unclear: whether Phase 4 should move only task wrappers and buses, or also relocate the file and runtime structs.
   - Recommendation: migrate ownership enough that the blessed path is unambiguous; use compatibility wrappers if full relocation would destabilize Phase 2/3 test seams.

3. **How should project-owned timing/config reach control-owned tasks?**
   - What we know: `observe_task.c`, `ins_task.c`, and `motor_control_task.c` include `app_params.h` directly today.
   - What's unclear: whether these constants stay project-owned and get injected, or whether shared timing moves to a more neutral contract/config seam.
   - Recommendation: plan this explicitly. It is the main remaining concrete coupling between app and control code.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| `python3` | CLI verification and Python unit tests | ✓ | 3.12.3 | — |
| `cmake` | Configure hw/SITL/host-test targets | ✓ | 3.28.3 | — |
| `ctest` | Host C regression execution | ✓ | 3.28.3 | — |
| `gcc` | Host-side C test builds | ✓ | 13.3.0 | — |
| `arm-none-eabi-gcc` | Hardware firmware builds | ✓ | 13.2.1 | — |
| `ninja` | Hardware build generator | ✓ | 1.11.1 | — |
| `java` | STM32CubeMX backend runtime | ✓ | 21.0.10 | — |
| `STM32CubeMX` CLI on PATH or repo-local install | Regenerating startup artifacts | ✗ | — | Use checked-in generated sources unless the phase must regenerate; otherwise set `STM32CUBEMX_BIN` or install supported CubeMX 6.17.0. |

**Missing dependencies with no fallback:**
- None for planning, docs, CMake composition changes, or host/Python validation.

**Missing dependencies with fallback:**
- `STM32CubeMX` CLI: fallback is to avoid regeneration during the phase unless generated user-block edits or IOC changes require it.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | CTest-backed host C tests + Python `unittest` |
| Config file | none — test registration lives in `robot_platform/CMakeLists.txt` |
| Quick run command | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_device_profile_sitl_runtime_bindings` |
| Full suite command | `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v` plus `ctest --test-dir build/robot_platform_host_tests --output-on-failure` after refreshing the host-test build directory |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| ARCH-01 | One authoritative ownership boundary is reflected in compiled startup/composition surfaces | host integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_balance_safety_path` after build refresh | ✅ but current executable missing in build tree |
| ARCH-03 | Coupling reduction preserves existing runtime behavior while simplifying composition | host integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R "test_balance_safety_path|test_device_profile_sitl_runtime_bindings"` | ✅ |
| ARCH-04 | `balance_chassis` remains the proving path through platform layers | smoke + report | `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case runtime_binding` | ✅ |
| OBS-03 | Blessed bring-up path is explicit in docs and verification entrypoints | Python/unit + manual doc review | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` plus manual review of runtime README/project docs | partial |

### Sampling Rate
- **Per task commit:** `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v`
- **Per wave merge:** `ctest --test-dir build/robot_platform_host_tests --output-on-failure`
- **Phase gate:** Python unittest suite, refreshed host CTest suite, and `verify phase3 --case runtime_binding` should all pass before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] Refresh or rebuild `build/robot_platform_host_tests` so all registered CTest executables actually exist; current tree is partial.
- [ ] Add a dedicated host test for the new authoritative app-startup API so startup convergence is validated without requiring SITL.
- [ ] Add regression coverage for whichever compatibility shell or relocation strategy is chosen for `chassis_task`/task registration.
- [ ] Add a documentation assertion surface if Phase 4 introduces a new blessed-path string in CLI or README output.

## Sources

### Primary (HIGH confidence)
- `.planning/phases/04-authoritative-platform-composition/04-CONTEXT.md` - locked Phase 4 decisions, canonical references, and scope
- `.planning/REQUIREMENTS.md` - requirement definitions for `ARCH-01`, `ARCH-03`, `ARCH-04`, `OBS-03`
- `.planning/STATE.md` - project history and carry-forward decisions from Phases 1-3
- `CLAUDE.md` - project constraints and current stack/toolchain summary
- `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/main.c` - current hardware entry surface; no visible `MX_FREERTOS_Init()` call
- `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/freertos.c` - generated owner of `MX_FREERTOS_Init()` in hardware build
- `robot_platform/runtime/bsp/sitl/main_sitl.c` - SITL host entry calling `MX_FREERTOS_Init()` and scheduler
- `robot_platform/runtime/app/balance_chassis/app_bringup/freertos_app.c` - parallel project-owned `MX_FREERTOS_Init()` implementation
- `robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c` - current task-registration seed
- `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c` - app-side remote ingress
- `robot_platform/runtime/control/state/observe_task.c` - current observation task implementation
- `robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.c` - current mixed ownership control seam
- `robot_platform/runtime/control/execution/motor_control_task.c` - control-owned execution task
- `robot_platform/runtime/app/balance_chassis/app_intent/remote_intent.c` - app-owned mode/intent shaping
- `robot_platform/runtime/README.md`, `robot_platform/runtime/device/README.md`, `robot_platform/runtime/control/README.md` - intended layer contracts
- `robot_platform/CMakeLists.txt` - actual target composition for hw, SITL, and host tests
- `.planning/codebase/ARCHITECTURE.md`, `.planning/codebase/STRUCTURE.md`, `.planning/codebase/CONCERNS.md` - current architecture map and known coupling concerns
- `.planning/phases/03-fake-link-runtime-proof/03-CONTEXT.md` - application responsibility framing carried into Phase 4
- `.planning/phases/03-fake-link-runtime-proof/03-VERIFICATION.md` - latest verified SITL/runtime-path truth entering Phase 4

### Secondary (MEDIUM confidence)
- Local environment probe on 2026-04-01 for `python3`, `cmake`, `ctest`, `gcc`, `arm-none-eabi-gcc`, `ninja`, `java`, and `STM32CubeMX`
- `ctest --test-dir build/robot_platform_host_tests --show-only=json-v1` - current registered host test inventory
- `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v` - passing Python regression surface
- `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_device_profile_sitl_runtime_bindings` - verified runtime-binding host regression

### Tertiary (LOW confidence)
- None

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - driven by current repo composition and local environment probes
- Architecture: HIGH - authoritative seams are directly visible in source, CMake, and prior phase verification
- Pitfalls: MEDIUM-HIGH - source evidence is strong, but the exact best migration sequence is still planner discretion

**Research date:** 2026-04-01
**Valid until:** 2026-04-15
