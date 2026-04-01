# Astra RM Robot Platform

## What This Is

This project is a Robotmaster-oriented embedded robot platform, with the current platform direction centered on a wheeled-legged robot (`balance_chassis`) as the first real integration target. It aims to unify firmware generation, cross-target builds, host-side verification, and controlled bring-up so the team can evolve robot control software with confidence instead of relying on risky first-on-robot debugging.

## Core Value

Make wheeled-legged Robotmaster control software safe to evolve by catching dangerous control and data-link errors before the robot ever gets a chance to go unstable on hardware.

## Current State (v1 shipped)

v1 is complete. The platform now has:

- A trusted host-side verification loop with 11 CTest targets covering message transport, actuator gateway, 6 safety oracles (SAFE-01..06), SITL runtime bindings, and app startup
- Runtime-backed SITL fake-link adapters that drive the real control path and produce machine-readable artifacts distinguishing communication faults from control faults
- One authoritative `balance_chassis` bring-up path with explicit ownership boundaries between app composition, device adapters, and control logic
- A single `validate` CLI command that sequences build → host tests → python tests → smoke → verify → firmware with early-exit and closure artifact
- Safety gates that block inverted outputs, broken saturation, invalid arming, stale sensor/command data, and wheel-leg coupling instability before any on-robot attempt

The developer inner loop is: `python3 -m robot_platform.tools.platform_cli.main validate`

## Next Milestone Goals (v2)

The project trajectory points at constrained real-robot bring-up for the wheeled-legged minimum closed loop. v2 requirements (sketched in the archived v1 requirements) include:

- Constrained hardware bring-up workflow with explicit entry criteria gated by v1 safety gates
- Minimum closed-loop wheeled-legged bring-up under restricted conditions
- Hardware artifact correlation with prior host/fake-link evidence
- Trace replay for regression testing known failure cases
- Richer fault-injection scenarios
- Platform reuse proof against a second robot or board profile

## Requirements

### Validated (v1)

All 24 v1 requirements satisfied. See [v1 requirements archive](.planning/milestones/v1-REQUIREMENTS.md).

### Active

*No active requirements. Run `/gsd:new-milestone` to define v2 requirements.*

### Out of Scope

- Directly optimizing for full competition-ready robot behavior in the current roadmap — the immediate priority is safe engineering closure, not end-state feature breadth
- Treating simulation data as a full substitute for real hardware truth — sim exists to catch logic and contract failures early, not to prove final real-world behavior
- Narrowing the platform into a one-off `balance_chassis` codebase only — the project explicitly chooses a reusable platform direction even though it increases upfront cost

## Constraints

- **Platform direction**: Build a reusable Robotmaster robot platform, not a one-off robot app
- **Safety**: On-robot testing must be gated by pre-hardware verification
- **Validation model**: Host-side tests and fake links catch logic and contract faults, but are not final physical proof — staged progression from fake data to constrained hardware validation
- **Architecture**: Existing runtime layering preserved where it provides leverage; coupling actively reviewed
- **Build environment**: Firmware generation depends on STM32CubeMX, cross-compilers, and local host setup
- **Current target**: `balance_chassis` is the first concrete robot profile and the proving ground for the reusable platform

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Keep the project oriented around a reusable platform | The intended end state is a Robotmaster platform; generality preferred even at higher upfront cost | v1 Validated |
| Split success criteria into staged maturity levels (v1 host-side, v2 hardware) | Simulated data is always fake; layered safety case needed | v1 Validated |
| Treat host-side TDD and fake data-link verification as first-class requirements | Primary blocker was lack of trust before hardware bring-up | v1 Validated |
| Define "safe to bring up" by explicit failure modes to prevent | Must block inverted outputs, broken limits, invalid transitions, stale-link control, unstable coupling | v1 Validated |
| Focused architecture review of platform weight and coupling | Implementation was overdesigned and too tightly coupled for effective TDD | v1 Validated |

## Evolution

This document evolves at phase transitions and milestone boundaries.

---
*Last updated: 2026-04-01 after v1 milestone completion*
