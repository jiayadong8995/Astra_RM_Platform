---
phase: 04-authoritative-platform-composition
plan: "03"
subsystem: runtime
tags: [docs, verification, cli, bringup, balance-chassis]
requires:
  - phase: 04-authoritative-platform-composition
    provides: "Shared balance_chassis app-startup seam and control-owned runtime chain from plans 04-01 and 04-02"
provides:
  - "Machine-readable authoritative bring-up metadata in verify phase3 output"
  - "Single authoritative balance_chassis bring-up document with legacy startup demotion"
  - "Aligned runtime and project ownership docs for app versus control responsibilities"
affects: [phase5, onboarding, runtime-docs, phase3-verification]
tech-stack:
  added: []
  patterns: [json-first bringup metadata, authoritative bringup doc, app-control ownership messaging]
key-files:
  created:
    - robot_platform/docs/balance_chassis_bringup.md
  modified:
    - robot_platform/tools/platform_cli/main.py
    - robot_platform/tools/platform_cli/tests/test_main.py
    - robot_platform/runtime/README.md
    - robot_platform/runtime/control/README.md
    - robot_platform/projects/balance_chassis/README.md
key-decisions:
  - "Reused the existing verify phase3 artifact as the authoritative machine-readable surface for bring-up metadata instead of creating a Phase 4-only report."
  - "Published one dedicated bring-up document and kept the other runtime/project READMEs as short ownership summaries that point back to it."
  - "Described freertos_app.c explicitly as a legacy compatibility surface so the docs match the codebase's authoritative startup seam."
patterns-established:
  - "Authoritative startup truth is published in both JSON verification output and a linked human-readable doc using the same path strings."
  - "Runtime and project READMEs stay short and defer detailed bring-up authority statements to one dedicated reference page."
requirements-completed: [OBS-03, ARCH-03, ARCH-04]
duration: 4min
completed: 2026-04-01
---

# Phase 04 Plan 03: Authoritative Platform Composition Summary

**Phase 3 verification now emits blessed balance_chassis bring-up metadata, and one authoritative doc names the current hardware/SITL startup path while demoting the legacy startup surface**

## Performance

- **Duration:** 4 min
- **Started:** 2026-04-01T12:17:00Z
- **Completed:** 2026-04-01T12:20:50Z
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments

- Added `authoritative_bringup` to `verify phase3` so the blessed hardware path, SITL path, shared startup API, and demoted legacy surface are machine-checkable.
- Created `robot_platform/docs/balance_chassis_bringup.md` as the single authoritative bring-up reference for developers.
- Aligned runtime and project docs around the same `app` versus `control` ownership split and the `balance_chassis` proving-path message.

## Task Commits

Each task was committed atomically:

1. **Task 1: Persist blessed-path and legacy-path metadata in the existing verification surface** - `4e88d4b9` (`feat`)
2. **Task 2: Publish one authoritative bring-up document and align runtime/project ownership docs** - `e59195f9` (`feat`)

## Files Created/Modified

- `robot_platform/tools/platform_cli/main.py` - Adds authoritative bring-up metadata to the existing Phase 3 verification artifact.
- `robot_platform/tools/platform_cli/tests/test_main.py` - Locks the authoritative bring-up JSON schema and exact strings with unit tests.
- `robot_platform/docs/balance_chassis_bringup.md` - Provides the single human-readable source of truth for blessed startup paths and legacy demotion.
- `robot_platform/runtime/README.md` - Points runtime readers at the authoritative bring-up doc and repeats the short ownership split.
- `robot_platform/runtime/control/README.md` - Clarifies that startup wiring stays with `app` while the observe -> control -> execution chain stays with `control`.
- `robot_platform/projects/balance_chassis/README.md` - States that `balance_chassis` is the proving path for the reusable platform and links to the bring-up doc.

## Decisions Made

- Reused `verify phase3` as the only machine-readable bring-up proof surface so OBS-03 stays attached to the existing trusted verification command.
- Kept detailed bring-up authority wording in one dedicated doc and limited the surrounding READMEs to short linked summaries to avoid parallel truth.
- Named `runtime/app/balance_chassis/app_bringup/freertos_app.c` directly as a compatibility-only legacy surface so developers do not have to infer its status.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Phase 5 can build on one explicit bring-up statement in both verification artifacts and docs instead of inferring authority from file layout.
- The existing `verify phase3 --case runtime_binding` gate remains the authoritative command for machine-readable startup-path truth.

## Known Stubs

 - `robot_platform/tools/platform_cli/main.py:1005` still contains the pre-existing `command placeholder` fallback for unimplemented CLI commands. It does not block the authoritative bring-up proof path delivered in this plan.

## Self-Check: PASSED

- `FOUND:.planning/phases/04-authoritative-platform-composition/04-03-SUMMARY.md`
- `FOUND:4e88d4b9`
- `FOUND:e59195f9`

---
*Phase: 04-authoritative-platform-composition*
*Completed: 2026-04-01*
