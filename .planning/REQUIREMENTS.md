# Requirements: Astra RM Robot Platform

**Defined:** 2026-03-30
**Core Value:** Make wheeled-legged Robotmaster control software safe to evolve by catching dangerous control and data-link errors before the robot ever gets a chance to go unstable on hardware.

## v1 Requirements

### Build Pipeline

- [ ] **PIPE-01**: Developer can run one documented command path that performs build, host-side tests, SITL smoke validation, and firmware generation for `balance_chassis`
- [ ] **PIPE-02**: Build/test/generate failures return machine-readable results that identify which stage failed
- [ ] **PIPE-03**: The project detects when generated STM32 artifacts are stale relative to the checked-in board source inputs before firmware output is trusted

### Host Verification

- [x] **HOST-01**: Developer can run host-native C tests for safety-critical runtime modules without requiring robot hardware
- [ ] **HOST-02**: Host-side tests cover message transport, control-path logic, device/profile binding seams, and actuator command mapping on the current `balance_chassis` path
- [ ] **HOST-03**: Host-side verification can inject fake sensor, remote, and data-link inputs to exercise runtime behavior deterministically
- [x] **HOST-04**: Host-side verification reports sanitizer failures for memory-safety or undefined-behavior defects in supported test targets

### Safety Gates

- [ ] **SAFE-01**: Runtime blocks actuator output when control direction or command mapping is invalid for the active robot profile
- [ ] **SAFE-02**: Runtime blocks or degrades actuator output when sensor data is stale, invalid, or unavailable
- [ ] **SAFE-03**: Runtime blocks invalid enable/state-machine transitions that could arm control in an unsafe state
- [ ] **SAFE-04**: Runtime enforces configured output saturation and fails verification when limits are violated
- [ ] **SAFE-05**: Runtime detects data-link loss or stale command input and transitions to a defined safe behavior
- [ ] **SAFE-06**: Verification includes a regression path for wheel-leg coupling instability risks identified for `balance_chassis`

### Fake-Link Validation

- [ ] **LINK-01**: Sim/fake-link adapters drive the real runtime control path rather than stub-only placeholder behavior
- [ ] **LINK-02**: Validation captures observable runtime outputs for the fake-link path, not only declared expectations
- [ ] **LINK-03**: Verification can distinguish communication-path failures from control-path failures in its output artifacts
- [ ] **LINK-04**: Topic, port, or contract mismatches between runtime and sim declarations fail validation explicitly

### Runtime Observability

- [ ] **OBS-01**: Smoke and verification runs emit machine-readable artifacts with adapter-binding status, validation outcomes, and failure reasons
- [ ] **OBS-02**: Verification artifacts expose counters or diagnostics for dropped packets, stale inputs, or missing runtime observations where applicable
- [ ] **OBS-03**: The authoritative `balance_chassis` bring-up path is documented well enough that developers know which runtime path is blessed and which legacy paths are not

### Platform Architecture

- [ ] **ARCH-01**: The platform defines one authoritative ownership boundary between orchestration, device adapters, control logic, and robot-project composition
- [ ] **ARCH-02**: Runtime contracts and message transport reject unsafe payload sizing instead of silently allowing overflow-prone behavior
- [ ] **ARCH-03**: The current platform architecture is reviewed for unnecessary coupling and overdesign, with v1 changes focused on making the existing reusable direction testable rather than broader
- [ ] **ARCH-04**: `balance_chassis` remains the proving path for the reusable platform, without collapsing the platform into one-off robot-specific shortcuts

## v2 Requirements

### Constrained Hardware Bring-Up

- **HW-01**: Developer can promote a validated change into a constrained hardware bring-up workflow with explicit entry criteria
- **HW-02**: The wheeled-legged robot can complete a minimum closed-loop bring-up under restricted conditions after v1 safety gates pass
- **HW-03**: Hardware bring-up artifacts allow developers to correlate on-robot failures with prior host/fake-link evidence

### Extended Validation

- **VAL-01**: Developer can replay stored SITL or hardware traces to regression-test known failure cases
- **VAL-02**: Verification includes richer fault-injection scenarios across sensors, data links, and actuator feedback degradation

### Platform Reuse

- **PLAT-01**: The reusable platform abstractions are proven against at least one additional robot or board profile beyond `balance_chassis`
- **PLAT-02**: The platform can extend robot-specific control features without reintroducing cross-layer ownership confusion

### Advanced Control

- **CTRL-01**: The wheeled-legged stack supports more advanced balancing and disturbance-rejection behaviors after the validation pipeline is trusted

## Out of Scope

| Feature | Reason |
|---------|--------|
| Full competition breadth such as autonomy, perception, and broad subsystem integration in the current roadmap | The immediate blocker is safe pre-hardware engineering closure, not end-state Robotmaster feature breadth |
| A high-fidelity digital twin before fake-link and runtime-contract correctness are proven | Simulated data is intentionally fake at this stage, so correctness of contracts and safety behavior matters more than realism |
| Advanced control research features before the v1 validation stack is trusted | It increases complexity and debugging surface before the baseline runtime can be verified safely |
| Proving broad multi-robot platform reuse in v1 | The project deliberately keeps a reusable platform direction, but first needs to prove it through one authoritative `balance_chassis` path |
| Rich UI or dashboard work in v1 | Machine-readable validation evidence is more valuable than presentation polish at the current stage |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| PIPE-01 | Phase 5 | Pending |
| PIPE-02 | Phase 1 | Pending |
| PIPE-03 | Phase 1 | Pending |
| HOST-01 | Phase 1 | Complete |
| HOST-02 | Phase 2 | Pending |
| HOST-03 | Phase 2 | Pending |
| HOST-04 | Phase 1 | Complete |
| SAFE-01 | Phase 2 | Pending |
| SAFE-02 | Phase 2 | Pending |
| SAFE-03 | Phase 2 | Pending |
| SAFE-04 | Phase 2 | Pending |
| SAFE-05 | Phase 2 | Pending |
| SAFE-06 | Phase 2 | Pending |
| LINK-01 | Phase 3 | Pending |
| LINK-02 | Phase 3 | Pending |
| LINK-03 | Phase 3 | Pending |
| LINK-04 | Phase 3 | Pending |
| OBS-01 | Phase 3 | Pending |
| OBS-02 | Phase 3 | Pending |
| OBS-03 | Phase 4 | Pending |
| ARCH-01 | Phase 4 | Pending |
| ARCH-02 | Phase 1 | Pending |
| ARCH-03 | Phase 4 | Pending |
| ARCH-04 | Phase 4 | Pending |

**Coverage:**
- v1 requirements: 24 total
- Mapped to phases: 24
- Unmapped: 0

---
*Requirements defined: 2026-03-30*
*Last updated: 2026-03-30 after initial definition*
