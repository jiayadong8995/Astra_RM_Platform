# Phase 02: Host Safety Control Verification - Research

**Researched:** 2026-03-31
**Domain:** Host-side verification for the current `balance_chassis` safety-critical control path
**Confidence:** HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **D-01:** Phase 2 should be understood as "turn the existing main control path into an injectible, observable, fail-fast safety verification chain."
- **D-02:** Work that does not directly improve `inject -> observe -> judge` behavior on the current `balance_chassis` runtime path should be treated as secondary within this phase.
- **D-03:** Phase 2 should verify the current real runtime path first, not an idealized future architecture path.

### Authoritative Runtime Path
- **D-04:** The authoritative path for Phase 2 is the currently running task/topic control chain, not a direct-call-only shadow harness.
- **D-05:** The Phase 2 proof target is the live path composed around `remote_task`, `Observe_task`, `Chassis_task`, and `motor_control_task`, including the current `message_center` transport seams between them.
- **D-06:** Passing Phase 2 must be interpreted as "the current implementation path is safety-checkable," not "the target architecture vision is already achieved."

### Input Injection Strategy
- **D-07:** Fake sensor and fake remote inputs should enter primarily through the real `device/profile` seams so the main runtime path consumes them the same way it does today.
- **D-08:** Data-link loss, stale-command, and topic-transport fault scenarios may use `message/topic` or equivalent runtime-ingress seams when that is the most direct way to exercise the current path.
- **D-09:** Phase 2 should avoid building a broad new fake framework; it should introduce only the deterministic injection seams needed to exercise the current path.

### Observation Strategy
- **D-10:** The first authoritative Phase 2 observation target should remain `actuator_command`, because it is the clearest proof that the control path produced a runtime decision.
- **D-11:** Phase 2 should also observe the key safety state carried with that output: at minimum `start`, `control_enable`, and `actuator_enable`.
- **D-12:** Additional intermediate observations should be introduced only when they are necessary to make a safety verdict unambiguous; Phase 2 should not begin by instrumenting every internal state.

### Safety Verdict Style
- **D-13:** Phase 2 verdicts should use hard fail-or-pass oracles, not soft "looks reasonable" review.
- **D-14:** Invalid direction or command mapping for the active profile should fail verification explicitly.
- **D-15:** Output saturation violations should fail verification explicitly.
- **D-16:** Stale, invalid, or unavailable sensor inputs that still lead to enabled actuator output should fail verification explicitly.
- **D-17:** Lost or stale command input that does not transition to a defined safe behavior should fail verification explicitly.
- **D-18:** Invalid arming or state-machine transitions that still allow closed-loop control to engage should fail verification explicitly.

### Wheel-Leg Coupling Scope
- **D-19:** Phase 2 should not attempt a broad stability proof for wheel-leg coupling.
- **D-20:** The first regression coverage should encode one or two explicit wheel-leg danger signatures on the current `balance_chassis` path.
- **D-21:** The initial coupling regression should focus on cases where attitude or support-state inputs can drive combined leg and wheel outputs into an unsafe enabled pattern without being blocked or limited.

### Current Reality Constraints
- **D-22:** Phase 2 planning must treat the current SITL/verify situation honestly: `verify phase1` currently proves a minimum live path, but standalone `sim` is not yet stable enough to be treated as the authoritative Phase 2 safety entrypoint.
- **D-23:** The current main-chain runtime still relies on task/topic transport for `robot_intent`, `device_feedback`, and `actuator_command`; Phase 2 must validate safety on that reality instead of assuming the architecture docs are already true in implementation.

### Architecture Pressure and Refactor Permission
- **D-24:** Phase 2 should not treat the current `device_layer` shape as a protected architectural boundary if it blocks clear injection seams or makes device behavior too hard to reason about.
- **D-25:** The current `device_layer` is considered an over-wrapped default-container abstraction unless planning can justify its continued existence with concrete verification value.
- **D-26:** Phase 2 planning may replace or simplify `device_layer` in favor of clearer device-ops ownership and a new device contract shape if that makes the safety-verification path easier to inject, observe, and judge.
- **D-27:** The current `robot_def.h -> control/internal/balance_params.h` relationship should be treated as a coupling smell, not a boundary worth preserving.
- **D-28:** `chassis_observer` and `ins_state_estimator` should be treated as reviewable implementation seams: they may keep their responsibilities, but they should not be preserved merely because they already exist as named layers.

### Claude's Discretion
- The exact host-side safety harness may combine host C tests and structured Python orchestration as long as the authoritative path still runs through the current `balance_chassis` runtime behavior.
- The exact machine-readable schema for Phase 2 verdict artifacts may be selected by planning/research as long as it can distinguish injection case, observed outputs, and safety failure reason.
- The precise first-wave danger signatures for wheel-leg coupling may be selected by the agent if they stay narrow, explicit, and tied to the current code path.

### Deferred Ideas (OUT OF SCOPE)
- Broad fake-link observability and communication/control fault separation belong mainly to Phase 3.
- Platform ownership simplification and direct-interface architectural cleanup belong mainly to Phase 4.
- Richer stability proof or replay-based fault libraries belong to later v1/v2 work once the first safety-verdict chain exists.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| HOST-02 | Host-side tests cover message transport, control-path logic, device/profile binding seams, and actuator command mapping on the current `balance_chassis` path | Extend existing CTest coverage from `message_center` and `actuator_gateway` into task/path seams plus profile-bound fake devices. |
| HOST-03 | Host-side verification can inject fake sensor, remote, and data-link inputs to exercise runtime behavior deterministically | Use `device_profile_sitl` bindings for IMU/remote injection and topic ingress for link-loss or stale-command faults. |
| SAFE-01 | Runtime blocks actuator output when control direction or command mapping is invalid for the active robot profile | Add explicit host assertions around `actuator_gateway` mapping and profile-valid command semantics. |
| SAFE-02 | Runtime blocks or degrades actuator output when sensor data is stale, invalid, or unavailable | Add health/freshness-aware safety oracles around INS readiness and device feedback validity before actuator enable. |
| SAFE-03 | Runtime blocks invalid enable/state-machine transitions that could arm control in an unsafe state | Assert `remote_intent -> controller -> actuator_command` never arms through invalid transitions. |
| SAFE-04 | Runtime enforces configured output saturation and fails verification when limits are violated | Convert current silent clamps into observable verdict checks tied to configured limits. |
| SAFE-05 | Runtime detects data-link loss or stale command input and transitions to a defined safe behavior | Use message/topic injection to create stale or missing command cases and require a deterministic disabled/degraded result. |
| SAFE-06 | Verification includes a regression path for wheel-leg coupling instability risks identified for `balance_chassis` | Encode one or two narrow danger signatures using current feedback/state inputs and actuator output observations. |
</phase_requirements>

## Summary

Phase 2 should be planned as an extension of the code and verification stack that already exists after Phase 1, not as a fresh harness. The current authoritative path is real and runnable: `remote_task` reads the default remote device, publishes `robot_intent`, `Observe_task` consumes `robot_intent` plus `device_feedback`, `Chassis_task` produces `actuator_command`, and `motor_control_task` dispatches that command through `actuator_gateway` and the default device profile. The current verification stack already has three pieces worth preserving: CTest-based host C tests, Python `unittest` report regression tests, and JSON-oriented CLI orchestration.

The main planning gap is not tooling. It is safety semantics. The code already exposes enable bits and already clamps some outputs, but most of the required Phase 2 verdicts are still implicit. `actuator_gateway` only gates dispatch on `control_enable && actuator_enable`; `balance_controller` mirrors `start_enabled` into output enables; `actuator_constraints` silently saturates values; and the SITL IMU/remote adapters always return valid canned inputs. That means the phase must add explicit injectible fault cases and explicit machine-checkable failure reasons, not just more wrappers around the existing runtime.

The fastest credible implementation shape is: keep the current task/topic path authoritative, add deterministic fake-device and topic-ingress seams for specific fault cases, observe `actuator_command` plus enable bits as the primary verdict surface, and introduce a small number of explicit safety oracles for stale sensor input, stale command input, invalid arming, command mapping validity, saturation, and one or two wheel-leg danger signatures. If `device_layer` makes this harder than necessary, simplify it in service of clearer verification seams instead of preserving it.

**Primary recommendation:** Plan Phase 2 around a small host-side safety matrix driven through the real runtime path, with new explicit verdict artifacts and only the minimum seam refactors needed to make injection and safety judgment deterministic.

## Project Constraints (from CLAUDE.md)

- Build a reusable Robotmaster platform, not a one-off `balance_chassis` app.
- On-robot testing must stay gated behind pre-hardware verification.
- Host tests and fake-link evidence are trust-building stages, not physical proof.
- Preserve runtime layering only where it provides concrete leverage; review and reduce coupling when it obstructs verification.
- Work within the existing toolchain reality: STM32CubeMX, cross-compilers, local host setup, CMake, and Python orchestration.
- Keep `balance_chassis` as the proving path without collapsing the reusable platform into robot-specific shortcuts.
- Follow the repository’s GSD workflow rather than ad hoc direct edits.

## Project Skill Notes

- No project-local skill directories were found under `.claude/skills/` or `.agents/skills/`.
- Research therefore relies on repo code, planning artifacts, and official tool documentation rather than additional project-specific skill rules.

## Standard Stack

### Core
| Library / Tool | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| CMake | 3.23.0 available, repo minimum 3.22 | Configure host tests, SITL, and firmware targets | Already owns all build graph and test target registration. |
| CTest | 3.23.0 | Run host-native C regression targets | Already wired in `robot_platform/CMakeLists.txt` and fast enough for per-commit use. |
| GCC | 13.4.0 on host | Compile host C tests with sanitizers | Matches existing Linux toolchain and sanitizer path. |
| Python | 3.11.9 | CLI orchestration and report regression tests | Already owns `verify`, `sim`, and JSON report logic. |

### Supporting
| Library / Tool | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `unittest` | Python stdlib | CLI and runner regression tests | Use for report schema and orchestration behavior. |
| AddressSanitizer + UndefinedBehaviorSanitizer | GCC-supported flags | Catch memory/UB regressions in host C targets | Keep enabled for all new host C targets. |
| `message_center` | repo-local | Transport the authoritative current task/topic path | Use for current-path verification, not as a stub target. |
| Existing SITL project profile and runner | repo-local | JSON-first runtime observation and smoke reporting | Reuse for Phase 2 verdict artifacts rather than inventing a second report stack. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Current task/topic path | Direct-call-only harness | Faster unit setup, but violates Phase 2 proof target and can hide transport defects. |
| Existing device/profile seams | Large custom fake framework | More flexibility, but contradicts narrow-scope seam guidance and adds maintenance burden. |
| CTest + `unittest` | New third-party test framework | Little benefit for this phase; existing stack is already present and passing. |

**Build prerequisites already present:** `python3`, `cmake`, `ctest`, `gcc`, `clang`, `ninja`, `arm-none-eabi-gcc`, and `java` are available in this environment. `STM32CubeMX` is not on `PATH`, but Phase 2 host safety work does not require it as the critical path.

## Architecture Patterns

### Recommended Project Structure
```text
robot_platform/
├── runtime/tests/host/              # New host C safety-path and seam tests
├── runtime/tests/host/test_support/ # Deterministic fake device/topic helpers
├── tools/platform_cli/              # New `verify phase2` orchestration and report schema
├── tools/platform_cli/tests/        # CLI/report regression coverage
└── sim/tests/                       # Report parsing and validation-target regression coverage
```

### Pattern 1: Verify The Authoritative Runtime Path
**What:** Keep `remote_task -> Observe_task -> Chassis_task -> motor_control_task` as the proof target.
**When to use:** For any Phase 2 requirement that claims the current runtime is safe-checkable.
**Why:** The current code path already transports `robot_intent`, `device_feedback`, and `actuator_command` through real topics, so bypassing it would produce a weaker proof than the phase requires.
**Example:**
```c
// Source: robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c
while (1)
{
    (void)platform_device_read_default_remote(&rc_input);
    platform_remote_intent_bus_pull_inputs(&intent_bus, &robot_state);
    platform_remote_intent_state_apply_inputs(&intent_state, &rc_input, &robot_state);
    intent = platform_remote_intent_build(&intent_state);
    platform_remote_intent_bus_publish(&intent_bus, &intent);
}
```

### Pattern 2: Inject Through Existing Device/Profile Or Topic Seams
**What:** Use fake IMU and remote behavior in the default profile for sensor/operator cases, and message/topic ingress for stale-command or link-fault cases.
**When to use:** When a fault naturally belongs at a device boundary or transport boundary.
**Why:** The repo already binds SITL devices through `device_profile_sitl`, and the context explicitly allows transport-level fault injection where that is the cleanest path.

### Pattern 3: Judge Safety From Observable Output Plus Minimal State
**What:** Use `actuator_command`, `start`, `control_enable`, and `actuator_enable` as the first verdict surface; add intermediate evidence only when required to disambiguate a failure reason.
**When to use:** For every Phase 2 verdict artifact.
**Why:** `chassis_topics.c` already emits observable runtime output and Phase 1 made it the authoritative output proof surface.
**Example:**
```c
// Source: robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c
printf("[RuntimeOutput] topic=actuator_command start=%u control_enable=%u actuator_enable=%u\n",
       actuator_command->start ? 1U : 0U,
       actuator_command->control_enable ? 1U : 0U,
       actuator_command->actuator_enable ? 1U : 0U);
```

### Pattern 4: Turn Silent Safety Behavior Into Explicit Oracles
**What:** Where the code currently clamps or disables implicitly, tests must assert that the behavior occurred and report why.
**When to use:** Saturation, invalid arming, stale sensor, stale command, and mapping faults.
**Why:** Silent success is not enough for Phase 2; the phase requires fail/pass verdicts with failure reasons.
**Example:**
```c
// Source: robot_platform/runtime/control/constraints/actuator_constraints.c
mySaturate(&chassis->wheel_motor[i].torque_set, -WHEEL_TORQUE_MAX, WHEEL_TORQUE_MAX);
platform_int16_clamp(&chassis->wheel_motor[i].give_current, -8000, 8000);
```

### Anti-Patterns to Avoid
- **Shadow-path verification:** Do not satisfy Phase 2 with direct controller calls that skip tasks, topics, or profile binding.
- **Fake-framework sprawl:** Do not introduce a broad generic simulation layer just to produce a handful of deterministic faults.
- **Log-only verdicts:** Do not rely on human review of stdout; use machine-readable case records.
- **Implicit safety claims:** Do not treat clamping or `valid=false` as proof unless the tests assert that behavior against a requirement-specific oracle.
- **Preserve-by-default wrapping:** Do not keep `device_layer` or mixed parameter headers unchanged if they directly block deterministic injection or judgment.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| End-to-end host verification | A new standalone harness runner | Extend `platform_cli.main` with `verify phase2` | Existing CLI already emits JSON stage reports and has tests. |
| Host C test execution | Custom shell scripts for every target | CMake + CTest targets | Existing build graph, sanitizer flags, and test registration are already working. |
| Fault injection | A generic fake-device framework | Narrow fake behaviors in SITL device/profile seams and topic ingress helpers | Scope stays aligned with current path and phase constraints. |
| Runtime verdicts | Manual log inspection | Structured JSON case artifacts | Planner and later verification need deterministic machine-readable evidence. |
| Safety transport | Alternate pub/sub layer | Existing `message_center` topics | Phase 2 must validate current reality, not a future transport story. |

**Key insight:** The repo already has enough infrastructure. The missing work is precise safety semantics, not a new platform for running tests.

## Common Pitfalls

### Pitfall 1: Verifying A Shadow Harness
**What goes wrong:** Tests pass on direct controller calls but fail to cover the real task/topic path.
**Why it happens:** Direct-call tests are cheaper to write than task/path verification.
**How to avoid:** Make at least one authoritative test path run through current task/topic contracts and treat direct-call tests only as subordinate seam coverage.
**Warning signs:** Tests never touch `message_center`, `device_profile_sitl`, or task bus helpers.

### Pitfall 2: Treating Enable Bits As Full Safety Proof
**What goes wrong:** `control_enable` and `actuator_enable` are asserted, but unsafe mapped commands or saturated values still pass unnoticed.
**Why it happens:** The current code already has a minimal dispatch gate, so it is easy to over-credit it.
**How to avoid:** Pair enable-state assertions with command-payload assertions and explicit limit checks.
**Warning signs:** Tests only check `valid` or enable flags and never inspect motor fields.

### Pitfall 3: Missing Startup And Warmup Semantics
**What goes wrong:** Sensor-invalid cases become flaky because INS readiness has built-in warmup ticks and task readiness loops.
**Why it happens:** `ins_state_estimator` and bus wait-ready paths gate early runtime behavior before the controller meaningfully runs.
**How to avoid:** Model warmup deliberately in tests and define whether a case targets pre-ready blocking or post-ready stale degradation.
**Warning signs:** Cases fail intermittently or only at startup.

### Pitfall 4: Silent Saturation Looks Like Success
**What goes wrong:** Output clamping occurs, but verification cannot tell whether the runtime protected itself or generated an over-limit command first.
**Why it happens:** Current constraints functions clamp in-place without publishing a safety reason.
**How to avoid:** Add explicit assertions or artifact fields that record limit application and whether any unclamped unsafe state was observed.
**Warning signs:** A test only checks final values are in range.

### Pitfall 5: Conflating Sensor Faults With Link Faults
**What goes wrong:** One generic “invalid input” case tries to prove both sensor safety and command-link safety.
**Why it happens:** Both appear as missing or stale data at the controller boundary.
**How to avoid:** Keep device-side fault cases and topic/command-side fault cases separate in the verdict schema.
**Warning signs:** Artifact reasons are vague, like `input_invalid`, with no source classification.

## Code Examples

Verified patterns from local code and official tool docs:

### Existing Output Gate In The Execution Layer
```c
// Source: robot_platform/runtime/control/execution/actuator_gateway.c
static bool platform_command_dispatch_enabled(const platform_actuator_command_t *actuator_msg)
{
    return actuator_msg->control_enable && actuator_msg->actuator_enable;
}
```

### Existing Silent Saturation That Needs An Explicit Oracle
```c
// Source: robot_platform/runtime/control/constraints/actuator_constraints.c
for (int i = 0; i < 2; i++)
{
    mySaturate(&chassis->wheel_motor[i].torque_set, -WHEEL_TORQUE_MAX, WHEEL_TORQUE_MAX);
    platform_int16_clamp(&chassis->wheel_motor[i].give_current, -8000, 8000);
}
```

### Existing CTest Host Registration Pattern
```cmake
# Source: robot_platform/CMakeLists.txt
add_test(NAME test_actuator_gateway COMMAND test_actuator_gateway)
set_tests_properties(test_actuator_gateway PROPERTIES
  ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
)
```

### Existing Verify-Stage JSON Pattern
```python
# Source: robot_platform/tools/platform_cli/main.py
smoke_stage: dict[str, object] = {
    "name": "smoke",
    "status": smoke_status,
    "exit_code": smoke_rc,
    "duration_s": smoke_duration_s,
    "observed_outputs": [],
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Phase 1 proof by runtime-output presence only | Phase 2 needs case-based safety verdicts | 2026-03-31 planning boundary | Observation alone is no longer sufficient; explicit safety reasons are required. |
| Minimal host coverage of `message_center` and `actuator_gateway` | Host verification must now cover current control path and injection seams | Phase 2 scope definition | New C targets should expand from seam tests into current-path safety checks. |
| Treat SITL adapters as smoke enablers | Treat SITL adapters as deterministic injection seams | Phase 2 context decisions | Fake IMU/remote behavior becomes a deliberate safety-testing mechanism. |
| Preserve default device wrapper by habit | Simplify `device_layer` if it obstructs verification | Phase 2 context decisions | Seam clarity is more important than preserving wrapper shape. |

**Deprecated/outdated for this phase:**
- “Observe `actuator_command` once” as the whole proof: too weak for SAFE-01 through SAFE-06.
- “Standalone `sim` is the authority”: contradicted by current context and Phase 1 verification history.
- “Current architecture docs are already realized”: not true for the active transported path.

## Open Questions

1. **What is the canonical safe behavior for stale command input?**
   - What we know: Phase 2 must fail if lost or stale command input does not transition to a defined safe behavior.
   - What's unclear: Whether that behavior should be fully disabled output, recover mode, zero-current wheels, or another explicit degraded mode.
   - Recommendation: Lock this in planning before writing verdict artifacts, because SAFE-05 depends on a concrete oracle.

2. **Which wheel-leg danger signatures should be first?**
   - What we know: Scope is intentionally narrow and should cover one or two explicit danger signatures.
   - What's unclear: The exact first signatures with the best value-to-effort ratio for current code.
   - Recommendation: Choose signatures that can be expressed from current support-state, pitch, and actuator output evidence without adding broad new instrumentation.

3. **Should `device_layer` be simplified in Phase 2 or only instrumented?**
   - What we know: The context explicitly allows simplification or replacement if needed.
   - What's unclear: Whether narrow fake-device hooks can be added cleanly without more refactor.
   - Recommendation: Decide after a short spike on deterministic IMU/remote injection; do not preserve the wrapper if it obscures ownership or hides fault-state evidence.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| `python3` | CLI/report tests and orchestration | Yes | 3.11.9 | — |
| `cmake` | Configure host and SITL builds | Yes | 3.23.0 | — |
| `ctest` | Run host C regression targets | Yes | 3.23.0 | — |
| `gcc` | Host C compilation | Yes | 13.4.0 | `clang` 14.0.0 for investigation only |
| `clang` | Optional alternate compiler investigation | Yes | 14.0.0 | `gcc` remains primary |
| `ninja` | Firmware/SITL build paths in CLI | Yes | 1.10.1 | Unix Makefiles already supported |
| `arm-none-eabi-gcc` | Hardware-target build path | Yes | 10.3.1 20210621 | Not needed for core Phase 2 host proof |
| `java` | CubeMX backend prerequisite | Yes | 21.0.10 | — |
| `STM32CubeMX` on `PATH` | Firmware generation path | No | — | Not required for Phase 2 host-safety critical path |

**Missing dependencies with no fallback:**
- None for planning or for the core host-side safety-verification path.

**Missing dependencies with fallback:**
- `STM32CubeMX` is absent on `PATH`, but Phase 2 can proceed using host tests, SITL build, and Python report validation without firmware generation.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | CTest 3.23.0 for host C targets; Python `unittest` on Python 3.11.9 for CLI/runner tests |
| Config file | none; test registration lives in `robot_platform/CMakeLists.txt` and Python modules |
| Quick run command | `ctest --test-dir build/robot_platform_host_tests --output-on-failure` |
| Full suite command | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v && python3 -m unittest robot_platform.sim.tests.test_runner -v && ctest --test-dir build/robot_platform_host_tests --output-on-failure` |

### Current Verified Baseline

- `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` passed on 2026-03-31 in this environment.
- `python3 -m unittest robot_platform.sim.tests.test_runner -v` passed on 2026-03-31 in this environment.
- `ctest --test-dir build/robot_platform_host_tests --output-on-failure` passed on 2026-03-31 in this environment.

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| HOST-02 | Cover current path seams beyond `message_center` and `actuator_gateway` | host C integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R "test_balance_.*|test_device_profile_.*"` | No, Wave 0 |
| HOST-03 | Deterministic sensor, remote, and link injection | host C + Python orchestration | `python3 -m robot_platform.tools.platform_cli.main verify phase2 --project balance_chassis --report build/verification_reports/phase2_balance_chassis.json` | No, Wave 0 |
| SAFE-01 | Invalid mapping/profile direction blocks output | host C | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_mapping` | No, Wave 0 |
| SAFE-02 | Stale/invalid sensor data blocks or degrades output | host C integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_sensor_faults` | No, Wave 0 |
| SAFE-03 | Invalid arming/state-machine transitions cannot arm control | host C integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_arming` | No, Wave 0 |
| SAFE-04 | Saturation is enforced and explicitly judged | host C | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_saturation` | No, Wave 0 |
| SAFE-05 | Lost/stale command input enters defined safe behavior | host C + Python orchestration | `python3 -m robot_platform.tools.platform_cli.main verify phase2 --project balance_chassis --case stale_command` | No, Wave 0 |
| SAFE-06 | Wheel-leg danger signatures regressions | host C integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_wheel_leg` | No, Wave 0 |

### Sampling Rate
- **Per task commit:** `ctest --test-dir build/robot_platform_host_tests --output-on-failure`
- **Per wave merge:** `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v`, `python3 -m unittest robot_platform.sim.tests.test_runner -v`, then the full host C suite
- **Phase gate:** A new `verify phase2` command must run a deterministic case matrix and emit a passed JSON report before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] `robot_platform/runtime/tests/host/test_balance_safety_path.c` for authoritative current-path safety cases
- [ ] `robot_platform/runtime/tests/host/test_device_profile_safety_seams.c` for IMU/remote/profile injection seams
- [ ] `robot_platform/runtime/tests/host/test_safety_mapping.c` for SAFE-01
- [ ] `robot_platform/runtime/tests/host/test_safety_sensor_faults.c` for SAFE-02
- [ ] `robot_platform/runtime/tests/host/test_safety_arming.c` for SAFE-03
- [ ] `robot_platform/runtime/tests/host/test_safety_saturation.c` for SAFE-04
- [ ] `robot_platform/runtime/tests/host/test_safety_wheel_leg.c` for SAFE-06
- [ ] `robot_platform/tools/platform_cli/main.py` support for `verify phase2`
- [ ] `robot_platform/tools/platform_cli/tests/test_main.py` coverage for Phase 2 report schema and failure modes
- [ ] `robot_platform/sim/tests/test_runner.py` coverage for new verdict artifact parsing, if Phase 2 reuses runner summaries

## Sources

### Primary (HIGH confidence)
- Local code:
  - `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c`
  - `robot_platform/runtime/control/state/observe_task.c`
  - `robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.c`
  - `robot_platform/runtime/control/execution/motor_control_task.c`
  - `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`
  - `robot_platform/runtime/device/device_layer.c`
  - `robot_platform/runtime/device/device_profile_sitl.c`
  - `robot_platform/runtime/device/imu/bmi088_device_sitl.c`
  - `robot_platform/runtime/device/remote/dbus_remote_device_sitl.c`
  - `robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c`
  - `robot_platform/runtime/control/execution/actuator_gateway.c`
  - `robot_platform/runtime/control/controllers/balance_controller.c`
  - `robot_platform/runtime/control/constraints/actuator_constraints.c`
  - `robot_platform/runtime/control/state/ins_state_estimator.c`
  - `robot_platform/CMakeLists.txt`
  - `robot_platform/tools/platform_cli/main.py`
  - `robot_platform/runtime/tests/host/test_actuator_gateway.c`
  - `robot_platform/tools/platform_cli/tests/test_main.py`
  - `robot_platform/sim/tests/test_runner.py`
- Planning context:
  - `.planning/phases/02-host-safety-control-verification/02-CONTEXT.md`
  - `.planning/REQUIREMENTS.md`
  - `.planning/ROADMAP.md`
  - `.planning/STATE.md`
  - `.planning/phases/01-contracts-and-verification-foundation/01-VERIFICATION.md`
- Official docs:
  - CMake `ctest(1)`: https://cmake.org/cmake/help/latest/manual/ctest.1.html
  - GCC instrumentation options: https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html

### Secondary (MEDIUM confidence)
- `CLAUDE.md` project constraints and architecture guidance

### Tertiary (LOW confidence)
- None

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - verified against local repo usage, local environment versions, and official CMake/GCC docs.
- Architecture: HIGH - derived directly from current runtime code and locked Phase 2 context decisions.
- Pitfalls: HIGH - backed by observed current code behavior, passing tests, and Phase 2 requirement gaps.

**Research date:** 2026-03-31
**Valid until:** 2026-04-30
