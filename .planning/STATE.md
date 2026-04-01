---
gsd_state_version: 1.0
milestone: v2.0
milestone_name: Platform Simplification
status: active
stopped_at: completed v2-03-02-PLAN.md (with SUMMARY)
last_updated: "2026-04-02T16:44:00Z"
last_activity: 2026-04-02
progress:
  total_phases: 4
  completed_phases: 2
  total_plans: 10
  completed_plans: 7
  percent: 70
---

# Project State

## Project Reference

See: `.planning/PROJECT.md` (updated 2026-04-01)

**Core value:** 缩减平台代码复杂度，让架构更直接、更易理解、更易测试，同时保持 v1 建立的安全验证能力。
**Current focus:** v2.0 Platform Simplification — Phase 3: Device Layer Removal (in progress)

## Current Position

Milestone: v2.0 — Platform Simplification
Phase: 3 of 4 (Device Layer Removal) — in progress
Plan: 2 of 3 completed with SUMMARY (v2-03-01, v2-03-02 done; v2-03-03 SUMMARY exists)
Status: Phase 3 plans complete, pending phase-level verification
Last activity: 2026-04-02 — Completed v2-03-02-PLAN.md with SUMMARY

Progress: [██████████████  ] 70%

## Performance Metrics

- Phases: 2/4 completed (Phase 3 plans done, pending phase-level verification)
- Plans: 7/10 completed (v2-01: 2/2, v2-02: 2/2, v2-03: 3/3 with summaries)
- Requirements: 0/12 satisfied (SLIM-04 partially satisfied by plan 01)

## Accumulated Context

### Key Decisions
- Migration order follows research-recommended sequence: command unification -> BSP ports -> test seam migration -> control migration -> readiness gate extraction -> device layer deletion -> topic consolidation -> directory flattening
- 4 phases derived from 8 migration steps, grouped by delivery boundary
- `test_balance_safety_path` is the critical invariant — must stay green at every step
- Unified command type uses indexed arrays (`joints[N]`, `wheels[N]`) with named enum accessors — `platform_device_command_t` kept as typedef alias for backward compatibility
- BSP port implementations now call device drivers directly via lazy-bind pattern — no longer delegate to device_layer
- ports_fake is now standalone — no device_layer dependency, intercepts port calls natively
- Replaced bsp_ports_sitl.c with ports_fake.c in balance_safety_host_runtime to avoid duplicate port symbols
- actuator_gateway_init is now a no-op — ports self-init, platform_device_init_default_profile removed from gateway
- Standalone readiness gates take raw Subscriber_t* pointers — no dependency on bus wrapper structs
- Original wait_ready functions kept as thin wrappers delegating to readiness.c — Phase 4 deletes wrappers
- [D-v2-03-02-01] Driver adapters relocated to bsp/drivers/ (not control/contracts/) — BSP-level code belongs under BSP
- [D-v2-03-02-02] Deleted test_device_profile_safety_seams and test_device_profile_sitl_runtime_bindings — tested dead device_layer concepts
- [D-v2-03-02-03] device_layer.c/h, device_profile.h, device_profile_hw/sitl.c, actuator_device.h all deleted — pure dead code after v2-03-01
- device_types.h relocated to bsp/device_types.h — used by ports.h and remote_task.h
- balance_chassis_device CMake library target removed — no longer needed

### Todos
- None

### Blockers
- None

## Session Continuity

Last session: 2026-04-02T16:44:00Z
Stopped at: Completed v2-03-02-PLAN.md with SUMMARY
Resume file: None
Next action: Phase 3 verification or begin Phase 4 (Consolidation)
