---
phase: 01-contracts-and-verification-foundation
plan: "05"
subsystem: infra
tags: [stm32cubemx, freshness, build-gate, cli, verification]
requires:
  - phase: 01-04
    provides: Minimum live SITL proof and JSON-first phase verification surface
provides:
  - Deterministic freshness manifests for generated STM32 artifacts
  - Hard CLI refusal for stale or missing generated-artifact metadata on hardware builds
  - Regression tests for missing_metadata and stale_generated_artifacts failures
affects: [phase-1-verification, hardware-builds, firmware-trust-boundary]
tech-stack:
  added: []
  patterns: [Deterministic source-and-output hashing for generated artifacts, JSON refusal payloads at hardware trust boundaries]
key-files:
  created: [.planning/phases/01-contracts-and-verification-foundation/01-05-SUMMARY.md]
  modified:
    - robot_platform/tools/cubemx_backend/main.py
    - robot_platform/tools/platform_cli/main.py
    - robot_platform/tools/platform_cli/tests/test_main.py
key-decisions:
  - "Freshness metadata is recorded immediately after successful CubeMX generation as a deterministic manifest of IOC and generated tree hashes."
  - "Only hardware-trusting build modes are gated by freshness refusal so the 01-04 minimum live proof path remains the shortest evidence path."
patterns-established:
  - "Generated artifact trust uses content hashes instead of ad hoc timestamps."
  - "Hardware build refusal emits machine-readable JSON with a stable stage and reason."
requirements-completed: [PIPE-03]
duration: 2min
completed: 2026-03-31
---

# Phase 1 Plan 05: Generated Artifact Freshness Summary

**Deterministic STM32 generated-artifact manifests with hard JSON refusal on stale or missing hardware-build inputs**

## Performance

- **Duration:** 2 min
- **Started:** 2026-03-31T14:34:25+08:00
- **Completed:** 2026-03-31T14:36:01+08:00
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments
- Added TDD regression coverage for deterministic freshness manifests and exact hardware refusal reasons.
- Recorded source and generated-tree hashes in `freshness_manifest.json` after successful CubeMX generation.
- Blocked `build hw_elf` and `build hw_seed` with machine-readable `generated_artifact_freshness` failures when metadata is missing or stale.

## Task Commits

Each task was committed atomically:

1. **Task 1: Define deterministic generated-artifact freshness metadata and failing CLI tests** - `113724d9` (test)
2. **Task 2: Implement freshness metadata and hard refusal in hardware-trusting commands** - `31d94690` (feat)

## Files Created/Modified
- `robot_platform/tools/cubemx_backend/main.py` - Computes deterministic source and generated tree hashes, writes and loads freshness manifests, and validates freshness.
- `robot_platform/tools/platform_cli/main.py` - Enforces the hardware-build freshness gate and emits JSON refusal payloads.
- `robot_platform/tools/platform_cli/tests/test_main.py` - Covers manifest creation plus `missing_metadata` and `stale_generated_artifacts` refusal behavior.

## Decisions Made
- Freshness is based on file-content hashing for the IOC source and generated output tree, excluding the manifest itself.
- The new refusal gate applies only to hardware-trusting build modes so 01-04’s shorter minimum live proof remains unchanged.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Test generation initially mocked `run_codegen` too high in the stack and bypassed manifest writing; the implementation-side test was adjusted to mock the CubeMX subprocess instead so the real manifest write path stays exercised.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Phase 1 now has a deterministic trust boundary for STM32 generated artifacts and satisfies `PIPE-03`.
- The minimum live SITL proof path from 01-04 remains unblocked by this hardening work.

## Self-Check: PASSED

- Found summary file `.planning/phases/01-contracts-and-verification-foundation/01-05-SUMMARY.md`.
- Found task commit `113724d9`.
- Found task commit `31d94690`.

---
*Phase: 01-contracts-and-verification-foundation*
*Completed: 2026-03-31*
