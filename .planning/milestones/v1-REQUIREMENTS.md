# v1 Requirements Archive: Astra RM Robot Platform

**Defined:** 2026-03-30
**Completed:** 2026-04-01
**Core Value:** Make wheeled-legged Robotmaster control software safe to evolve by catching dangerous control and data-link errors before the robot ever gets a chance to go unstable on hardware.

## Build Pipeline — All Complete

- [x] **PIPE-01**: Developer can run one documented command path that performs build, host-side tests, SITL smoke validation, and firmware generation for `balance_chassis` (Phase 5)
- [x] **PIPE-02**: Build/test/generate failures return machine-readable results that identify which stage failed (Phase 5)
- [x] **PIPE-03**: The project detects when generated STM32 artifacts are stale relative to the checked-in board source inputs before firmware output is trusted (Phase 1)

## Host Verification — All Complete

- [x] **HOST-01**: Developer can run host-native C tests for safety-critical runtime modules without requiring robot hardware (Phase 1)
- [x] **HOST-02**: Host-side tests cover message transport, control-path logic, device/profile binding seams, and actuator command mapping on the current `balance_chassis` path (Phase 2)
- [x] **HOST-03**: Host-side verification can inject fake sensor, remote, and data-link inputs to exercise runtime behavior deterministically (Phase 2)
- [x] **HOST-04**: Host-side verification reports sanitizer failures for memory-safety or undefined-behavior defects in supported test targets (Phase 1)

## Safety Gates — All Complete

- [x] **SAFE-01**: Runtime blocks actuator output when control direction or command mapping is invalid for the active robot profile (Phase 2)
- [x] **SAFE-02**: Runtime blocks or degrades actuator output when sensor data is stale, invalid, or unavailable (Phase 2)
- [x] **SAFE-03**: Runtime blocks invalid enable/state-machine transitions that could arm control in an unsafe state (Phase 2)
- [x] **SAFE-04**: Runtime enforces configured output saturation and fails verification when limits are violated (Phase 2)
- [x] **SAFE-05**: Runtime detects data-link loss or stale command input and transitions to a defined safe behavior (Phase 2)
- [x] **SAFE-06**: Verification includes a regression path for wheel-leg coupling instability risks identified for `balance_chassis` (Phase 2)

## Fake-Link Validation — All Complete

- [x] **LINK-01**: Sim/fake-link adapters drive the real runtime control path rather than stub-only placeholder behavior (Phase 3)
- [x] **LINK-02**: Validation captures observable runtime outputs for the fake-link path, not only declared expectations (Phase 3)
- [x] **LINK-03**: Verification can distinguish communication-path failures from control-path failures in its output artifacts (Phase 3)
- [x] **LINK-04**: Topic, port, or contract mismatches between runtime and sim declarations fail validation explicitly (Phase 3)

## Runtime Observability — All Complete

- [x] **OBS-01**: Smoke and verification runs emit machine-readable artifacts with adapter-binding status, validation outcomes, and failure reasons (Phase 3)
- [x] **OBS-02**: Verification artifacts expose counters or diagnostics for dropped packets, stale inputs, or missing runtime observations where applicable (Phase 3)
- [x] **OBS-03**: The authoritative `balance_chassis` bring-up path is documented well enough that developers know which runtime path is blessed and which legacy paths are not (Phase 4)

## Platform Architecture — All Complete

- [x] **ARCH-01**: The platform defines one authoritative ownership boundary between orchestration, device adapters, control logic, and robot-project composition (Phase 4)
- [x] **ARCH-02**: Runtime contracts and message transport reject unsafe payload sizing instead of silently allowing overflow-prone behavior (Phase 1)
- [x] **ARCH-03**: The current platform architecture is reviewed for unnecessary coupling and overdesign, with v1 changes focused on making the existing reusable direction testable rather than broader (Phase 4)
- [x] **ARCH-04**: `balance_chassis` remains the proving path for the reusable platform, without collapsing the platform into one-off robot-specific shortcuts (Phase 4)

## Coverage

- v1 requirements: 24 total
- Satisfied: 24
- Unsatisfied: 0

---
*Archived: 2026-04-01*
