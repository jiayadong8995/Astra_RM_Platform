# Roadmap: Astra RM Robot Platform

## Completed Milestones

- [x] **v1: Safe Pre-Hardware Validation Foundation** — 6 phases, 18 plans, 24/24 requirements. Host-side TDD, fake-link validation, safety gates, authoritative platform composition, and default closure loop for `balance_chassis`. See [archive](.planning/milestones/v1-ROADMAP.md). (2026-03-30 -> 2026-04-01)

## Current Milestone: v2.0 Platform Simplification

**Goal:** 缩减平台代码复杂度，从 5-6 层精简到 4 层（bsp -> control -> app -> module），去掉 device_layer 间接层和 topic wrapper 样板代码，同时保持 v1 建立的安全验证能力。

**Depth:** Standard
**Phases:** 4
**Requirements:** 12/12 mapped

### Phase 1: Port Foundation

**Goal:** Developers have a unified command type and BSP port interface layer that coexists with the current device_layer, establishing the link-time polymorphism pattern without breaking anything.

**Dependencies:** None (first phase)

**Requirements:** SLIM-04, KEEP-03

**Plans:** 2 plans

Plans:
- [x] v2-01-01-PLAN.md — Unify command types (eliminate parallel `platform_actuator_command_t` / `platform_device_command_t`, delete ~160 lines of mapping code)
- [x] v2-01-02-PLAN.md — Introduce BSP port interfaces (`bsp/ports.h` + HW/SITL/fake implementations, wired into CMake)

**Success Criteria:**
1. `platform_actuator_command_t` and `platform_device_command_t` are unified into a single command struct — no field mapping code remains in actuator_gateway or device_layer
2. BSP port header (`bsp/ports.h`) declares `platform_imu_read()`, `platform_remote_read()`, `platform_motor_write_command()` with concrete HW and SITL implementations alongside existing device_layer code
3. Both HW ELF and SITL binary compile and link cleanly with the new port interfaces present (old path still active)
4. All 11 CTest targets pass — zero regressions

### Phase 2: Seam Migration

**Goal:** Host-side tests and control tasks use BSP port interfaces instead of device_layer, proving the new path carries equivalent verification power.

**Dependencies:** Phase 1 (port interfaces must exist)

**Requirements:** KEEP-04, KEEP-01

**Plans:** 2 plans

Plans:
- [x] v2-02-01-PLAN.md — Enhance ports_fake with test injection API and migrate 7 hook-using tests from device_layer hooks to BSP port fakes
- [x] v2-02-02-PLAN.md — Migrate control tasks (ins_task, remote_task, actuator_gateway) to BSP port calls and migrate test_actuator_gateway to ports_fake

**Success Criteria:**
1. All 7 CTest targets that depended on `platform_device_set_test_hooks` now inject through link-time BSP port fakes — no test uses the old hook API
2. `ins_task`, `remote_task`, and `motor_control_task` call BSP port functions instead of device_layer functions
3. All 6 safety oracles (SAFE-01..06) pass on the new code path
4. `test_balance_safety_path` stays green throughout the migration (verified at each sub-step)

### Phase 3: Device Layer Removal

**Goal:** The device_layer indirection is fully deleted — control code reaches hardware through exactly 2 hops (control -> BSP port -> BSP implementation).

**Dependencies:** Phase 2 (all consumers migrated off device_layer)

**Requirements:** SLIM-01, SLIM-02, QUAL-01

**Plans:** 3 plans

Plans:
- [x] v2-03-01-PLAN.md — Rewire BSP port implementations to call drivers directly, relocate device_types.h, clean actuator_gateway
- [x] v2-03-02-PLAN.md — Delete device/ directory, remove CMake device library, relocate driver files to BSP, clean tests
- [x] v2-03-03-PLAN.md — Extract wait_ready startup gates into standalone functions

**Success Criteria:**
1. All 29 device_layer files and 14 semantic wrapper vtable files are deleted from the repository
2. No source file imports or includes anything from `runtime/device/`
3. Control code to hardware indirection is 2 hops: control -> BSP port -> BSP implementation (verified by call-chain audit)
4. `wait_ready` startup gates extracted from topic wrappers into explicit standalone functions before any deletion
5. Both HW ELF and SITL binary compile, link, and all CTest targets pass

### Phase 4: Consolidation

**Goal:** Runtime directory structure is flat, topic wrappers are gone, CMake is clean — the codebase matches the target 4-layer architecture.

**Dependencies:** Phase 3 (device layer gone, no stale references)

**Requirements:** SLIM-03, SLIM-05, KEEP-02, QUAL-02, QUAL-03

**Plans:** 3 plans

Plans:
- [x] v2-04-01-PLAN.md — Consolidate topic wrappers: inline PubRegister/SubGetMessage into task code, create single topics.h, delete 8 wrapper files
- [x] v2-04-02-PLAN.md — Relocate runtime/generated/ into bsp/boards/, verify 4-layer directory structure
- [x] v2-04-03-PLAN.md — CMake interface library cleanup + full validate pipeline end-to-end verification

**Success Criteria:**
1. 10 topic wrapper files replaced by a single `topics.h` with direct `PubRegister`/`SubGetMessage` calls in task code
2. `runtime/generated/` moved into `bsp/boards/`, runtime directory nesting is max 2 levels deep
3. `CMakeLists.txt` uses CMake interface libraries to eliminate duplicated include paths and sanitizer config
4. `python3 -m robot_platform.tools.platform_cli.main validate` runs all 5 stages to completion (build_sitl -> host_tests -> python_tests -> smoke -> verify_phase3)
5. Runtime layer count is 4: bsp -> control -> app -> module

## Progress

| Phase | Name | Requirements | Status |
|-------|------|--------------|--------|
| 1 | Port Foundation | SLIM-04, KEEP-03 | Complete |
| 2 | Seam Migration | KEEP-04, KEEP-01 | Complete |
| 3 | Device Layer Removal | SLIM-01, SLIM-02, QUAL-01 | Complete |
| 4 | Consolidation | SLIM-03, SLIM-05, KEEP-02, QUAL-02, QUAL-03 | Complete |

## Coverage

| Requirement | Phase | Rationale |
|-------------|-------|-----------|
| SLIM-04 | 1 | Command unification is the safest first step — pure refactor |
| KEEP-03 | 1 | BSP port interfaces establish link-time backend selection pattern |
| KEEP-04 | 2 | Test seam migration from device_layer hooks to BSP port fakes |
| KEEP-01 | 2 | Safety oracles verified on new code path |
| SLIM-01 | 3 | Device layer deletion after all consumers migrated |
| SLIM-02 | 3 | Semantic wrapper vtables deleted with device layer |
| QUAL-01 | 3 | Indirection depth drops to 2 hops when device layer is gone |
| SLIM-03 | 4 | Topic wrapper consolidation into single topics.h |
| SLIM-05 | 4 | Directory flattening and layer count reduction to 4 |
| KEEP-02 | 4 | Validate pipeline verified end-to-end after all structural changes |
| QUAL-02 | 4 | Directory nesting reduced after flattening |
| QUAL-03 | 4 | CMake cleanup in final consolidation pass |

**Mapped:** 12/12
**Orphaned:** 0

---
*Roadmap created: 2026-04-01*
*Migration order derived from: .planning/research/SUMMARY.md*
