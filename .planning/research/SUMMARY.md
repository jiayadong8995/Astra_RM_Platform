# Project Research Summary

**Project:** Astra RM Robot Platform
**Domain:** Safety-sensitive embedded Robotmaster wheeled-legged control platform
**Researched:** 2026-03-30
**Confidence:** HIGH

## Executive Summary

This repo is not missing a new framework. It is missing a trustworthy validation path for the existing one. The research consistently points to the same v1 goal: keep the current `C11 + CMake + Python + STM32CubeMX` platform, but add a host-native C verification lane that proves the real control contracts, safety logic, and adapter wiring before SITL or hardware. For this stage, experts would build a staged pipeline, not a richer simulator or broader platform surface: `host contract/unit tests -> fake-link validation -> SITL smoke -> firmware build/generate -> constrained hardware bring-up`.

The recommended approach is to thin and clarify the architecture rather than replace it. `runtime/control` should own state, intent, control, safety, and execution math; `runtime/device` should own backend-specific adapters only; orchestration should own lifecycle only; `projects/balance_chassis` should define composition and limits, not hide platform semantics. The highest-value corrections are explicit contract sizing, a first-class safety gate before actuator writes, and observable runtime outputs that let fake-link and SITL prove the control path is actually connected.

The main risks are false confidence and unsafe coupling. A green loop that only tests Python orchestration, a message bus that can overflow contract payloads, and SITL built around stub adapters will all look like progress while leaving the dangerous failure modes intact. Mitigation is straightforward: make contract-size checks mandatory, require host-side C tests on safety-critical runtime seams, reject SITL success when outputs are `declared_only` or adapters are stubbed, and gate any hardware-facing path on explicit safety-state behavior.

## Key Findings

### Recommended Stack

The stack direction is conservative by design. Keep the repo’s current embedded foundation and extend it with verification tooling that fits the existing build graph. The right v1 change is not a migration to ROS 2, Gazebo, Ceedling, or a second test framework. It is one CMake graph with host-native test targets, sanitizer-enabled runs, and lightweight C fakes around device and transport seams.

**Core technologies:**
- `C11` runtime code under `robot_platform/runtime/` remains the primary implementation surface.
- `CMake + CTest` should stay the single build graph across hardware, SITL, and new host tests.
- `Python 3` should stay on the orchestration side for CLI, smoke summaries, and report validation only.
- `Unity` should be vendored for host-side C assertions on runtime modules.
- `FFF` should be vendored for lightweight fake devices and link seams.
- `ASan + UBSan` should be enabled on host verification targets, especially around `message_center`.
- `gcovr` should report coverage for host C tests only, focused on hazard-bearing modules first.
- `STM32CubeMX 6.17.0` stays in the workflow, but outside the fast inner loop.

### Expected Features

V1 is about pre-hardware trust for `balance_chassis`, not end-state competition breadth. The expected baseline is a reproducible loop that proves the real runtime behavior, captures machine-readable evidence, and blocks known unsafe states before anything reaches hardware.

**Must have (table stakes):**
- Reliable `build -> host test -> SITL smoke -> firmware generate` loop.
- Host-side C tests for control-path runtime logic.
- Fake data-link validation with observable runtime outputs.
- Safety gates for stale data, invalid enable transitions, saturation, and unsafe output conditions.
- Deterministic startup, disable, and fault handling.
- Basic runtime observability: smoke JSON, counters, adapter-binding status, contract mismatch reporting.
- One authoritative bring-up path for `projects/balance_chassis`.

**Should have (competitive):**
- Fault-injection matrix for link, sensor, and actuator degradation.
- Replayable regression corpus from SITL and later hardware traces.

**Defer (v2+):**
- Constrained closed-loop hardware bring-up workflow as a polished product feature.
- Generalizing the platform to a second robot profile before `balance_chassis` is proven.
- Advanced wheeled-legged control layers and richer operator dashboards.

### Architecture Approach

The architecture recommendation is to keep the current platform split but make ownership singular and directional. Contracts define typed, size-checked runtime boundaries; ports isolate HW/SITL/fake adapters; control owns state, intent, controller, safety, and execution transforms; orchestration owns lifecycle and scheduling; sim owns fake-link input and output observation. The control path must stay backend-agnostic, and robot-specific semantics must live in `projects/<robot_profile>` composition and limits rather than leaking across runtime layers.

**Major components:**
1. `projects/<robot_profile>` - robot selection, limits, task graph, and safety policy composition.
2. `runtime/contracts` and `runtime/platform_bus` - typed messages, topic ids, and transport guarantees.
3. `runtime/device` - backend-specific sensor, remote, actuator, and link adapters behind narrow ports.
4. `runtime/control` - state, intent, controllers, safety gate, and actuator execution mapping.
5. `runtime/orchestration` - startup order, scheduling, lifecycle, and health transitions.
6. `sim/adapters` and `sim/projects/*` - fake-link input, runtime observation export, smoke assertions, and reports.

### Critical Pitfalls

1. **Green tests that miss the real control path** - require host-side C tests on contracts, control, execution, startup, and device-profile binding before calling CI trustworthy.
2. **Message transport violating contract sizes** - add compile-time size assertions and reject oversized topic registrations instead of relying on unchecked `memcpy()` into fixed 64-byte buffers.
3. **Treating stubbed SITL as verification** - fail validation when adapters are still stubs or runtime outputs are not actually observed.
4. **Split ownership across app, control, and device layers** - assign one owner per behavior boundary and keep legacy app code out of runtime semantics.
5. **No explicit safety state machine before actuator writes** - centralize arming, freshness, degraded mode, and transition guards in one auditable supervisor stage.

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: Contract and Verification Foundation
**Rationale:** Nothing else is trustworthy until runtime contracts, transport sizing, and host test entrypoints are real.
**Delivers:** Size-checked contracts, host C test targets in the existing CMake graph, sanitizer-enabled runs, initial Unity/FFF harness, and fast-fail checks for generated-artifact drift.
**Addresses:** Reliable build/test/generate loop, host-side control-path tests, basic observability.
**Avoids:** False green CI, message bus corruption, CubeMX drift.

### Phase 2: Safety and Control-Path Closure
**Rationale:** Once tests can run natively, the next highest-risk gap is unsafe or implicit control behavior.
**Delivers:** Explicit state/intent/controller/execution seams, first-class safety supervisor, deterministic startup and disable handling, and hazard-focused regression cases.
**Uses:** C11 runtime, CMake/CTest, Unity/FFF, sanitizers.
**Implements:** `runtime/control` ownership model with orchestration separated from control math.

### Phase 3: Fake-Link and Runtime Observability
**Rationale:** Pre-hardware confidence requires proof that simulated inputs drive the real runtime path and produce observable outputs.
**Delivers:** Real fake-link adapters, runtime output exporter, topic-name parity checks, machine-readable observation artifacts, and non-stub adapter validation.
**Addresses:** Fake data-link validation, runtime observability, single authoritative bring-up path.
**Avoids:** SITL that only proves plumbing, not behavior.

### Phase 4: SITL Hardening and Gated Bring-Up
**Rationale:** SITL becomes useful only after contracts, safety, and observability are grounded.
**Delivers:** Trustworthy `test sim` smoke lane, replayable regression inputs, constrained promotion gates to firmware build/generate, and documented bring-up criteria for `balance_chassis`.
**Addresses:** End-to-end pre-hardware closure and later hardware gatekeeping.
**Avoids:** Jumping from toy simulation to uncontrolled robot tests.

### Phase Ordering Rationale

- Contract sizing and host-native tests come first because all later safety and simulation evidence depends on them.
- Safety closure comes before richer SITL because unsafe control behavior is a bigger risk than incomplete simulation fidelity.
- Fake-link observability belongs before hardening SITL smoke because the repo currently lacks proof that runtime outputs match declared validations.
- Broader reuse, richer hardware workflows, and advanced control research should wait until `balance_chassis` passes the staged trust pipeline reliably.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 2:** Safety supervisor design and actuator capability mapping need careful review against existing runtime/control and motor semantics.
- **Phase 3:** SITL adapter parity and observation export need implementation-specific validation because current bindings appear partially stubbed.
- **Phase 4:** Constrained hardware bring-up criteria should be researched again once host and SITL evidence are in place.

Phases with standard patterns (skip research-phase):
- **Phase 1:** Host-side C test integration, sanitizer wiring, and coverage reporting are established patterns and already fit the repo shape.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Strongly grounded in the repo’s existing build graph, toolchains, and CLI flow. |
| Features | HIGH | Table stakes align closely with the repo’s stated blockers and current project phase. |
| Architecture | HIGH | Recommendations are driven by visible ownership leaks and boundary problems in the current runtime layout. |
| Pitfalls | HIGH | Main risks are backed by direct local evidence in transport, SITL bindings, and control-path gaps. |

**Overall confidence:** HIGH

### Gaps to Address

- Exact host test harness shape for FreeRTOS-adjacent runtime code needs validation during Phase 1 implementation.
- The final transport fix for `message_center` is still a design choice: strict payload cap enforcement vs per-topic storage sizing.
- Current SITL adapter fidelity and topic-name parity need to be revalidated once observation export exists.
- The minimal safe actuator capability matrix for wheels vs joints should be confirmed before claiming reusable execution semantics.

## Sources

### Primary (HIGH confidence)
- `.planning/research/STACK.md` - stack direction and repo-fit build/test recommendations.
- `.planning/research/FEATURES.md` - v1 table stakes, differentiators, and anti-features.
- `.planning/research/ARCHITECTURE.md` - ownership model, boundaries, and build order.
- `.planning/research/PITFALLS.md` - critical failure modes and phase warnings.
- `.planning/PROJECT.md` and `.planning/codebase/CONCERNS.md` - current project goals, blockers, and repo maturity.
- `robot_platform/CMakeLists.txt` and `robot_platform/tools/platform_cli/main.py` - existing build graph and orchestration path.

### Secondary (MEDIUM confidence)
- PX4 and ArduPilot SITL documentation - supports using simulation as staged validation, not final hardware proof.
- ROS 2 Control hardware testing guidance - supports offline hardware-interface testing as standard practice.
- External wheeled-legged control literature cited in `FEATURES.md` - supports deferring advanced control research until the validation stack is stable.

---
*Research completed: 2026-03-30*
*Ready for roadmap: yes*
