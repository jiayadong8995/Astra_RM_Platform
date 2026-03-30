# Domain Pitfalls

**Domain:** Safety-sensitive embedded Robotmaster wheeled-legged robot platform
**Researched:** 2026-03-30

## Critical Pitfalls

Mistakes that cause rewrites or major issues.

### Pitfall 1: Trusting a green test loop that does not execute the real control path
**What goes wrong:** The team believes TDD is in place because `test sim` stays green, but the automated tests only cover Python CLI parsing and smoke-report summarization. The C runtime path that computes state, publishes commands, maps them to devices, and gates startup remains effectively untested.
**Why it happens:** Brownfield embedded projects often bolt tests onto orchestration first because it is easier than isolating RTOS tasks, device adapters, and controller code. That creates a false sense of verification.
**Consequences:** Control regressions are discovered only in SITL or on hardware, exactly when failures are most expensive and dangerous. "Crazy robot" classes such as inverted outputs, stale-link control, and broken enable logic can slip through CI.
**Prevention:** Treat host-side tests as incomplete until they execute the C contracts and controller/execution seams directly. Add contract-level tests for `message_center`, `balance_controller`, `actuator_gateway`, startup sequencing, and device-profile binding. Make "safe to merge" depend on these tests, not only on Python smoke tests.
**Detection:** CI passes while SITL still reports `declared_only`, motor feedback unavailable, or stub device names. Test count remains low and concentrated in `robot_platform/sim/tests/` and `robot_platform/tools/platform_cli/tests/`.

### Pitfall 2: Letting message transport violate control contracts
**What goes wrong:** Runtime topics publish structs that exceed the message bus storage model, which uses `uint8_t data_buf[64]` with unchecked `memcpy()`. Large control contracts such as `platform_robot_state_t`, `platform_actuator_command_t`, and `platform_device_feedback_t` can overflow subscriber buffers.
**Why it happens:** Reusable platform efforts often start with a small static pub-sub bus and assume topic payloads will stay "small enough". As abstractions mature, structs grow faster than transport guarantees.
**Consequences:** Silent memory corruption, nondeterministic startup behavior, SITL/hardware divergence, task faults, and actuator commands derived from corrupted state. In a safety-sensitive bring-up, this is a direct route to uncontrolled motion.
**Prevention:** Make payload size part of the contract. Reject topic registration when the payload exceeds the configured maximum, or move to per-topic storage sized at registration. Add compile-time assertions for every published contract and runtime counters for registration failures and pool saturation.
**Detection:** Random control behavior that disappears under logging, inconsistent subscriber state across tasks, crashes near message publish/read sites, and any contract expansion that is not accompanied by transport-size checks.

### Pitfall 3: Building SITL around stubs and then treating it as evidence
**What goes wrong:** The repository compiles SITL-capable BSP code, but the device profile currently binds to stub IMU, remote, and motor devices that return unavailable or invalid data. Validation also lacks runtime output observations, so reports can look structurally healthy without proving the control loop is connected.
**Why it happens:** Brownfield teams often get the build and bridge working before binding the real semantic adapters, then postpone the last 20 percent of wiring because the harness already "runs".
**Consequences:** Simulation exercises startup plumbing rather than controller behavior. Teams debug the wrong layer, overestimate readiness, and bring unstable assumptions onto hardware.
**Prevention:** Do not call SITL "verification" until the runtime path consumes meaningful simulated inputs and emits observable outputs tied to declared validation targets. Require parity checks between profile declarations, published topic names, and adapter observations.
**Detection:** Device names containing `*_stub`, `collect_runtime_output_observations()` returning empty results, smoke reports stuck at `declared_only`, and controller loops waiting forever for valid feedback.

### Pitfall 4: Reusable platform abstractions that leak ownership across layers
**What goes wrong:** Legacy task code, new device/control contracts, and app orchestration all retain partial ownership of the same behaviors. The system becomes "platformized" in structure but not in authority.
**Why it happens:** Brownfield reuse programs try to preserve working code while introducing new layers, but stop halfway. This leaves duplicate responsibilities and implicit cross-layer assumptions.
**Consequences:** Refactors are high risk because changing one layer breaks another layer's hidden expectations. TDD becomes hard because tests cannot identify the single authoritative seam. Safety logic gets split between app code, controller code, and device code, so nobody can prove enable/disable behavior end to end.
**Prevention:** Define one owner for each capability boundary and force all new work through it. The legacy app layer should orchestrate lifecycle and mode flow only; state formation, control, execution, and device semantics need singular ownership. Add boundary tests that fail if app code reaches past the contract layer.
**Detection:** The same concept appears in both legacy task files and new runtime modules, reviews keep asking "which layer owns this?", and changes require synchronized edits across `app`, `control`, and `device` for one behavior.

### Pitfall 5: No explicit safety state machine before actuator writes
**What goes wrong:** Control enable, actuator enable, sensor validity, and startup readiness are inferred from message contents or booleans spread across contracts, not enforced by a single auditable safety state machine.
**Why it happens:** Teams focus on controller math first and postpone supervision logic, especially when simulation and TDD are still immature.
**Consequences:** The robot can continue computing or dispatching commands on invalid IMU/remote data, stale feedback, or impossible mode transitions. These are exactly the pre-hardware failure modes the project is trying to block.
**Prevention:** Implement a single supervisor that owns arm/disarm, degraded mode, stale-data timeout, and transition guards before `platform_device_write_default_command()` can run. Test it with explicit fault-injection cases: lost IMU, invalid remote frame, stale actuator feedback, and mode flapping.
**Detection:** Busy-wait startup loops, per-module validity flags with no central arbitration, actuator dispatch paths that ignore freshness/timeouts, and inability to answer "what exact condition prevents torque output right now?"

## Moderate Pitfalls

### Pitfall 1: Porting TDD by mocking the wrong layer
**What goes wrong:** Tests mock high-level outputs or Python orchestration instead of testing contract transformations and state/controller logic with deterministic fixtures.
**Prevention:** Write most host tests at the C contract seam. Use fake device inputs and assert resulting `RobotState`, `ActuatorCommand`, and safety decisions. Keep only a thin layer of process-level smoke tests.

### Pitfall 2: Simulation semantics diverge from hardware semantics
**What goes wrong:** SIM packets, runtime topics, and hardware commands use different naming, timing, units, or validity rules.
**Prevention:** Maintain one formal contract per boundary and reuse it in hardware, SITL, and validation. Every sim adapter field should map to a runtime contract, not to legacy internal structs.

### Pitfall 3: Abstractions erase actuator mode meaning
**What goes wrong:** A generic platform command shape exists, but the backend collapses different motor modes into a narrower hardware behavior. In this repo, joint writes currently send MIT torque commands with zeroed position/velocity targets regardless of broader command semantics.
**Prevention:** Treat actuator mode support as a capability matrix, not as a generic struct. Test every joint and wheel mode path end to end before calling the abstraction reusable.

### Pitfall 4: Startup readiness depends on timing accidents
**What goes wrong:** Tasks poll topics until something looks valid, with latest-value-wins semantics and no explicit initialization handshake.
**Prevention:** Replace implicit readiness with explicit init events, timestamps, and timeout handling. Add tests that vary task ordering and delayed first messages.

### Pitfall 5: Fault-injection stops at transport errors
**What goes wrong:** Teams test dropped packets or process startup failures but not semantically dangerous cases such as sign inversion, timestamp freeze, stale-but-valid packets, or leg/wheel coupling mismatch.
**Prevention:** Build a fault matrix around the project's named failure modes and require each one to have a host-side test before hardware bring-up.

## Minor Pitfalls

### Pitfall 1: Generated-code drift is treated as a tooling annoyance
**What goes wrong:** CubeMX-generated sources drift from the checked-in IOC and the breakage is discovered late.
**Prevention:** Add a freshness check and pin the supported CubeMX workflow in automation.

### Pitfall 2: Local SITL transport is trusted too much
**What goes wrong:** UDP localhost transport is assumed benign, so malformed or cross-run packets contaminate smoke sessions.
**Prevention:** Add session IDs or protocol headers, validate packet sizes, and surface dropped/rejected packet counters.

### Pitfall 3: Coverage discussions center on percentages instead of hazard closure
**What goes wrong:** The team starts optimizing for line coverage without covering the failure modes that matter most to bring-up safety.
**Prevention:** Track hazard-based coverage first: command saturation, stale-link shutdown, invalid-state inhibition, startup ordering, and actuator mode mapping.

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation |
|-------------|---------------|------------|
| Host-side TDD foundation | Declaring victory after adding more Python tests while the C runtime remains untested | Start with contract-size, safety-state, and controller/execution seam tests in C or host-buildable runtime units |
| Fake-link validation | Validating bridge metadata instead of validating runtime behavior | Require observable runtime outputs and topic-name parity before accepting fake-link results |
| SITL adapter wiring | Binding profiles to stubs because the bridge already launches | Make adapter binding and non-stub device identity a hard precondition for SITL success |
| Platform abstraction review | Preserving both legacy and new ownership paths "for flexibility" | Remove duplicate authority and publish a single boundary map per capability |
| Safety gates before bring-up | Encoding safety decisions as scattered booleans instead of a supervisor | Centralize arm/disarm, freshness, degraded-mode, and transition guards in one auditable module |
| Generated firmware pipeline | Allowing IOC/codegen drift to silently alter runtime behavior | Add freshness checks and fail the pipeline when generated artifacts are stale |
| Constrained hardware bring-up | Jumping from toy SITL directly to free-running robot tests | Insert staged bring-up: command logging only, outputs clamped, partial actuator enable, then constrained closed loop |

## Sources

- Local project brief: `/home/xbd/worspcae/code/Astra_RM2025_Balance/.planning/PROJECT.md` (HIGH)
- Local codebase concerns audit: `/home/xbd/worspcae/code/Astra_RM2025_Balance/.planning/codebase/CONCERNS.md` (HIGH)
- Local boundary definition: `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/docs/runtime_capability_boundaries.md` (HIGH)
- Local runtime transport: `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/module/message_center/message_center.h` and `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/module/message_center/message_center.c` (HIGH)
- Local SITL binding and adapter evidence: `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/device/device_profile_sitl.c`, `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/device/imu/bmi088_device_sitl.c`, `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/device/remote/dbus_remote_device_sitl.c`, `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c`, `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/sim/projects/balance_chassis/bridge_adapter.py` (HIGH)
- Local command-dispatch evidence: `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/execution/actuator_gateway.c` and `/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/device/actuator/motor/motor_actuator_device_hw.c` (HIGH)
- PX4 simulation guidance, emphasizing simulation as early-safe testing and distinguishing SITL from HITL: https://docs.px4.io/main/en/simulation/ and https://docs.px4.io/main/en/simulation/hitl (MEDIUM)
- ArduPilot SITL guidance, emphasizing SITL for failure-mode and environment testing rather than as final hardware proof: https://ardupilot.org/dev/docs/using-sitl-for-ardupilot-testing.html (MEDIUM)
