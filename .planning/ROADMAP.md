# Roadmap: Astra RM Robot Platform

## Overview

This roadmap turns the existing embedded platform into a trustworthy v1 validation loop for `balance_chassis`: first make contracts and host verification reliable, then prove safety-critical control behavior, then prove fake-link runtime wiring and observability, then narrow the platform to one authoritative bring-up path, and finally make the full build/test/generate closure the default developer loop. The outcome is a practical pre-hardware trust stack that blocks risky changes before any constrained v2 hardware bring-up.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [x] **Phase 1: Contracts and Verification Foundation** - Make host-native verification, contract sizing, and stage reporting trustworthy.
- [x] **Phase 2: Host Safety Control Verification** - Prove `balance_chassis` control-path and safety behavior with deterministic host-side tests.
- [ ] **Phase 3: Fake-Link Runtime Proof** - Drive the real runtime through fake-link adapters and capture observable validation artifacts.
- [ ] **Phase 4: Authoritative Platform Composition** - Clarify ownership boundaries and define one blessed `balance_chassis` bring-up path.
- [ ] **Phase 5: Default Closure Loop** - Make one command path the default trusted loop for build, verification, smoke, and firmware generation.

## Phase Details

### Phase 1: Contracts and Verification Foundation
**Goal**: Developers can trust the host verification entrypoints, machine-readable stage results, and contract-size enforcement before deeper control validation begins.
**Depends on**: Nothing (first phase)
**Requirements**: PIPE-02, PIPE-03, HOST-01, HOST-04, ARCH-02
**Success Criteria** (what must be TRUE):
  1. Developer can run host-native C tests for supported safety-critical runtime modules without robot hardware.
  2. Verification reports clearly identify whether build, test, smoke, or generation failed in machine-readable output.
  3. Firmware generation is refused when checked-in STM32-generated artifacts are stale relative to their source inputs.
  4. Unsafe runtime contract or transport payload sizing is rejected explicitly instead of being silently accepted.
  5. Supported host verification targets surface sanitizer failures when memory-safety or undefined-behavior defects occur.
**Plans**: 5 plans
Plans:
- [x] 01-01-PLAN.md - Build the minimal host harness and checked-in host test entrypoint.
- [x] 01-02-PLAN.md - Harden `message_center` transport sizing and registration-time safety checks.
- [x] 01-03-PLAN.md - Add secondary actuator seam coverage once the core proof path is stable.
- [x] 01-04-PLAN.md - Prove the minimum live SITL path and JSON-first phase verification entrypoint.
- [x] 01-05-PLAN.md - Add generated-artifact freshness hard gating for hardware-trusting builds.

### Phase 2: Host Safety Control Verification
**Goal**: Developers can deterministically exercise the real `balance_chassis` control path on host and prove that known unsafe control behaviors are blocked before simulation or hardware.
**Depends on**: Phase 1
**Requirements**: HOST-02, HOST-03, SAFE-01, SAFE-02, SAFE-03, SAFE-04, SAFE-05, SAFE-06
**Success Criteria** (what must be TRUE):
  1. Developer can inject fake sensor, remote, and link inputs into host verification and observe the real control path respond deterministically.
  2. Verification fails when control direction, actuator command mapping, or output saturation is invalid for the active `balance_chassis` profile.
  3. Verification proves actuator output is blocked or degraded when sensor data is stale, invalid, unavailable, or command input is lost.
  4. Verification proves invalid arming or state-machine transitions are rejected before closed-loop control can engage.
  5. The host regression suite includes explicit cases for wheel-leg coupling instability risks on the current robot path.
**Plans**: 3 plans
Plans:
- [x] 02-01-PLAN.md - Build the deterministic host harness and `verify phase2` entrypoint for the current task/topic path.
- [x] 02-02-PLAN.md - Add hard safety oracles for mapping, sensor validity, arming transitions, and saturation.
- [x] 02-03-PLAN.md - Close stale-command and wheel-leg danger-signature coverage and finalize the Phase 2 verdict matrix.

### Phase 3: Fake-Link Runtime Proof
**Goal**: Developers can prove that fake-link and sim inputs drive the real runtime path and produce observable artifacts that separate communication faults from control faults.
**Depends on**: Phase 2
**Requirements**: LINK-01, LINK-02, LINK-03, LINK-04, OBS-01, OBS-02
**Success Criteria** (what must be TRUE):
  1. Fake-link or sim adapters drive the same runtime control path used by validation, not placeholder stub-only behavior.
  2. Validation artifacts capture observable runtime outputs and adapter-binding status, not only declared expectations.
  3. Verification output distinguishes communication-path failures from control-path failures in machine-readable artifacts.
  4. Contract mismatches such as topic, port, or declaration drift fail validation explicitly.
  5. Developers can inspect diagnostics for dropped packets, stale inputs, or missing runtime observations in verification artifacts.
**Plans**: 3 plans
Plans:
- [x] 03-01-PLAN.md - Rebind SITL IMU and remote ingress to runtime-backed fake-link adapters instead of stub-only devices.
- [x] 03-02-PLAN.md - Add `verify phase3` and persist runtime-output plus adapter-binding evidence in authoritative JSON artifacts.
- [ ] 03-03-PLAN.md - Classify communication/control/observation failures, fail contract drift explicitly, and expose diagnostics.

### Phase 4: Authoritative Platform Composition
**Goal**: Developers have one explicit ownership model and one authoritative `balance_chassis` bring-up path that reduces coupling without abandoning the reusable platform direction.
**Depends on**: Phase 3
**Requirements**: ARCH-01, ARCH-03, ARCH-04, OBS-03
**Success Criteria** (what must be TRUE):
  1. Developers can identify one authoritative ownership boundary between orchestration, device adapters, control logic, and project composition.
  2. The blessed `balance_chassis` bring-up path is documented clearly enough that developers can tell which runtime path is current and which legacy paths are not.
  3. The current platform shape is simplified where needed to make testing and validation practical, without collapsing into one-off robot-specific shortcuts.
  4. `balance_chassis` remains the proving path for the reusable platform rather than a special-case bypass around it.
**Plans**: TBD

### Phase 5: Default Closure Loop
**Goal**: Developers can use one trusted command path as the default inner loop for `balance_chassis`, with earlier validation gates enforced before firmware output is considered usable.
**Depends on**: Phase 4
**Requirements**: PIPE-01
**Success Criteria** (what must be TRUE):
  1. Developer can run one documented command path that performs build, host-side verification, fake-link or SITL smoke validation, and firmware generation for `balance_chassis`.
  2. The command path stops at the failing stage and does not treat later outputs as trusted when earlier gates fail.
  3. A successful run leaves machine-readable evidence that the validated stages completed before firmware output was produced.
**Plans**: TBD

## Progress

**Execution Order:**
Phases execute in numeric order: 1 -> 2 -> 3 -> 4 -> 5

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Contracts and Verification Foundation | 5/5 | Complete | 2026-03-31 |
| 2. Host Safety Control Verification | 3/3 | Complete | 2026-03-31 |
| 3. Fake-Link Runtime Proof | 0/TBD | Not started | - |
| 4. Authoritative Platform Composition | 0/TBD | Not started | - |
| 5. Default Closure Loop | 0/TBD | Not started | - |
