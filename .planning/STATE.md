---
gsd_state_version: 1.0
milestone: v2.0
milestone_name: Platform Simplification
status: active
stopped_at: completed v2-02-01-PLAN.md
last_updated: "2026-04-02T16:04:00Z"
last_activity: 2026-04-02
progress:
  total_phases: 4
  completed_phases: 1
  total_plans: 4
  completed_plans: 3
  percent: 37
---

# Project State

## Project Reference

See: `.planning/PROJECT.md` (updated 2026-04-01)

**Core value:** 缩减平台代码复杂度，让架构更直接、更易理解、更易测试，同时保持 v1 建立的安全验证能力。
**Current focus:** v2.0 Platform Simplification — Phase 2: Seam Migration (in progress)

## Current Position

Milestone: v2.0 — Platform Simplification
Phase: 2 of 4 (Seam Migration) — in progress
Plan: 1 of 2 completed
Status: In progress
Last activity: 2026-04-02 — Completed v2-02-01-PLAN.md

Progress: [██████          ] 37%

## Performance Metrics

- Phases: 1/4 completed
- Plans: 3/4 completed (v2-01: 2/2, v2-02: 1/2)
- Requirements: 0/12 satisfied (SLIM-04 partially satisfied by plan 01)

## Accumulated Context

### Key Decisions
- Migration order follows research-recommended sequence: command unification -> BSP ports -> test seam migration -> control migration -> readiness gate extraction -> device layer deletion -> topic consolidation -> directory flattening
- 4 phases derived from 8 migration steps, grouped by delivery boundary
- `test_balance_safety_path` is the critical invariant — must stay green at every step
- Unified command type uses indexed arrays (`joints[N]`, `wheels[N]`) with named enum accessors — `platform_device_command_t` kept as typedef alias for backward compatibility
- BSP port implementations delegate to device_layer default API as transitional bridge; motor_write_command wraps motor_command_set into actuator_command_t for compatibility
- ports_fake_set_hooks installs bridge adapters into device_layer test hooks so tasks still calling device_layer default API get routed through ports_fake
- Replaced bsp_ports_sitl.c with ports_fake.c in balance_safety_host_runtime to avoid duplicate port symbols
- test_device_profile_safety_seams uses hybrid approach — port hooks via ports_fake, init_default_profile via device_layer (not a port concept)

### Todos
- None

### Blockers
- None

## Session Continuity

Last session: 2026-04-02T16:04:00Z
Stopped at: Completed v2-02-01-PLAN.md
Resume file: None
Next action: Execute v2-02-02-PLAN.md (task migration to call ports directly)
