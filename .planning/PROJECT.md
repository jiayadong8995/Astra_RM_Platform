# Astra RM Robot Platform

## What This Is

This project is a Robotmaster-oriented embedded robot platform, with the current platform direction centered on a wheeled-legged robot (`balance_chassis`) as the first real integration target. It aims to unify firmware generation, cross-target builds, host-side verification, and controlled bring-up so the team can evolve robot control software with confidence instead of relying on risky first-on-robot debugging.

The immediate project focus is not full competition readiness. The roadmap is centered on a safe v1 foundation: host-side TDD, fake-link validation, and a reliable build-to-firmware pipeline, while the longer-term project goal points at a constrained real-robot bring-up for the wheeled-legged minimum closed loop.

## Core Value

Make wheeled-legged Robotmaster control software safe to evolve by catching dangerous control and data-link errors before the robot ever gets a chance to go unstable on hardware.

## Requirements

### Validated

- ✓ Build the same runtime stack for STM32 hardware and Linux SITL targets — existing
- ✓ Compose the runtime into layered build, device, control, app, and simulation surfaces — existing
- ✓ Generate STM32 board sources through the current CubeMX-backed workflow — existing
- ✓ Run Python-side CLI, simulation orchestration, and smoke-report plumbing for the `balance_chassis` project — existing

### Active

- [ ] Establish a host-side TDD workflow that tests robot control logic and data-link behavior before hardware bring-up
- [ ] Make the compile -> test_case -> firmware generation path reliable enough to use as the default development loop
- [ ] Use fake data-link validation to separate communication faults from control faults before real-robot testing
- [ ] Define and enforce safety gates that block risky control changes from reaching on-robot validation
- [ ] Review the current platform architecture and reduce unnecessary implementation coupling without abandoning the decision to build a reusable platform
- [ ] Reach a v1 foundation where host-side verification is trusted, while keeping the project trajectory aimed at v2 constrained wheeled-legged bring-up

### Out of Scope

- Directly optimizing for full competition-ready robot behavior in the current roadmap — the immediate priority is safe engineering closure, not end-state feature breadth
- Treating simulation data as a full substitute for real hardware truth — sim exists to catch logic and contract failures early, not to prove final real-world behavior
- Narrowing the platform into a one-off `balance_chassis` codebase only — the project explicitly chooses a reusable platform direction even though it increases upfront cost

## Context

The repository is already a brownfield platform with a layered embedded runtime in `robot_platform/runtime/`, a project-configured build surface in `robot_platform/CMakeLists.txt`, and a Python simulation/tooling sidecar in `robot_platform/sim/` and `robot_platform/tools/`. The current active robot target is `robot_platform/projects/balance_chassis/`, and the codebase already supports both STM32H723 hardware builds and Linux SITL builds.

The main project pressure is not lack of structure, but lack of trustworthy closure. The platform shape exists, yet several pieces are still more scaffold than proven system: implementation coupling remains high, the current testing surface is too light for control-heavy work, and the team does not yet trust the stack enough to put it on the real robot. That gap is especially visible in host-side TDD, which is currently the biggest source of frustration.

Safety drives the project definition. Before any serious on-robot attempt, the system must prove that it will not exhibit known "crazy robot" failure modes such as inverted outputs, broken saturation, continuing closed-loop control on bad sensor data, invalid state-machine enable transitions, stale data after link loss, or wheel-leg coupling that diverges when posture changes. Fake data-link testing is also a required diagnostic layer so that real bring-up can quickly distinguish control defects from communication defects.

The current roadmap is intentionally split by maturity level. v1 focuses on host-side TDD, fake-link validation, and a reliable build/test/generate loop. The project still ultimately targets v2, where a constrained wheeled-legged minimum closed loop can be brought up on hardware under safety gates. This split exists because simulated data is necessarily fake, so the project needs staged validation rather than pretending sim alone proves hardware readiness.

Codebase concerns already identified in `.planning/codebase/CONCERNS.md` materially affect this project definition: the message bus has unsafe fixed payload sizing, the SITL device path is not yet wired to meaningful runtime adapters, and the runtime C control path has major test coverage gaps. These are not side issues; they directly block the desired TDD and safe bring-up workflow.

## Constraints

- **Platform direction**: Build a reusable Robotmaster robot platform, not a one-off robot app — this is a deliberate project choice even though it increases design pressure early
- **Safety**: On-robot testing must be gated by pre-hardware verification — avoiding uncontrolled robot behavior is more important than fast manual bring-up
- **Validation model**: Host-side tests and fake links must catch logic and contract faults, but cannot be treated as final physical proof — the roadmap must preserve staged progression from fake data to constrained hardware validation
- **Architecture**: Existing runtime layering should be preserved where it provides real leverage, but implementation-level coupling and overdesign need active review — current complexity is a project risk
- **Build environment**: Firmware generation currently depends on STM32CubeMX, cross-compilers, and local host setup — the platform must work within that toolchain reality while making the loop more reliable
- **Current target**: `balance_chassis` is the first concrete robot profile — the roadmap should use it as the proving ground without collapsing the entire platform abstraction into this single target

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Keep the project oriented around a reusable platform instead of narrowing immediately to a single robot | The intended end state is a Robotmaster platform, and the user explicitly prefers maintaining generality even at higher upfront cost | — Pending |
| Split success criteria into staged maturity levels (`v1` host-side validation, later `v2` constrained hardware bring-up) | Simulated data is always fake, so the project needs a layered safety case instead of a single pass/fail milestone | — Pending |
| Treat host-side TDD and fake data-link verification as first-class product requirements, not tooling nice-to-haves | The primary current blocker is lack of trust before hardware bring-up, and this trust gap is what causes the user to avoid on-robot testing | — Pending |
| Define "safe to bring up" by explicit failure modes to prevent | The project must block inverted outputs, broken limits, invalid state transitions, stale-link control, and unstable wheel-leg coupling before real tests | — Pending |
| Plan a focused architecture review of platform weight and coupling as part of current scope | The existing architecture has structure, but the implementation may be overdesigned and too tightly coupled for effective TDD | — Pending |

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `$gsd-transition`):
1. Requirements invalidated? -> Move to Out of Scope with reason
2. Requirements validated? -> Move to Validated with phase reference
3. New requirements emerged? -> Add to Active
4. Decisions to log? -> Add to Key Decisions
5. "What This Is" still accurate? -> Update if drifted

**After each milestone** (via `$gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check -> still the right priority?
3. Audit Out of Scope -> reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-03-30 after initialization*
