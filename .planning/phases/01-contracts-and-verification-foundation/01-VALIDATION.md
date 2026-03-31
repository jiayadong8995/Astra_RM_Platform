---
phase: 1
slug: contracts-and-verification-foundation
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-03-30
---

# Phase 1 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Python `unittest` + CMake/CTest host C tests |
| **Config file** | none — Wave 0 adds host test targets to `robot_platform/CMakeLists.txt` |
| **Quick run command** | `ctest --test-dir build/robot_platform_host_tests --output-on-failure` |
| **Full suite command** | `python3 -m robot_platform.tools.platform_cli.main test sim` plus `ctest --test-dir build/robot_platform_host_tests --output-on-failure` |
| **Estimated runtime** | ~15 seconds |

---

## Sampling Rate

- **After every task commit:** Run `ctest --test-dir build/robot_platform_host_tests --output-on-failure`
- **After every plan wave:** Run `python3 -m robot_platform.tools.platform_cli.main test sim` and `ctest --test-dir build/robot_platform_host_tests --output-on-failure`
- **Before `$gsd-verify-work`:** Full suite must be green
- **Max feedback latency:** 20 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|-----------|-------------------|-------------|--------|
| 1-01-01 | 01 | 0 | HOST-01 | unit | `ctest --test-dir build/robot_platform_host_tests --output-on-failure` | ❌ W0 | ⬜ pending |
| 1-01-02 | 01 | 0 | HOST-04 | unit | `ctest --test-dir build/robot_platform_host_tests --output-on-failure` | ❌ W0 | ⬜ pending |
| 1-02-01 | 02 | 1 | ARCH-02 | unit | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R message_center` | ❌ W0 | ⬜ pending |
| 1-03-01 | 03 | 1 | PIPE-02 | integration | `python3 -m robot_platform.tools.platform_cli.main test sim` | ✅ | ⬜ pending |
| 1-03-02 | 03 | 1 | PIPE-02 | integration | `python3 -m robot_platform.tools.platform_cli.main sim --duration 1` | ✅ | ⬜ pending |
| 1-04-01 | 04 | 2 | PIPE-03 | integration | `python3 -m robot_platform.tools.platform_cli.main generate` | ✅ | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `robot_platform/runtime/tests/host/test_message_center.c` — first-wave host test for transport sizing and registration safety
- [ ] `robot_platform/runtime/tests/host/test_actuator_gateway.c` — first-wave host test for contract-to-command execution seam
- [ ] `robot_platform/runtime/tests/host/test_support/` — thin stub support only for required device seams
- [ ] `robot_platform/CMakeLists.txt` — host test target(s), CTest registration, sanitizer flags
- [ ] `build/robot_platform_host_tests/` — dedicated configure/build directory for host test execution

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| UDP bridge can open sockets in the intended runtime environment | PIPE-02 | Current sandbox blocks local UDP socket init with `Operation not permitted`, so one live proof must run outside this restricted environment | Run `python3 -m robot_platform.tools.platform_cli.main sim --duration 1` in the intended dev environment and confirm bridge startup proceeds beyond socket init |
| Minimum live path proves one real observed runtime output | PIPE-02 | Requires environment where SITL + bridge can run fully and emit a passed smoke artifact | Execute the chosen minimum live path and inspect the generated smoke/verification JSON for at least one observed runtime output target |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 20s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
