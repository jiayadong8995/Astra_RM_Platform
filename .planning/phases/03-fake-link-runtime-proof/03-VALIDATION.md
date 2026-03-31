---
phase: 03
slug: fake-link-runtime-proof
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-04-01
---

# Phase 03 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Python `unittest` + CTest host tests |
| **Config file** | none — direct module invocation and CMake/CTest |
| **Quick run command** | `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v` |
| **Full suite command** | `python3 -m robot_platform.tools.platform_cli.main verify phase3` |
| **Estimated runtime** | ~30 seconds |

---

## Sampling Rate

- **After every task commit:** Run `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v`
- **After every plan wave:** Run `python3 -m robot_platform.tools.platform_cli.main verify phase3`
- **Before `$gsd-verify-work`:** Full suite must be green
- **Max feedback latency:** 30 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|-----------|-------------------|-------------|--------|
| 03-01-01 | 01 | 1 | LINK-01 | host regression | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_device_profile_sitl_runtime_bindings` | ❌ W0 | ⬜ pending |
| 03-01-02 | 01 | 1 | LINK-01 | host regression | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_device_profile_sitl_runtime_bindings` | ❌ W0 | ⬜ pending |
| 03-02-01 | 02 | 2 | LINK-01, LINK-02, OBS-01 | unit | `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v` | ❌ W0 | ⬜ pending |
| 03-02-02 | 02 | 2 | LINK-01, LINK-02, OBS-01 | integration | `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v` and `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case runtime_binding` and `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case runtime_outputs` and `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case artifact_schema` | ❌ W0 | ⬜ pending |
| 03-03-01 | 03 | 3 | LINK-03, LINK-04, OBS-02 | unit | `python3 -m unittest robot_platform.sim.tests.test_runner robot_platform.tools.platform_cli.tests.test_main -v` | ❌ W0 | ⬜ pending |
| 03-03-02 | 03 | 3 | LINK-03, LINK-04, OBS-02 | integration | `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case classification` and `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case contract_drift` and `python3 -m robot_platform.tools.platform_cli.main verify phase3 --case diagnostics` | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `robot_platform/runtime/tests/host/test_device_profile_sitl_runtime_bindings.c` — create the new LINK-01 binding regression scaffold from 03-01-01
- [ ] `robot_platform/tools/platform_cli/tests/test_main.py` — add Phase 3 verification-report coverage for `runtime_binding`, `runtime_outputs`, `artifact_schema`, `classification`, `contract_drift`, and `diagnostics`
- [ ] `robot_platform/sim/tests/test_runner.py` — add artifact-schema, classification, and diagnostics summary coverage
- [ ] `robot_platform/tools/platform_cli/main.py` — add the `verify phase3` entrypoint and case matrix
- [ ] `robot_platform/runtime/device/device_profile_sitl.c` — expose runtime-backed adapter-binding truth for SITL IMU/remote ingress before runtime binding proof runs

---

## Manual-Only Verifications

All phase behaviors should have automated verification through `unittest`, CTest-backed flows, or `verify phase3`.

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 30s
- [ ] LINK-01 ownership is closed by 03-01 binding preparation plus 03-02 `runtime_binding` runtime proof
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
