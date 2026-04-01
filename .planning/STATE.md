---
gsd_state_version: 1.0
milestone: v2.0
milestone_name: Platform Simplification
status: active
stopped_at: completed v2-03-03-PLAN.md
last_updated: "2026-04-02T16:30:00Z"
last_activity: 2026-04-02
progress:
  total_phases: 4
  completed_phases: 2
  total_plans: 10
  completed_plans: 5
  percent: 50
---

# Project State

## Project Reference

See: `.planning/PROJECT.md` (updated 2026-04-01)

**Core value:** 缩减平台代码复杂度，让架构更直接、更易理解、更易测试，同时保持 v1 建立的安全验证能力。
**Current focus:** v2.0 Platform Simplification — Phase 3: Device Layer Removal (in progress)

## Current Position

Milestone: v2.0 — Platform Simplification
Phase: 3 of 4 (Device Layer Removal) — in progress
Plan: 3 of 3 completed (v2-03-01, v2-03-02 executed externally; v2-03-03 just completed)
Status: Phase 3 plans complete
Last activity: 2026-04-02 — Completed v2-03-03-PLAN.md

Progress: [██████████      ] 50%

## Performance Metrics

- Phases: 2/4 completed (Phase 3 plans done, pending phase-level verification)
- Plans: 5/10 completed (v2-01: 2/2, v2-02: 2/2, v2-03: 1/3 with summary)
- Requirements: 0/12 satisfied (SLIM-04 partially satisfied by plan 01)

## Accumulated Context

### Key Decisions
- Migration order follows research-recommended sequence: command unification -> BSP ports -> test seam migration -> control migration -> readiness gate extraction -> device layer deletion -> topic consolidation -> directory flattening
- 4 phases derived from 8 migration steps, grouped by delivery boundary
- `test_balance_safety_path` is the critical invariant — must stay green at every step
- Unified command type uses indexed arrays (`joints[N]`, `wheels[N]`) with named enum accessors — `platform_device_command_t` kept as typedef alias for backward compatibility
- BSP port implementations delegate to device_layer default API as transitional bridge; motor_write_command wraps motor_command_set into actuator_command_t for compatibility
- ports_fake is now standalone — no device_layer dependency, intercepts port calls natively
- Replaced bsp_ports_sitl.c with ports_fake.c in balance_safety_host_runtime to avoid duplicate port symbols
- test_device_profile_safety_seams verifies port calls directly; init_default_profile still tested via device_layer hooks
- Keep platform_device_init_default_profile call in actuator_gateway — HW/SITL port implementations still delegate to device_layer
- device_layer_stubs.c/h are unused by any test target — candidate for deletion in Phase 3
- Standalone readiness gates take raw Subscriber_t* pointers — no dependency on bus wrapper structs
- Original wait_ready functions kept as thin wrappers delegating to readiness.c — Phase 4 deletes wrappers

### Todos
- None

### Blockers
- None

## Session Continuity

Last session: 2026-04-02T16:30:00Z
Stopped at: Completed v2-03-03-PLAN.md
Resume file: None
Next action: Begin Phase 4 (Consolidation) or complete remaining Phase 3 summaries
