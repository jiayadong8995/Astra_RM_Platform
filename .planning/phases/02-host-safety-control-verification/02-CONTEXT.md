# Phase 2: Host Safety Control Verification - Context

**Gathered:** 2026-03-31
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase turns the current `balance_chassis` runtime path into a host-side safety verification chain that is injectible, observable, and decisively judgeable. It must exercise the current real control path rather than a bypass harness, and it must prove that known unsafe behaviors are blocked before the project treats simulation or later hardware work as trustworthy. This phase does not yet broaden into fake-link runtime proof ownership questions from Phase 3 or platform-authority simplification work from Phase 4.

</domain>

<decisions>
## Implementation Decisions

### Phase-Defining Priority
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

### the agent's Discretion
- The exact host-side safety harness may combine host C tests and structured Python orchestration as long as the authoritative path still runs through the current `balance_chassis` runtime behavior.
- The exact machine-readable schema for Phase 2 verdict artifacts may be selected by planning/research as long as it can distinguish injection case, observed outputs, and safety failure reason.
- The precise first-wave danger signatures for wheel-leg coupling may be selected by the agent if they stay narrow, explicit, and tied to the current code path.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Phase Scope and Requirements
- `.planning/ROADMAP.md` — Phase 2 goal, requirement mapping, and success criteria
- `.planning/REQUIREMENTS.md` — Phase 2 requirements `HOST-02`, `HOST-03`, `SAFE-01` through `SAFE-06`
- `.planning/PROJECT.md` — project-level trust model and v1 focus on pre-hardware safety evidence
- `.planning/STATE.md` — current focus after Phase 1 closure and recent decisions
- `.planning/phases/01-contracts-and-verification-foundation/01-VERIFICATION.md` — verified baseline that established the current minimum live path and its limits

### Current Runtime Path
- `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c` — current remote-input ingress task
- `robot_platform/runtime/control/state/observe_task.c` — current observer task in the active runtime path
- `robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.c` — current balance-controller task on the active path
- `robot_platform/runtime/control/execution/motor_control_task.c` — current actuator execution task on the active path
- `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c` — active runtime bus bindings and current `actuator_command` observability hook
- `robot_platform/runtime/control/state/observe_topics.c` — current observer-side topic ingress
- `robot_platform/runtime/control/execution/actuator_topics.c` — current execution-side topic ingress/egress

### Injection and Binding Seams
- `robot_platform/runtime/device/device_layer.c` — primary device/profile seam for deterministic fake sensor and remote injection
- `robot_platform/runtime/device/device_layer.h` — current default-device facade and global layer contract
- `robot_platform/runtime/device/device_profile_sitl.c` — current SITL device binding composition
- `robot_platform/runtime/device/imu/bmi088_device_sitl.c` — current IMU SITL input source
- `robot_platform/runtime/device/remote/dbus_remote_device_sitl.c` — current remote SITL input source
- `robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c` — current SITL command/feedback loop
- `robot_platform/runtime/module/message_center/message_center.h` — authoritative topic transport contract
- `robot_platform/runtime/module/message_center/message_center.c` — authoritative topic transport behavior

### Safety-Critical Logic and Contracts
- `robot_platform/runtime/app/balance_chassis/app_config/robot_def.h` — current robot-definition facade that currently aliases internal balance params
- `robot_platform/runtime/control/internal/balance_params.h` — current mixed robot/control parameter header and coupling hotspot
- `robot_platform/runtime/app/balance_chassis/app_intent/remote_intent.c` — start/enable and emergency-stop intent shaping
- `robot_platform/runtime/control/controllers/balance_controller.c` — current control-path logic and actuator-command construction
- `robot_platform/runtime/control/execution/actuator_gateway.c` — current command-mapping and dispatch gating seam
- `robot_platform/runtime/control/state/chassis_observer.c` — current thin observer implementation and state-layer review target
- `robot_platform/runtime/control/state/ins_state_estimator.c` — current INS estimation seam, warmup policy, and message projection path
- `robot_platform/runtime/control/contracts/robot_intent.h` — control enable contract
- `robot_platform/runtime/control/contracts/actuator_command.h` — runtime output contract to be judged
- `robot_platform/runtime/control/contracts/device_feedback.h` — feedback contract consumed by the active path
- `robot_platform/runtime/control/contracts/robot_state.h` — runtime state contract used by intent and safety reasoning

### Existing Verification Entry Points
- `robot_platform/tools/platform_cli/main.py` — current `verify phase1`, `sim`, and test/build orchestration
- `robot_platform/sim/core/runner.py` — current smoke-report structure and runtime output parsing
- `robot_platform/sim/projects/balance_chassis/profile.py` — current SITL project profile and required runtime output declaration
- `robot_platform/sim/tests/test_runner.py` — existing report-oriented regression examples
- `robot_platform/tools/platform_cli/tests/test_main.py` — existing CLI/report regression examples

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- Phase 1 already proved that the repository can produce machine-readable verification output and observe live `actuator_command` traffic through the current SITL path.
- `platform_cli.main` is already the natural orchestration point for a richer Phase 2 verification entry once safety-case injection and verdicts are defined.
- `chassis_topics.c` already exposes the first concrete runtime-output observation seam that can be extended carefully without inventing a parallel control path.
- `remote_intent.c` already centralizes start/enable behavior and is the obvious first seam for invalid arming and stale-command expectations.
- `actuator_gateway.c` already enforces one minimal safety property (`control_enable && actuator_enable`) and is the natural execution-side seam for command-mapping and dispatch safety checks.

### Established Patterns
- The active path is task-driven and topic-connected today; Phase 2 should treat that as the implementation truth.
- Existing verification artifacts are JSON-first and summary-oriented; Phase 2 should continue that pattern.
- The current runtime already distinguishes contract layers (`robot_intent`, `device_feedback`, `actuator_command`) even though they are still transported by `message_center`.
- Deterministic SITL inputs currently come from simple backend implementations; this is sufficient for narrow fault injection if planning keeps the scope tight.
- Several current named layers are not equally mature: `ins_state_estimator` has a real estimation role, while `chassis_observer` is still a very thin seam and `device_layer` currently behaves more like a global default wrapper than a stable ownership boundary.

### Integration Points
- Sensor/remote injection should plug into the device/profile bindings rather than bypassing the runtime tasks.
- Link-loss and stale-transport checks should integrate with the current topic/runtime-ingress path instead of inventing a separate synthetic control harness.
- Safety verdicts should ultimately be derived from observed runtime outputs plus key enable-state evidence, not from internal developer interpretation alone.
- If `device_layer` prevents clear injection or keeps device ownership ambiguous, Phase 2 may restructure that seam instead of layering more tests on top of it.

</code_context>

<specifics>
## Specific Ideas

- The first concrete Phase 2 proof should be: inject a deterministic case into the current runtime path, observe `actuator_command` plus its enable bits, and machine-judge whether the system blocked or emitted unsafe output.
- Phase 2 should favor a small matrix of high-value safety cases over a broad but shallow collection of integration shells.
- The repository should stop speaking as if the direct-interface architecture principle is already realized; Phase 2 validates the current transported main path as it really exists.
- Standalone `sim` instability should be treated as context for planning, not hidden. The phase should prefer the currently verified path as the initial authoritative execution path.
- The project should feel free to simplify over-wrapped seams that directly obstruct host-side safety verification, especially the current default device wrapper and mixed robot/control parameter headers.

</specifics>

<deferred>
## Deferred Ideas

- Broad fake-link observability and communication/control fault separation belong mainly to Phase 3.
- Platform ownership simplification and direct-interface architectural cleanup belong mainly to Phase 4.
- Richer stability proof or replay-based fault libraries belong to later v1/v2 work once the first safety-verdict chain exists.

</deferred>

---
*Phase: 02-host-safety-control-verification*
*Context gathered: 2026-03-31*
