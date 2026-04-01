# v1 Roadmap Archive: Astra RM Robot Platform

**Milestone:** v1 — Safe Pre-Hardware Validation Foundation
**Completed:** 2026-04-01
**Phases:** 6 (18 plans)
**Commits:** 82
**Requirements:** 24/24 satisfied

## Overview

This roadmap turned the existing embedded platform into a trustworthy v1 validation loop for `balance_chassis`: first make contracts and host verification reliable, then prove safety-critical control behavior, then prove fake-link runtime wiring and observability, then narrow the platform to one authoritative bring-up path, and finally make the full build/test/generate closure the default developer loop. The outcome is a practical pre-hardware trust stack that blocks risky changes before any constrained v2 hardware bring-up.

## Key Accomplishments

1. Host-native C test infrastructure with ASan/UBSan, covering message transport, actuator gateway, and 6 safety oracles (SAFE-01..06)
2. Deterministic host harness driving the live balance_chassis task/topic chain with fake sensor, remote, and link injection
3. Runtime-backed SITL fake-link adapters with observable artifacts that separate communication faults from control faults
4. Authoritative platform ownership model with shared app-startup API, control-owned task registration, and one blessed bring-up path
5. Single `validate` CLI command sequencing build → host tests → python tests → smoke → verify → firmware with early-exit and closure artifact
6. Full validate pipeline coverage of all 11 host CTest targets with residual coupling cleaned up

## Phases

### Phase 1: Contracts and Verification Foundation
**Goal**: Developers can trust the host verification entrypoints, machine-readable stage results, and contract-size enforcement before deeper control validation begins.
**Requirements**: PIPE-02, PIPE-03, HOST-01, HOST-04, ARCH-02
**Plans**: 5 plans (01-01 through 01-05)
**Completed**: 2026-03-31

### Phase 2: Host Safety Control Verification
**Goal**: Developers can deterministically exercise the real `balance_chassis` control path on host and prove that known unsafe control behaviors are blocked before simulation or hardware.
**Requirements**: HOST-02, HOST-03, SAFE-01, SAFE-02, SAFE-03, SAFE-04, SAFE-05, SAFE-06
**Plans**: 3 plans (02-01 through 02-03)
**Completed**: 2026-03-31

### Phase 3: Fake-Link Runtime Proof
**Goal**: Developers can prove that fake-link and sim inputs drive the real runtime path and produce observable artifacts that separate communication faults from control faults.
**Requirements**: LINK-01, LINK-02, LINK-03, LINK-04, OBS-01, OBS-02
**Plans**: 3 plans (03-01 through 03-03)
**Completed**: 2026-04-01

### Phase 4: Authoritative Platform Composition
**Goal**: Developers have one explicit ownership model and one authoritative `balance_chassis` bring-up path that reduces coupling without abandoning the reusable platform direction.
**Requirements**: ARCH-01, ARCH-03, ARCH-04, OBS-03
**Plans**: 3 plans (04-01 through 04-03)
**Completed**: 2026-04-01

### Phase 5: Default Closure Loop
**Goal**: Developers can use one trusted command path as the default inner loop for `balance_chassis`, with earlier validation gates enforced before firmware output is considered usable.
**Requirements**: PIPE-01, PIPE-02
**Plans**: 2 plans (05-01 through 05-02)
**Completed**: 2026-04-01

### Phase 6: v1 Tech Debt Cleanup
**Goal**: Close accumulated tech debt identified by the v1 milestone audit so the validation pipeline covers all host tests and residual coupling is reduced.
**Requirements**: None (tech debt closure)
**Plans**: 2 plans (06-01 through 06-02)
**Completed**: 2026-04-01

## Progress

| Phase | Plans | Status | Completed |
|-------|-------|--------|-----------|
| 1. Contracts and Verification Foundation | 5/5 | Complete | 2026-03-31 |
| 2. Host Safety Control Verification | 3/3 | Complete | 2026-03-31 |
| 3. Fake-Link Runtime Proof | 3/3 | Complete | 2026-04-01 |
| 4. Authoritative Platform Composition | 3/3 | Complete | 2026-04-01 |
| 5. Default Closure Loop | 2/2 | Complete | 2026-04-01 |
| 6. v1 Tech Debt Cleanup | 2/2 | Complete | 2026-04-01 |

## Known Residual Items

- Hardware build resolves to host `cc` instead of `arm-none-eabi-gcc` — hardware-target behavioral verification not yet usable as a trusted gate
- `main.py:1005` command placeholder fallback for unimplemented CLI commands (flash/debug/replay)

---
*Archived: 2026-04-01*
