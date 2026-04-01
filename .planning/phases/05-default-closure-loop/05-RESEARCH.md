# Phase 5: Default Closure Loop - Research

**Researched:** 2026-04-01
**Domain:** Python CLI pipeline orchestration, machine-readable stage reporting, firmware generation gating
**Confidence:** HIGH

## Summary

Phase 5 needs to compose existing CLI stages (build, host tests, SITL smoke, firmware generation) into a single sequential command path that stops on failure and produces a machine-readable closure artifact proving all gates passed before firmware output is considered usable.

The codebase already has all the individual stages implemented and working. The CLI at `robot_platform/tools/platform_cli/main.py` already has `build sitl`, `build hw_elf`, host test runners (`_run_host_ctest`, `_run_host_message_center_tests`), SITL smoke (`_run_sim`), code generation (`_generate_balance_chassis`), freshness gating (`_require_generated_artifact_freshness`), and three `verify phaseN` commands that produce JSON reports with stage-level pass/fail tracking. The `_run_stage` helper already wraps any callable into a timed stage result dict with `name`, `status`, `exit_code`, and `duration_s`.

The primary work is: (1) a new CLI command (e.g., `validate` or `closure`) that sequences these stages with early-exit, (2) a final JSON closure artifact that records each stage's outcome and timestamps, and (3) ensuring the command path refuses to produce firmware output when earlier gates fail.

**Primary recommendation:** Add a `validate` command to the existing CLI that sequences build_sitl -> host_tests -> smoke -> verify_phase3 -> build_hw_elf/generate, writing a single `build/closure_reports/closure_balance_chassis.json` artifact. Reuse `_run_stage` for each step. Stop at the first failure. Only attempt firmware-trusting stages after all validation gates pass.

## Standard Stack

### Core

| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| Python stdlib (`json`, `subprocess`, `time`, `pathlib`, `sys`) | 3.x | CLI orchestration, JSON reports, process spawning | Already used throughout `main.py`; no external deps needed |
| `unittest` | stdlib | CLI test coverage | Already used in `test_main.py` |
| CMake 3.22+ | 3.22 | Build system for C targets | Already configured in `CMakeLists.txt` |

### Supporting

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `robot_platform.tools.cubemx_backend.main` | internal | Freshness manifest, code generation | For the generate/firmware gate stage |
| `robot_platform.sim.projects` | internal | Profile lookup, smoke runner | For the SITL smoke stage |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Custom stage sequencer in `main.py` | External task runner (invoke, make, etc.) | External tool adds a dependency; the existing `_run_stage` pattern is sufficient and consistent with the codebase |
| Single `validate` command | Shell script chaining existing commands | Loses machine-readable artifact production and cross-platform consistency |

**Installation:**
No new dependencies required. Everything is already in the codebase.

## Architecture Patterns

### Recommended Approach: Extend Existing CLI

The closure loop should be a new command in `main.py` that reuses existing internal functions. This follows the established pattern where `verify phase1/2/3` already compose multiple stages with early-exit and JSON reporting.

### Pattern 1: Sequential Stage Pipeline with Early-Exit

**What:** A linear sequence of stages where each stage must pass before the next runs. On failure, the pipeline writes a partial closure report identifying the failing stage and exits non-zero.

**When to use:** This is the only pattern needed for Phase 5.

**Example (derived from existing `_run_verify_phase1` at main.py:433-529):**
```python
def _run_validate(project: str, report_path: Path) -> int:
    stages: list[dict[str, object]] = []

    # Stage 1: build SITL
    stage = _run_stage("build_sitl", lambda: _build_sitl(target))
    stages.append(stage)
    if stage["exit_code"] != 0:
        return _write_closure_report(report_path, stages, "build_sitl")

    # Stage 2: host tests
    stage = _run_stage("host_tests", lambda: _run_host_ctest(...))
    stages.append(stage)
    if stage["exit_code"] != 0:
        return _write_closure_report(report_path, stages, "host_tests")

    # ... continue for each stage
```

### Pattern 2: Closure Artifact as Gate Evidence

**What:** The final JSON artifact records every stage's outcome, timing, and the overall verdict. Downstream consumers (humans, CI, future phases) can inspect this artifact to determine whether the firmware output is trustworthy.

**When to use:** Always produced by the `validate` command, whether it passes or fails.

**Example (derived from existing verification report structure at main.py:511-516):**
```python
payload = {
    "closure_version": 1,
    "project": project,
    "overall_status": "passed" | "failed",
    "failure_stage": None | "stage_name",
    "failure_reason": None | "reason_string",
    "stages": stages,  # list of _run_stage dicts
    "timestamp": "ISO-8601",
}
```

### Pattern 3: Firmware Gate After Validation

**What:** The `generate` or `build hw_elf` step only runs after all validation stages pass. This is the key safety property of the closure loop.

**When to use:** Always. The closure command must not produce firmware output when earlier gates fail.

### Recommended Stage Sequence

```
validate balance_chassis
  |
  +-- 1. build_sitl          (compile SITL binary)
  +-- 2. host_tests           (run all host C tests via ctest)
  +-- 3. python_tests         (run Python CLI + sim tests)
  +-- 4. smoke                (run SITL smoke session)
  +-- 5. verify_phase3        (machine-readable runtime proof)
  +-- 6. build_hw_elf         (cross-compile firmware — optional/gated)
  |
  +-- Write closure_balance_chassis.json
```

### Anti-Patterns to Avoid

- **Running all stages regardless of failure:** The whole point is early-exit. Never continue past a failing stage.
- **Producing firmware output before validation:** The closure artifact must prove validation passed before firmware is considered usable.
- **Creating a parallel report path:** Reuse the existing `_run_stage` and JSON report patterns. Don't invent a new reporting format.
- **Fixing the hw_elf toolchain issue in this phase:** The known blocker (host `cc` rejecting Cortex-M flags) is a pre-existing issue. Phase 5 should work around it by making the hw_elf stage optional or skippable, not by fixing the cross-compiler resolution.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Stage timing and status tracking | Custom timer/status logic | `_run_stage()` at main.py:396-405 | Already handles timing, exit code, and status dict construction |
| JSON report writing | Manual file I/O | `write_report()` from `robot_platform/sim/reports/report_writer.py` or inline pattern from verify commands | Already handles mkdir, JSON serialization, encoding |
| Smoke session orchestration | Custom SITL launcher | `_run_sim()` at main.py:883-921 | Already handles profile lookup, build skip, and smoke runner dispatch |
| Host test execution | Custom test runner | `_run_host_ctest()` at main.py:337-379 | Already handles cmake configure, build, and ctest execution |
| Freshness gating | Custom hash checking | `_require_generated_artifact_freshness()` at main.py:219-225 and `generated_artifacts_are_fresh()` from cubemx_backend | Already handles manifest loading, hash comparison, and machine-readable refusal |

## Common Pitfalls

### Pitfall 1: Hardware Build Toolchain Blocker

**What goes wrong:** `build hw_elf` currently fails because the CMake configure step for the hardware target resolves to host `cc` instead of `arm-none-eabi-gcc`, causing Cortex-M flags (`-mthumb`, `-mfpu`, `-mfloat-abi`) to be rejected.

**Why it happens:** The `_build_hw_seed` function at main.py:228-256 passes the ARM toolchain file, but the CMake configure may still pick up the host compiler depending on environment state. The STATE.md explicitly notes this as a pre-existing blocker.

**How to avoid:** Make the hw_elf/firmware stage optional in the closure loop. The closure command should still pass if all validation stages pass, even if the firmware build is skipped or fails due to toolchain issues. The closure artifact should record whether the firmware stage was attempted and its outcome.

**Warning signs:** `_build_hw_seed` returns non-zero; CMake output shows `cc` instead of `arm-none-eabi-gcc`.

### Pitfall 2: Freshness Manifest Missing Before HW Build

**What goes wrong:** `build hw_elf` and `build hw_seed` are gated by `_require_generated_artifact_freshness()`. If `generate` hasn't been run, the freshness manifest doesn't exist and the build refuses to proceed.

**Why it happens:** The freshness manifest at `runtime/generated/stm32h7_ctrl_board_raw/freshness_manifest.json` is only written by `run_codegen()` after a successful CubeMX generation. Currently no manifest exists in the repo.

**How to avoid:** If the closure loop includes a firmware stage, it must either run `generate` first or skip the firmware stage when freshness metadata is missing. The closure artifact should record this as a known skip reason.

### Pitfall 3: SITL Smoke Requires UDP Sockets

**What goes wrong:** The SITL bridge uses UDP sockets. In sandboxed environments (some CI, restricted containers), UDP socket creation fails with `[Errno 1] Operation not permitted`.

**Why it happens:** The bridge opens UDP ports for IMU, remote, and motor communication. The existing `_smoke_stage_status` at main.py:408-430 already detects this and reports `blocked` status.

**How to avoid:** The closure command should handle `blocked` smoke status the same way `verify phase1` does — report it in the closure artifact and fail the pipeline. The existing detection logic is sufficient.

### Pitfall 4: Redundant SITL Builds

**What goes wrong:** Multiple stages (build_sitl, smoke, verify_phase3) each trigger their own SITL build. This wastes time.

**Why it happens:** `_run_sim` builds SITL by default unless `skip_build=True`. `_run_verify_phase3` also calls `_build_sitl` internally.

**How to avoid:** The closure command should build SITL once at the start, then pass `skip_build=True` to subsequent stages. The existing `--skip-build` flag in `_run_sim` supports this pattern.

### Pitfall 5: Test Scope Confusion

**What goes wrong:** The existing `test sim` command runs both `test_runner` and `test_main` Python tests. The existing `verify phase2` runs host C tests. The closure loop needs to run both without double-counting.

**Why it happens:** Host C tests and Python tests are separate test suites with different runners (ctest vs unittest).

**How to avoid:** The closure loop should have distinct stages for host C tests and Python tests, each with clear scope. Use `_run_host_ctest` for C tests and a Python unittest runner for Python tests.

## Code Examples

### Existing `_run_stage` Pattern (main.py:396-405)
```python
def _run_stage(stage_name: str, runner) -> dict[str, object]:
    started = time.monotonic()
    rc = runner()
    duration_s = round(time.monotonic() - started, 3)
    return {
        "name": stage_name,
        "status": "passed" if rc == 0 else "failed",
        "exit_code": rc,
        "duration_s": duration_s,
    }
```

### Existing Early-Exit Pattern (main.py:443-458)
```python
build_stage = _run_stage("build_sitl", lambda: _build_sitl(profile.sitl_target))
stages.append(build_stage)
if build_stage["exit_code"] != 0:
    payload = {
        "verification_run_version": 1,
        "project": project,
        "overall_status": "failed",
        "failure_stage": "build_sitl",
        "failure_reason": "build_failed",
        "stages": stages,
    }
    resolved_report_path.parent.mkdir(parents=True, exist_ok=True)
    resolved_report_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    return 1
```

### Existing Freshness Gate Pattern (main.py:219-225)
```python
def _require_generated_artifact_freshness() -> int:
    source_ioc, generated_dir = _generated_artifact_paths()
    is_fresh, reason, _manifest = generated_artifacts_are_fresh(source_ioc, generated_dir)
    if is_fresh:
        return 0
    _print_freshness_refusal(reason, source_ioc, generated_dir)
    return 1
```

### Existing Verification Report Structure (build/verification_reports/phase3_balance_chassis.json)
```json
{
  "verification_run_version": 1,
  "phase": "phase3",
  "project": "balance_chassis",
  "overall_status": "passed",
  "authoritative_bringup": { ... },
  "cases": [ ... ],
  "stages": [
    {
      "name": "smoke",
      "status": "passed",
      "exit_code": 0,
      "duration_s": 1.041,
      "smoke_report": "/path/to/sitl_smoke.json",
      "runtime_output_observation_count": 421,
      "adapter_binding_summary": { ... }
    }
  ]
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Manual multi-command workflow | Individual CLI commands (`build`, `sim`, `verify`) | Phase 1-4 | Developer must remember and sequence commands manually |
| No machine-readable stage evidence | `verify phase1/2/3` produce JSON reports | Phase 1-3 | Each phase has its own verification artifact, but no unified closure artifact |
| No freshness gating | `_require_generated_artifact_freshness` gates hw builds | Phase 1 | Hardware builds refuse to proceed with stale generated code |

**Current gap:** No single command composes all stages into one pipeline with a unified closure artifact.

## Existing CLI Command Map

| Command | Function | Returns | Machine-Readable Output |
|---------|----------|---------|------------------------|
| `generate` | `_generate_balance_chassis()` | int (exit code) | Freshness manifest JSON |
| `build sitl` | `_build_sitl("balance_chassis_sitl")` | int | None (stdout only) |
| `build hw_elf` | `_require_generated_artifact_freshness()` then `_build_hw_seed(...)` | int | Freshness refusal JSON on failure |
| `sim` | `_run_sim(...)` | int | `build/sim_reports/sitl_smoke.json` |
| `test sim` | `_run_tests("sim")` | int | None (stdout only) |
| `verify phase1` | `_run_verify_phase1(...)` | int | `build/verification_reports/phase1_balance_chassis.json` |
| `verify phase2` | `_run_verify_phase2(...)` | int | `build/verification_reports/phase2_balance_chassis.json` |
| `verify phase3` | `_run_verify_phase3(...)` | int | `build/verification_reports/phase3_balance_chassis.json` |

### Stages That Already Produce Machine-Readable Output (PIPE-02 coverage)

| Stage | Machine-Readable | Format | Gap |
|-------|-----------------|--------|-----|
| build_sitl | No | stdout only | Needs stage wrapper to capture exit code into closure artifact |
| host_tests (ctest) | No | stdout only | Needs stage wrapper |
| python_tests | No | stdout only | Needs stage wrapper |
| smoke (SITL) | Yes | `sitl_smoke.json` | Already sufficient |
| verify phase3 | Yes | `phase3_balance_chassis.json` | Already sufficient |
| generate | Partial | Freshness manifest | Needs stage wrapper for closure artifact |
| build hw_elf | Partial | Freshness refusal JSON on failure | Needs stage wrapper |

**Key insight for PIPE-02:** The closure artifact itself satisfies PIPE-02 by recording each stage's name, status, exit_code, and duration_s. Individual stages don't need to produce their own machine-readable output as long as the closure artifact captures the stage-level verdict.

## Hardware Build Toolchain Analysis

The STATE.md blocker says "Hardware build path still resolves to host `cc` (rejects Cortex-M flags)." However:

1. `arm-none-eabi-gcc` IS available on this system at `/usr/bin/arm-none-eabi-gcc`.
2. The ARM toolchain file at `cmake/toolchains/arm-none-eabi-gcc.cmake` uses `find_program(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)` which should find it.
3. The actual failure observed today is the freshness gate (`missing_metadata`), not the compiler resolution.
4. The real hw build may or may not work once freshness is satisfied — this is uncertain.

**Recommendation for Phase 5:** Treat the hw_elf stage as best-effort in the closure loop. If it fails, record the failure in the closure artifact but don't block the overall closure verdict if all validation stages passed. The closure artifact should distinguish between "validation passed, firmware build failed" and "validation failed."

## Open Questions

1. **Should `validate` include `generate` as a stage?**
   - What we know: `generate` requires STM32CubeMX and Java, which may not be available in all environments. The freshness manifest is missing, so `build hw_elf` will refuse without it.
   - What's unclear: Whether the closure loop should require CubeMX availability or treat firmware generation as optional.
   - Recommendation: Make `generate` optional. The core closure loop is about validation (build_sitl + host_tests + smoke + verify). Firmware generation is a separate concern that can be gated by freshness.

2. **Should the closure loop run `verify phase2` in addition to `verify phase3`?**
   - What we know: Phase 2 verification runs host safety tests (SAFE-01 through SAFE-06). Phase 3 verification runs SITL smoke with runtime binding proof. Both are independent.
   - What's unclear: Whether the closure loop should run both or just phase3 (which is the most comprehensive).
   - Recommendation: Run host C tests (which include the safety tests) as a stage, then run smoke/verify_phase3. This covers phase2's test surface without running the phase2 verification command separately.

3. **What should the closure report path be?**
   - Recommendation: `build/closure_reports/closure_balance_chassis.json` — follows the existing `build/verification_reports/` and `build/sim_reports/` pattern.

## Sources

### Primary (HIGH confidence)
- `robot_platform/tools/platform_cli/main.py` — full CLI source, all existing commands and patterns
- `robot_platform/tools/platform_cli/tests/test_main.py` — existing test coverage and mock patterns
- `robot_platform/sim/core/runner.py` — SITL session runner, report generation
- `robot_platform/tools/cubemx_backend/main.py` — freshness manifest, code generation
- `robot_platform/CMakeLists.txt` — build targets, test targets
- `build/verification_reports/phase3_balance_chassis.json` — existing report structure
- `.planning/STATE.md` — known blockers and project state

### Secondary (MEDIUM confidence)
- `.planning/phases/03-fake-link-runtime-proof/03-VERIFICATION.md` — Phase 3 verification details
- `.planning/phases/04-authoritative-platform-composition/04-VERIFICATION.md` — Phase 4 verification details

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all code is first-party Python, no external dependencies needed
- Architecture: HIGH — patterns are directly derived from existing `verify phase1/2/3` implementations in the same file
- Pitfalls: HIGH — all pitfalls are observed from actual codebase state and documented blockers

**Research date:** 2026-04-01
**Valid until:** 2026-05-01 (stable — no external dependency churn)
