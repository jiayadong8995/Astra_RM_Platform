---
milestone: v1
audited: 2026-04-01T22:30:00Z
status: tech_debt
scores:
  requirements: 24/24
  phases: 5/5
  integration: 8/8
  flows: 4/4
gaps:
  requirements: []
  integration: []
  flows: []
tech_debt:
  - phase: 02-host-safety-control-verification
    items:
      - "Missing formal VERIFICATION.md — phase has 3/3 plan summaries with self-checks and a VALIDATION.md, but no gsd-verifier report was produced"
  - phase: 03-fake-link-runtime-proof
    items:
      - "Warning: SITL main_sitl.c uses escaped \\n sequences that collapse log lines, weakening runner fallback parsing for [RuntimeOutput] records"
  - phase: 04-authoritative-platform-composition
    items:
      - "Warning: control_task_registry.c still includes ../../app/balance_chassis/app_config/app_params.h directly — residual app-to-control coupling in task priority/stack sizing"
      - "Warning: Hardware build uses host cc instead of Cortex-M cross-compiler — hardware-target behavioral verification cannot be used as a trusted gate"
      - "Info: main.py:1005 command placeholder fallback for unimplemented CLI commands (flash/debug/replay)"
  - phase: cross-phase
    items:
      - "Phase 1 host tests (test_message_center, test_actuator_gateway) not included in validate pipeline's host_tests stage — message_center sizing edge cases could be missed"
      - "Phase 3 and Phase 4 host tests (test_device_profile_sitl_runtime_bindings, test_balance_app_startup) not gated by validate pipeline — partially covered by verify_phase3 stage"
      - "Three orphaned host test targets exist but are not wired into any verify or validate command: test_device_profile_safety_seams, test_device_profile_sitl_runtime_bindings, test_balance_app_startup"
---

# v1 Milestone Audit Report

**Milestone:** v1 — Safe Pre-Hardware Validation Foundation
**Audited:** 2026-04-01T22:30:00Z
**Status:** tech_debt (all requirements met, no critical blockers, accumulated debt needs review)

## Requirements Coverage

All 24 v1 requirements are satisfied across 5 phases.

| Requirement | Description | Phase | Status |
|-------------|-------------|-------|--------|
| PIPE-01 | One documented command path for build + test + smoke + firmware | Phase 5 | ✓ Satisfied |
| PIPE-02 | Machine-readable results identify which stage failed | Phase 5 | ✓ Satisfied |
| PIPE-03 | Detect stale STM32 generated artifacts before firmware output is trusted | Phase 1 | ✓ Satisfied |
| HOST-01 | Host-native C tests for safety-critical runtime modules without hardware | Phase 1 | ✓ Satisfied |
| HOST-02 | Host-side tests cover message transport, control-path, device/profile binding, actuator mapping | Phase 2 | ✓ Satisfied |
| HOST-03 | Host-side verification can inject fake sensor, remote, and data-link inputs | Phase 2 | ✓ Satisfied |
| HOST-04 | Host-side verification reports sanitizer failures | Phase 1 | ✓ Satisfied |
| SAFE-01 | Runtime blocks actuator output when control direction or mapping is invalid | Phase 2 | ✓ Satisfied |
| SAFE-02 | Runtime blocks or degrades actuator output when sensor data is stale/invalid | Phase 2 | ✓ Satisfied |
| SAFE-03 | Runtime blocks invalid enable/state-machine transitions | Phase 2 | ✓ Satisfied |
| SAFE-04 | Runtime enforces configured output saturation | Phase 2 | ✓ Satisfied |
| SAFE-05 | Runtime detects data-link loss or stale command input | Phase 2 | ✓ Satisfied |
| SAFE-06 | Verification includes wheel-leg coupling instability regression | Phase 2 | ✓ Satisfied |
| LINK-01 | Sim/fake-link adapters drive the real runtime control path | Phase 3 | ✓ Satisfied |
| LINK-02 | Validation captures observable runtime outputs, not only declared expectations | Phase 3 | ✓ Satisfied |
| LINK-03 | Verification distinguishes communication-path from control-path failures | Phase 3 | ✓ Satisfied |
| LINK-04 | Topic, port, or contract mismatches fail validation explicitly | Phase 3 | ✓ Satisfied |
| OBS-01 | Smoke and verification runs emit machine-readable artifacts | Phase 3 | ✓ Satisfied |
| OBS-02 | Verification artifacts expose counters/diagnostics for dropped packets, stale inputs | Phase 3 | ✓ Satisfied |
| OBS-03 | Authoritative bring-up path is documented clearly | Phase 4 | ✓ Satisfied |
| ARCH-01 | One authoritative ownership boundary between orchestration, device, control, composition | Phase 4 | ✓ Satisfied |
| ARCH-02 | Runtime contracts reject unsafe payload sizing | Phase 1 | ✓ Satisfied |
| ARCH-03 | Platform architecture reviewed for unnecessary coupling | Phase 4 | ✓ Satisfied |
| ARCH-04 | balance_chassis remains the proving path for the reusable platform | Phase 4 | ✓ Satisfied |

**Score: 24/24 requirements satisfied**

## Phase Verification Status

| Phase | Plans | Verification | Status |
|-------|-------|-------------|--------|
| 1. Contracts and Verification Foundation | 5/5 complete | 01-VERIFICATION.md: passed (5/5 truths) | ✓ Verified |
| 2. Host Safety Control Verification | 3/3 complete | ⚠️ No VERIFICATION.md (summaries + self-checks present) | ✓ Complete, unverified |
| 3. Fake-Link Runtime Proof | 3/3 complete | 03-VERIFICATION.md: passed (10/10 truths) | ✓ Verified |
| 4. Authoritative Platform Composition | 3/3 complete | 04-VERIFICATION.md: passed (4/4 truths) | ✓ Verified |
| 5. Default Closure Loop | 2/2 complete | 05-VERIFICATION.md: passed (5/5 truths) | ✓ Verified |

**Score: 5/5 phases complete, 4/5 formally verified**

Phase 2 is the only phase without a formal gsd-verifier VERIFICATION.md. However, all 3 plan summaries include self-checks, the `verify phase2` command produces a passing SAFE-01..06 JSON matrix, and all Phase 2 host tests pass. The missing verification is a documentation gap, not a functional gap.

## Cross-Phase Integration

| Connection | Status | Details |
|-----------|--------|---------|
| Phase 1 → Phase 2 | ✓ Connected | Phase 2 harness links real message_center.c with ASan/UBSan from Phase 1 infrastructure |
| Phase 2 → Phase 3 | ✓ Connected | SITL profile binds same runtime chain that Phase 2 tests drive deterministically |
| Phase 3 → Phase 4 | ✓ Connected | verify phase3 works after Phase 4 restructuring; authoritative_bringup metadata added |
| Phase 4 → Phase 5 | ✓ Connected | validate pipeline invokes verify_phase3 as Stage 5, exercising Phase 3+4 surface |
| Phase 1 → Phase 5 | ✓ Connected | Freshness gating preserved in validate's hw_elf stage |
| Phase 2 → Phase 5 | ✓ Connected | All 6 SAFE cases gated through validate's host_tests stage |
| Runtime chain consistency | ✓ Consistent | Same authoritative chain across Phase 2 host tests, Phase 3 SITL, and Phase 4 ownership |
| PHASE3_RUNTIME_CHAIN string | ✓ Consistent | Identical in runner.py and main.py |

**Score: 8/8 integration points connected**

## E2E Flows

| Flow | Status | Details |
|------|--------|---------|
| Developer runs `validate` | ✓ Complete | 5-stage pipeline with early-exit and closure artifact |
| Safety gate enforcement | ✓ Complete | SAFE-01..06 all gated through host_tests; "passed" closure requires all green |
| Freshness gating | ✓ Complete | hw_elf gated by freshness check; failure → skipped, not blocked |
| Failure localization | ✓ Complete | Closure artifact records failure_stage and failure_reason |

**Score: 4/4 E2E flows complete**

## Tech Debt

### Phase 2: Missing Formal Verification
- No `02-VERIFICATION.md` was produced by gsd-verifier. Phase completion is evidenced by plan summaries and self-checks, but the formal verification step was skipped.

### Phase 3: SITL Log Coalescing
- `main_sitl.c` uses escaped `\n` sequences instead of real newlines, which can collapse SITL log lines and weaken runner fallback parsing for `[RuntimeOutput]` records.

### Phase 4: Residual Coupling and Toolchain
- `control_task_registry.c` still includes `../../app/balance_chassis/app_config/app_params.h` directly for task priority/stack sizing — residual app-to-control coupling.
- Hardware build resolves to host `cc` instead of `arm-none-eabi-gcc`, so hardware-target behavioral verification cannot be used as a trusted gate.
- `main.py:1005` has a pre-existing `command placeholder` fallback for unimplemented CLI commands.

### Cross-Phase: Validate Pipeline Coverage Gaps
- Phase 1 host tests (`test_message_center`, `test_actuator_gateway`) are not included in the validate pipeline's `host_tests` stage. Message_center sizing edge cases could be missed.
- Phase 3/4 host tests (`test_device_profile_sitl_runtime_bindings`, `test_balance_app_startup`) are not gated by the validate pipeline.
- Three orphaned host test targets exist but are not wired into any verify or validate command: `test_device_profile_safety_seams`, `test_device_profile_sitl_runtime_bindings`, `test_balance_app_startup`.

### Total: 8 items across 4 categories

## Anti-Patterns Across Phases

| File | Pattern | Severity | Phase |
|------|---------|----------|-------|
| `main_sitl.c:9,15,19` | Escaped `\n` in printf | Warning | Phase 3 |
| `control_task_registry.c:6` | Direct app_params.h include from control | Warning | Phase 4 |
| `main.py:1005` | Command placeholder fallback | Info | Phase 4 |
| Hardware build | Host cc used instead of cross-compiler | Warning | Phase 4 |

---

*Audited: 2026-04-01T22:30:00Z*
*Auditor: Claude (gsd-audit-milestone)*
