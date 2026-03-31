from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from robot_platform.tools.platform_cli.main import _parse_sim_args, _parse_verify_phase1_args, _run_verify_phase1


class ParseSimArgsTests(unittest.TestCase):
    def test_defaults(self) -> None:
        self.assertEqual(_parse_sim_args([]), ("balance_chassis", "sitl", 3.0, False))

    def test_duration_and_skip_build(self) -> None:
        self.assertEqual(
            _parse_sim_args(["--duration", "1.5", "--skip-build"]),
            ("balance_chassis", "sitl", 1.5, True),
        )

    def test_explicit_scenario(self) -> None:
        self.assertEqual(
            _parse_sim_args(["sitl", "--duration", "5"]),
            ("balance_chassis", "sitl", 5.0, False),
        )

    def test_explicit_project(self) -> None:
        self.assertEqual(
            _parse_sim_args(["--project", "balance_chassis", "--duration", "2"]),
            ("balance_chassis", "sitl", 2.0, False),
        )

    def test_rejects_unknown_option(self) -> None:
        with self.assertRaisesRegex(ValueError, "unknown sim option"):
            _parse_sim_args(["--unknown"])

    def test_rejects_invalid_duration(self) -> None:
        with self.assertRaisesRegex(ValueError, "invalid value for --duration"):
            _parse_sim_args(["--duration", "abc"])

    def test_rejects_non_positive_duration(self) -> None:
        with self.assertRaisesRegex(ValueError, "--duration must be positive"):
            _parse_sim_args(["--duration", "0"])


class ParseVerifyPhase1ArgsTests(unittest.TestCase):
    def test_defaults(self) -> None:
        project, report, smoke_duration = _parse_verify_phase1_args(["phase1"])
        self.assertEqual(project, "balance_chassis")
        self.assertEqual(report, Path("build/verification_reports/phase1_balance_chassis.json"))
        self.assertEqual(smoke_duration, 1.0)

    def test_explicit_values(self) -> None:
        project, report, smoke_duration = _parse_verify_phase1_args(
            ["phase1", "--project", "balance_chassis", "--report", "tmp/report.json", "--smoke-duration", "2.5"]
        )
        self.assertEqual(project, "balance_chassis")
        self.assertEqual(report, Path("tmp/report.json"))
        self.assertEqual(smoke_duration, 2.5)

    def test_rejects_unknown_verify_option(self) -> None:
        with self.assertRaisesRegex(ValueError, "unknown verify option"):
            _parse_verify_phase1_args(["phase1", "--unknown"])


class VerifyPhase1Tests(unittest.TestCase):
    def test_writes_blocked_report_for_udp_restriction(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            smoke_report = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
            smoke_report.parent.mkdir(parents=True, exist_ok=True)
            smoke_report.write_text(
                json.dumps(
                    {
                        "bridge_startup_error": {"message": "[Errno 1] Operation not permitted"},
                        "runtime_output_observations": [],
                        "smoke_result": {"passed": False, "primary_failure": "bridge_startup_error"},
                    }
                ),
                encoding="utf-8",
            )
            verification_report = repo_root / "build" / "verification_reports" / "phase1.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_message_center_tests", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=1),
            ):
                rc = _run_verify_phase1("balance_chassis", verification_report, 1.0)

            self.assertEqual(rc, 1)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "blocked")
            self.assertEqual(payload["failure_stage"], "smoke")
            self.assertIn("Operation not permitted", payload["failure_reason"])
            self.assertEqual(payload["stages"][-1]["status"], "blocked")
            self.assertEqual(payload["stages"][-1]["observed_outputs"], [])

    def test_writes_passed_report_when_all_stages_pass(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            smoke_report = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
            smoke_report.parent.mkdir(parents=True, exist_ok=True)
            smoke_report.write_text(
                json.dumps(
                    {
                        "runtime_output_observations": [{"topic": "actuator_command", "sample_count": 2}],
                        "smoke_result": {"passed": True},
                    }
                ),
                encoding="utf-8",
            )
            verification_report = repo_root / "build" / "verification_reports" / "phase1.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_message_center_tests", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=0),
            ):
                rc = _run_verify_phase1("balance_chassis", verification_report, 1.0)

            self.assertEqual(rc, 0)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "passed")
            self.assertEqual([stage["name"] for stage in payload["stages"]], ["build_sitl", "host_tests", "smoke"])
            self.assertEqual(payload["stages"][-1]["status"], "passed")
            self.assertEqual(
                payload["stages"][-1]["observed_outputs"],
                [{"topic": "actuator_command", "sample_count": 2}],
            )


if __name__ == "__main__":
    unittest.main()
