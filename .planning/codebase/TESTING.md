# Testing Patterns

**Analysis Date:** 2026-03-30

## Test Framework

**Runner:**
- Python `unittest` via the standard library.
- Config: no dedicated test config file detected; test entry is hard-coded in `robot_platform/tools/platform_cli/main.py`.

**Assertion Library:**
- `unittest.TestCase` assertions such as `assertEqual`, `assertTrue`, `assertFalse`, `assertIn`, `assertNotIn`, `assertIsInstance`, and `assertRaisesRegex` in `robot_platform/sim/tests/test_runner.py` and `robot_platform/tools/platform_cli/tests/test_main.py`.

**Run Commands:**
```bash
python3 -m robot_platform.tools.platform_cli.main test sim              # Run the supported regression entry
python3 -m unittest robot_platform.sim.tests.test_runner -v             # Run sim summary/parser tests directly
python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v # Run CLI argument parsing tests directly
```

## Test File Organization

**Location:**
- Tests live in dedicated `tests/` directories next to the Python subsystem they cover, not co-located with each source file.
- Active first-party tests detected:
  - `robot_platform/sim/tests/test_runner.py`
  - `robot_platform/tools/platform_cli/tests/test_main.py`

**Naming:**
- Use `test_*.py` filenames and `*Tests` class names, for example `RunnerSummaryTests` in `robot_platform/sim/tests/test_runner.py` and `ParseSimArgsTests` in `robot_platform/tools/platform_cli/tests/test_main.py`.

**Structure:**
```text
robot_platform/
├── sim/tests/test_runner.py
└── tools/platform_cli/tests/test_main.py
```

## Test Structure

**Suite Organization:**
```python
class ParseSimArgsTests(unittest.TestCase):
    def test_defaults(self) -> None:
        self.assertEqual(_parse_sim_args([]), ("balance_chassis", "sitl", 3.0, False))

    def test_rejects_unknown_option(self) -> None:
        with self.assertRaisesRegex(ValueError, "unknown sim option"):
            _parse_sim_args(["--unknown"])
```

**Patterns:**
- Group related cases by behavior area using one `unittest.TestCase` subclass per concern, such as `RunnerMetadataTests` versus `RunnerSummaryTests` in `robot_platform/sim/tests/test_runner.py`.
- Prefer direct function-level tests over full process integration when the logic is pure or summary-oriented, as in `robot_platform/sim/tests/test_runner.py`.
- Encode expected outputs inline as full dict literals for report-shaping functions, which makes schema drift visible in one assertion block.
- Each test module keeps a local `if __name__ == "__main__": unittest.main()` launcher for standalone execution.

## Mocking

**Framework:** None detected in active tests.

**Patterns:**
```python
summary = {
    "status": "bridge_exited_early",
    "elapsed_s": 0.1,
    "sitl_exit_code": -15,
    "bridge_startup_error": {"message": "[Errno 1] Operation not permitted"},
}

_build_smoke_result(summary)
self.assertEqual(summary["smoke_result"]["primary_failure"], "bridge_startup_error")
```

**What to Mock:**
- Prefer plain dictionaries, tuples, and small imported profile objects instead of runtime mocks. `robot_platform/sim/tests/test_runner.py` uses synthetic `summary` payloads and bridge log lines rather than patching subprocesses or sockets.

**What NOT to Mock:**
- Do not add mocks around deterministic parsing and summarization helpers in `robot_platform/sim/core/runner.py`; the current pattern tests them as pure functions.
- No precedent exists for mocking embedded C entrypoints, HAL calls, or FreeRTOS tasks in first-party automated tests.

## Fixtures and Factories

**Test Data:**
```python
metadata = _extract_bridge_metadata(
    [
        '[BridgeEvent] {"type":"protocol_version","payload":{"bridge_protocol_version":1}}',
        '[BridgeEvent] {"type":"transport_ports","payload":{"imu":9001,"motor_fb":9002,"motor_cmd":9003}}',
        "[Bridge] stats imu_sent=5 mit_seen=2 wheel_seen=1 fb_sent=2",
    ]
)
```

**Location:**
- No shared fixture or factory module is present.
- Test data is constructed inline inside each test in `robot_platform/sim/tests/test_runner.py` and `robot_platform/tools/platform_cli/tests/test_main.py`.

## Coverage

**Requirements:** None enforced by tooling or config.

**View Coverage:**
```bash
Not detected
```

Coverage signals currently available:
- The documented regression entry is `python3 -m robot_platform.tools.platform_cli.main test sim` in `robot_platform/sim/README.md` and `robot_platform/tools/platform_cli/README.md`.
- That command currently runs exactly two Python test modules from `_run_tests` in `robot_platform/tools/platform_cli/main.py`.
- I observed the command pass on 2026-03-30 with 18 tests total.
- No coverage reporter, thresholds, or badges were detected.

## Test Types

**Unit Tests:**
- Present for Python parsing/reporting logic in `robot_platform/sim/core/runner.py` via `robot_platform/sim/tests/test_runner.py`.
- Present for CLI argument parsing in `robot_platform/tools/platform_cli/main.py` via `robot_platform/tools/platform_cli/tests/test_main.py`.

**Integration Tests:**
- The `sim` command performs a smoke session and writes `build/sim_reports/sitl_smoke.json`, documented in `robot_platform/sim/README.md`, but there is no first-party automated test file that launches the smoke session end to end.
- Some test cases simulate integration-shaped payloads by constructing realistic bridge output and smoke summaries in `robot_platform/sim/tests/test_runner.py`.

**E2E Tests:**
- Not used as an automated framework in first-party code.
- Hardware firmware under `robot_platform/runtime/` and BSP code under `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/` have no detected automated test suite.

## Common Patterns

**Async Testing:**
```python
summary = {
    "status": "ok",
    "sitl_exit_code": -15,
    "bridge_stats_last": {"imu_sent": 10, "mit_seen": 0, "wheel_seen": 0, "fb_sent": 0},
    "sitl_output": ["Starting FreeRTOS POSIX Scheduler..."],
}

_summarize_smoke_health(summary, BALANCE_CHASSIS_PROFILE)
self.assertTrue(summary["smoke_health"]["passed"])
```
- There is no `asyncio` or threaded test harness. Async-like behavior is tested by summarizing recorded process output and exit codes rather than driving concurrent processes.

**Error Testing:**
```python
with self.assertRaisesRegex(ValueError, "--duration must be positive"):
    _parse_sim_args(["--duration", "0"])

self.assertTrue(_detect_runtime_error({"bridge_output": ["Traceback (most recent call last):"]}))
```
- Error-path tests are first-class and focus on boundary validation, malformed input, startup failure attribution, and degraded smoke health in `robot_platform/tools/platform_cli/tests/test_main.py` and `robot_platform/sim/tests/test_runner.py`.

## Gaps To Respect When Adding Tests

- If you add tests for Python tooling, place them under the nearest `tests/` directory and wire them into `_run_tests` in `robot_platform/tools/platform_cli/main.py` if they belong to the default regression path.
- Keep new tests deterministic and data-driven. The current suite avoids network, real subprocess orchestration, and timing-sensitive assertions.
- There is no existing harness for C unit tests, FreeRTOS task tests, or hardware abstraction tests under `robot_platform/runtime/`; adding coverage there currently requires introducing new build and execution infrastructure.

---

*Testing analysis: 2026-03-30*
