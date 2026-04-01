---
phase: 05-default-closure-loop
plan: 01
subsystem: cli-pipeline
tags: [python, cli, closure-loop, validation-pipeline, json-artifact]
dependency_graph:
  requires: [phase-01, phase-02, phase-03, phase-04]
  provides: [validate-command, closure-artifact, sequential-stage-pipeline]
  affects: [05-02]
tech_stack:
  added: []
  patterns: [sequential-stage-pipeline-with-early-exit, closure-artifact-as-gate-evidence]
key_files:
  created: []
  modified:
    - robot_platform/tools/platform_cli/main.py
    - robot_platform/tools/platform_cli/tests/test_main.py
decisions:
  - Reuse _run_stage for all pipeline stages instead of creating a new wrapper
  - Smoke stage in validate is separate from verify_phase3 internal smoke (intentional duplication)
  - hw_elf is best-effort after validation gates; freshness gate failure results in skipped status
  - Closure artifact uses closure_version=1 schema distinct from verification_run_version
metrics:
  duration: 3min
  completed: 2026-04-01
---

# Phase 5 Plan 1: Validate CLI Command with Sequential Stage Pipeline Summary

Sequential closure pipeline composing build_sitl, host_tests, python_tests, smoke, and verify_phase3 into one `validate` command with early-exit and machine-readable closure JSON artifact at `build/closure_reports/closure_balance_chassis.json`.

## What Was Built

- `_parse_validate_args`: Accepts `--project` and `--report` options following existing arg-parsing patterns.
- `_write_closure_report`: Writes closure JSON with `closure_version`, `project`, `overall_status`, `failure_stage`, `failure_reason`, `hw_elf_attempted`, `hw_elf_status`, `stages`, and `timestamp`.
- `_run_validate`: Sequences 5 validation stages with early-exit, then attempts hw_elf as best-effort. Uses `_run_stage` for all stages.
- Command routing and HELP dict updated to include `validate`.

## Test Coverage

- `ParseValidateArgsTests`: 3 tests (defaults, explicit values, unknown option rejection)
- `ValidateTests`: 6 tests (early-exit at build_sitl, host_tests, python_tests, smoke; full-pass artifact structure; hw_elf failure preserves passed status)
- All 39 tests pass (33 existing + 9 new).

## Deviations from Plan

None - plan executed exactly as written.

## Commits

| Task | Commit | Description |
|------|--------|-------------|
| 1 | 7c150ba1 | feat: validate command with sequential stage pipeline and closure artifact |
| 2 | 162fa8a5 | test: unit tests for validate command |

## Next Phase Readiness

Plan 05-02 can proceed. The `validate` command is functional and tested. No blockers introduced.
