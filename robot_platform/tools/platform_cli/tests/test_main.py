from __future__ import annotations

import json
import io
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from robot_platform.tools.platform_cli.main import (
    _generate_balance_chassis,
    _parse_sim_args,
    _parse_validate_args,
    _parse_verify_phase1_args,
    _parse_verify_phase2_args,
    _parse_verify_phase3_args,
    main,
    _run_validate,
    _run_verify_phase1,
    _run_verify_phase2,
    _run_verify_phase3,
)


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


class ParseVerifyPhase2ArgsTests(unittest.TestCase):
    def test_defaults(self) -> None:
        project, report, case_name = _parse_verify_phase2_args(["phase2"])
        self.assertEqual(project, "balance_chassis")
        self.assertEqual(report, Path("build/verification_reports/phase2_balance_chassis.json"))
        self.assertIsNone(case_name)

    def test_explicit_values(self) -> None:
        project, report, case_name = _parse_verify_phase2_args(
            ["phase2", "--project", "balance_chassis", "--report", "tmp/report.json", "--case", "stale_command"]
        )
        self.assertEqual(project, "balance_chassis")
        self.assertEqual(report, Path("tmp/report.json"))
        self.assertEqual(case_name, "stale_command")

    def test_rejects_unknown_verify_option(self) -> None:
        with self.assertRaisesRegex(ValueError, "unknown verify option"):
            _parse_verify_phase2_args(["phase2", "--unknown"])


class ParseVerifyPhase3ArgsTests(unittest.TestCase):
    def test_defaults(self) -> None:
        project, report, case_name = _parse_verify_phase3_args(["phase3"])
        self.assertEqual(project, "balance_chassis")
        self.assertEqual(report, Path("build/verification_reports/phase3_balance_chassis.json"))
        self.assertIsNone(case_name)

    def test_explicit_case(self) -> None:
        project, report, case_name = _parse_verify_phase3_args(
            ["phase3", "--project", "balance_chassis", "--report", "tmp/report.json", "--case", "runtime_binding"]
        )
        self.assertEqual(project, "balance_chassis")
        self.assertEqual(report, Path("tmp/report.json"))
        self.assertEqual(case_name, "runtime_binding")

    def test_rejects_unknown_verify_option(self) -> None:
        with self.assertRaisesRegex(ValueError, "unknown verify option"):
            _parse_verify_phase3_args(["phase3", "--unknown"])


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
                        "smoke_result": {
                            "passed": True,
                            "validation": {"required_count": 1, "observed_count": 1},
                        },
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

    def test_fails_when_smoke_report_has_no_observed_runtime_output(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            smoke_report = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
            smoke_report.parent.mkdir(parents=True, exist_ok=True)
            smoke_report.write_text(
                json.dumps(
                    {
                        "runtime_output_observations": [],
                        "smoke_result": {
                            "passed": True,
                            "validation": {"required_count": 1, "observed_count": 0},
                        },
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

            self.assertEqual(rc, 1)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "failed")
            self.assertEqual(payload["failure_stage"], "smoke")
            self.assertEqual(payload["failure_reason"], "runtime_output_not_observed:0/1")


class VerifyPhase2Tests(unittest.TestCase):
    def test_writes_case_report_for_stale_command(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            verification_report = repo_root / "build" / "verification_reports" / "phase2_stale_command.json"

            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_ctest", return_value=0),
            ):
                rc = _run_verify_phase2("balance_chassis", verification_report, "stale_command")

            self.assertEqual(rc, 0)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "passed")
            self.assertEqual(payload["cases"], [{"name": "stale_command", "requirements": ["SAFE-05"], "status": "passed"}])
            self.assertEqual(payload["stages"][0]["name"], "host_tests")

    def test_writes_failed_report_when_host_case_fails(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            verification_report = repo_root / "build" / "verification_reports" / "phase2_failed.json"

            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_ctest", return_value=1),
            ):
                rc = _run_verify_phase2("balance_chassis", verification_report, "wheel_leg_danger")

            self.assertEqual(rc, 1)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "failed")
            self.assertEqual(payload["failure_stage"], "host_tests")
            self.assertEqual(payload["cases"][0]["reason"], "unsafe_wheel_leg_danger_signature")

    def test_full_run_includes_cli_tests_stage(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            verification_report = repo_root / "build" / "verification_reports" / "phase2_full.json"

            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_ctest", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_phase2_cli_tests", return_value=0),
            ):
                rc = _run_verify_phase2("balance_chassis", verification_report, None)

            self.assertEqual(rc, 0)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "passed")
            self.assertEqual([stage["name"] for stage in payload["stages"]], ["host_tests", "cli_tests"])
            self.assertEqual(len(payload["cases"]), 6)


class VerifyPhase3Tests(unittest.TestCase):
    def test_writes_phase3_case_matrix_with_machine_readable_status(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            smoke_report = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
            smoke_report.parent.mkdir(parents=True, exist_ok=True)
            smoke_report.write_text(
                json.dumps(
                    {
                        "adapter_bindings": [
                            {"name": "imu", "transport": "udp", "bound": True, "port": 9001},
                            {"name": "remote", "transport": "udp", "bound": True, "port": 9004},
                            {"name": "motor", "transport": "udp", "bound": True, "port": 9003},
                        ],
                        "adapter_binding_summary": {"bound_count": 3, "expected_count": 3, "all_bound": True},
                        "runtime_output_observations": [{"topic": "actuator_command", "sample_count": 2}],
                        "runtime_output_observation_count": 1,
                        "runtime_binding": {
                            "passed": True,
                            "chain": "remote input + state observation -> intent parsing / mode constraints -> chassis control -> execution output",
                        },
                        "smoke_result": {"passed": True},
                    }
                ),
                encoding="utf-8",
            )
            verification_report = repo_root / "build" / "verification_reports" / "phase3.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=0),
            ):
                rc = _run_verify_phase3("balance_chassis", verification_report, None)

            self.assertEqual(rc, 0)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "passed")
            self.assertEqual(payload["stages"][0]["name"], "smoke")
            self.assertEqual(
                [case["name"] for case in payload["cases"]],
                ["runtime_binding", "runtime_outputs", "artifact_schema", "classification", "contract_drift", "diagnostics"],
            )
            self.assertEqual(
                payload["cases"][0]["description"],
                "remote input + state observation -> intent parsing / mode constraints -> chassis control -> execution output",
            )
            self.assertEqual(payload["cases"][3]["status"], "passed")
            self.assertEqual(payload["cases"][4]["status"], "passed")
            self.assertEqual(payload["cases"][5]["status"], "passed")
            self.assertEqual(payload["authoritative_bringup"]["hardware_path"], "main.c -> MX_FREERTOS_Init() -> balance_chassis_app_startup() -> scheduler")
            self.assertEqual(payload["authoritative_bringup"]["sitl_path"], "main_sitl.c -> balance_chassis_app_startup() -> scheduler")
            self.assertEqual(payload["authoritative_bringup"]["shared_app_startup_api"], "balance_chassis_app_startup()")
            self.assertEqual(
                payload["authoritative_bringup"]["legacy_paths"],
                [
                    {
                        "path": "runtime/app/balance_chassis/app_bringup/freertos_app.c",
                        "status": "compatibility-only",
                        "reason": "removed as an authoritative startup owner in favor of balance_chassis_app_startup().",
                    }
                ],
            )

    def test_classification_case_requires_separate_safety_provenance(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            smoke_report = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
            smoke_report.parent.mkdir(parents=True, exist_ok=True)
            smoke_report.write_text(
                json.dumps(
                    {
                        "runtime_binding": {
                            "passed": True,
                            "chain": "remote input + state observation -> intent parsing / mode constraints -> chassis control -> execution output",
                        },
                        "runtime_output_observation_count": 1,
                        "adapter_binding_summary": {"bound_count": 3, "expected_count": 3, "all_bound": True},
                        "smoke_result": {
                            "passed": True,
                            "failure_layer": "safety_protection",
                            "safety_protection_diagnostics": {"reasons": ["unsafe_actuator_verdict"]},
                        },
                    }
                ),
                encoding="utf-8",
            )
            verification_report = repo_root / "build" / "verification_reports" / "phase3_classification.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=0),
            ):
                rc = _run_verify_phase3("balance_chassis", verification_report, "classification")

            self.assertEqual(rc, 0)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["cases"][0]["name"], "classification")
            self.assertEqual(payload["cases"][0]["status"], "passed")

    def test_contract_drift_case_fails_for_protocol_boundary_and_port_mismatch(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            smoke_report = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
            smoke_report.parent.mkdir(parents=True, exist_ok=True)
            smoke_report.write_text(
                json.dumps(
                    {
                        "runtime_binding": {
                            "passed": False,
                            "chain": "remote input + state observation -> intent parsing / mode constraints -> chassis control -> execution output",
                        },
                        "runtime_output_observation_count": 1,
                        "adapter_binding_summary": {"bound_count": 3, "expected_count": 3, "all_bound": True},
                        "smoke_result": {
                            "passed": False,
                            "failure_layer": "communication",
                            "communication_diagnostics": {
                                "reasons": ["protocol_mismatch", "runtime_boundary_mismatch", "transport_ports_mismatch"]
                            },
                        },
                    }
                ),
                encoding="utf-8",
            )
            verification_report = repo_root / "build" / "verification_reports" / "phase3_contract_drift.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=0),
            ):
                rc = _run_verify_phase3("balance_chassis", verification_report, "contract_drift")

            self.assertEqual(rc, 1)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "failed")
            self.assertEqual(payload["cases"][0]["name"], "contract_drift")
            self.assertEqual(payload["cases"][0]["reason"], "protocol_mismatch,runtime_boundary_mismatch,transport_ports_mismatch")

    def test_diagnostics_case_requires_diagnostics_payloads(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            smoke_report = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
            smoke_report.parent.mkdir(parents=True, exist_ok=True)
            smoke_report.write_text(
                json.dumps(
                    {
                        "adapter_bindings": [{"name": "imu", "transport": "udp", "bound": True, "port": 9001}],
                        "adapter_binding_summary": {"bound_count": 1, "expected_count": 3, "all_bound": False},
                        "runtime_output_observations": [],
                        "runtime_output_observation_count": 0,
                        "runtime_binding": {
                            "passed": False,
                            "chain": "remote input + state observation -> intent parsing / mode constraints -> chassis control -> execution output",
                        },
                        "smoke_result": {
                            "passed": False,
                            "failure_layer": "observation",
                            "communication_diagnostics": {"bridge_counters": {"imu_sent": 10}},
                            "observation_diagnostics": {"reasons": ["missing_runtime_observations"], "missing_runtime_output_count": 1},
                            "control_diagnostics": {"reasons": []},
                            "safety_protection_diagnostics": {"reasons": ["unsafe_actuator_verdict"]},
                        },
                    }
                ),
                encoding="utf-8",
            )
            verification_report = repo_root / "build" / "verification_reports" / "phase3_diagnostics.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=0),
            ):
                rc = _run_verify_phase3("balance_chassis", verification_report, "diagnostics")

            self.assertEqual(rc, 0)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["cases"][0]["name"], "diagnostics")
            self.assertEqual(payload["cases"][0]["status"], "passed")

    def test_runtime_outputs_case_fails_without_observed_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            smoke_report = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
            smoke_report.parent.mkdir(parents=True, exist_ok=True)
            smoke_report.write_text(
                json.dumps(
                    {
                        "adapter_bindings": [],
                        "adapter_binding_summary": {"bound_count": 0, "expected_count": 3, "all_bound": False},
                        "runtime_output_observations": [],
                        "runtime_output_observation_count": 0,
                        "runtime_binding": {"passed": False},
                        "smoke_result": {"passed": False},
                    }
                ),
                encoding="utf-8",
            )
            verification_report = repo_root / "build" / "verification_reports" / "phase3_runtime_outputs.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=0),
            ):
                rc = _run_verify_phase3("balance_chassis", verification_report, "runtime_outputs")

            self.assertEqual(rc, 1)
            payload = json.loads(verification_report.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "failed")
            self.assertEqual(len(payload["cases"]), 1)
            self.assertEqual(payload["cases"][0]["name"], "runtime_outputs")
            self.assertEqual(payload["cases"][0]["status"], "failed")
            self.assertEqual(payload["cases"][0]["reason"], "runtime_output_observation_count=0")


class ParseValidateArgsTests(unittest.TestCase):
    def test_defaults(self) -> None:
        project, report = _parse_validate_args([])
        self.assertEqual(project, "balance_chassis")
        self.assertEqual(report, Path("build/closure_reports/closure_balance_chassis.json"))

    def test_explicit_values(self) -> None:
        project, report = _parse_validate_args(
            ["--project", "balance_chassis", "--report", "tmp/closure.json"]
        )
        self.assertEqual(project, "balance_chassis")
        self.assertEqual(report, Path("tmp/closure.json"))

    def test_rejects_unknown_option(self) -> None:
        with self.assertRaisesRegex(ValueError, "unknown validate option"):
            _parse_validate_args(["--unknown"])


class ValidateTests(unittest.TestCase):
    def _make_smoke_report(self, repo_root: Path, passed: bool) -> None:
        smoke_path = repo_root / "build" / "sim_reports" / "sitl_smoke.json"
        smoke_path.parent.mkdir(parents=True, exist_ok=True)
        smoke_path.write_text(
            json.dumps(
                {
                    "runtime_output_observations": [{"topic": "actuator_command", "sample_count": 2}],
                    "runtime_output_observation_count": 1,
                    "smoke_result": {"passed": passed},
                }
            ),
            encoding="utf-8",
        )

    def test_early_exit_on_build_sitl_failure(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            report_path = repo_root / "build" / "closure_reports" / "closure.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=1),
            ):
                rc = _run_validate("balance_chassis", report_path)

            self.assertEqual(rc, 1)
            payload = json.loads(report_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "failed")
            self.assertEqual(payload["failure_stage"], "build_sitl")
            self.assertEqual(len(payload["stages"]), 1)
            self.assertEqual(payload["closure_version"], 1)
            self.assertIsInstance(payload["timestamp"], str)

    def test_early_exit_on_host_tests_failure(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            report_path = repo_root / "build" / "closure_reports" / "closure.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_ctest", return_value=1),
            ):
                rc = _run_validate("balance_chassis", report_path)

            self.assertEqual(rc, 1)
            payload = json.loads(report_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "failed")
            self.assertEqual(payload["failure_stage"], "host_tests")
            self.assertEqual(len(payload["stages"]), 2)
            self.assertEqual(payload["stages"][0]["status"], "passed")
            self.assertEqual(payload["stages"][1]["status"], "failed")

    def test_early_exit_on_python_tests_failure(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            report_path = repo_root / "build" / "closure_reports" / "closure.json"

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_ctest", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_tests", return_value=1),
            ):
                rc = _run_validate("balance_chassis", report_path)

            self.assertEqual(rc, 1)
            payload = json.loads(report_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "failed")
            self.assertEqual(payload["failure_stage"], "python_tests")
            self.assertEqual(len(payload["stages"]), 3)

    def test_early_exit_on_smoke_failure(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            report_path = repo_root / "build" / "closure_reports" / "closure.json"
            self._make_smoke_report(repo_root, passed=False)

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_ctest", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_tests", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=1),
            ):
                rc = _run_validate("balance_chassis", report_path)

            self.assertEqual(rc, 1)
            payload = json.loads(report_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "failed")
            self.assertEqual(payload["failure_stage"], "smoke")
            self.assertEqual(len(payload["stages"]), 4)

    def test_full_pass_writes_closure_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            report_path = repo_root / "build" / "closure_reports" / "closure.json"
            self._make_smoke_report(repo_root, passed=True)

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_ctest", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_tests", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_verify_phase3", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._require_generated_artifact_freshness", return_value=1),
            ):
                rc = _run_validate("balance_chassis", report_path)

            self.assertEqual(rc, 0)
            payload = json.loads(report_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "passed")
            self.assertIsNone(payload["failure_stage"])
            self.assertEqual(payload["hw_elf_status"], "skipped")
            self.assertEqual(len(payload["stages"]), 5)
            self.assertEqual(
                [s["name"] for s in payload["stages"]],
                ["build_sitl", "host_tests", "python_tests", "smoke", "verify_phase3"],
            )
            self.assertEqual(payload["closure_version"], 1)
            self.assertIsInstance(payload["timestamp"], str)

    def test_hw_elf_failure_does_not_change_overall_passed(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            report_path = repo_root / "build" / "closure_reports" / "closure.json"
            self._make_smoke_report(repo_root, passed=True)

            fake_profile = mock.Mock(report_name="sitl_smoke.json", sitl_target="balance_chassis_sitl")
            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.platform_cli.main.get_project_profile", return_value=fake_profile),
                mock.patch("robot_platform.tools.platform_cli.main._build_sitl", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_host_ctest", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_tests", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_sim", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._run_verify_phase3", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._require_generated_artifact_freshness", return_value=0),
                mock.patch("robot_platform.tools.platform_cli.main._build_hw_seed", return_value=1),
            ):
                rc = _run_validate("balance_chassis", report_path)

            self.assertEqual(rc, 0)
            payload = json.loads(report_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["overall_status"], "passed")
            self.assertEqual(payload["hw_elf_status"], "failed")
            self.assertEqual(len(payload["stages"]), 6)


class GeneratedArtifactFreshnessTests(unittest.TestCase):
    def test_generate_records_deterministic_freshness_metadata(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            source_ioc = (
                repo_root
                / "references"
                / "legacy"
                / "Astra_RM2025_Balance_legacy"
                / "Chassis"
                / "CtrlBoard-H7_IMU.ioc"
            )
            source_ioc.parent.mkdir(parents=True, exist_ok=True)
            source_ioc.write_text("ioc-version=1\n", encoding="utf-8")

            generated_dir = repo_root / "robot_platform" / "runtime" / "generated" / "stm32h7_ctrl_board_raw"
            generated_dir.mkdir(parents=True, exist_ok=True)
            (generated_dir / "Src").mkdir()
            (generated_dir / "Src" / "main.c").write_text("int main(void) { return 0; }\n", encoding="utf-8")

            with (
                mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
                mock.patch("robot_platform.tools.cubemx_backend.main.find_cubemx", return_value="/tmp/fake-cubemx"),
                mock.patch(
                    "robot_platform.tools.cubemx_backend.main.subprocess.run",
                    return_value=mock.Mock(returncode=0),
                ),
            ):
                rc = _generate_balance_chassis()

            self.assertEqual(rc, 0)
            manifest_path = generated_dir / "freshness_manifest.json"
            self.assertTrue(manifest_path.exists(), "expected freshness manifest to be written after generate")
            payload = json.loads(manifest_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["source"]["path"], str(source_ioc))
            self.assertEqual(payload["generated"]["path"], str(generated_dir))
            self.assertIn("sha256", payload["source"])
            self.assertIn("tree_sha256", payload["generated"])

    def test_build_hw_elf_refuses_when_freshness_metadata_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            source_ioc = (
                repo_root
                / "references"
                / "legacy"
                / "Astra_RM2025_Balance_legacy"
                / "Chassis"
                / "CtrlBoard-H7_IMU.ioc"
            )
            source_ioc.parent.mkdir(parents=True, exist_ok=True)
            source_ioc.write_text("ioc-version=1\n", encoding="utf-8")
            generated_dir = repo_root / "robot_platform" / "runtime" / "generated" / "stm32h7_ctrl_board_raw"
            generated_dir.mkdir(parents=True, exist_ok=True)

            rc, stdout = self._run_main_with_stdout(repo_root, ["platform_cli", "build", "hw_elf"])

            self.assertEqual(rc, 1)
            payload = json.loads(stdout.getvalue())
            self.assertEqual(payload["stage"], "generated_artifact_freshness")
            self.assertEqual(payload["reason"], "missing_metadata")
            self.assertEqual(payload["source_ioc"], str(source_ioc))

    def test_build_hw_seed_refuses_when_generated_artifacts_are_stale(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            source_ioc = (
                repo_root
                / "references"
                / "legacy"
                / "Astra_RM2025_Balance_legacy"
                / "Chassis"
                / "CtrlBoard-H7_IMU.ioc"
            )
            source_ioc.parent.mkdir(parents=True, exist_ok=True)
            source_ioc.write_text("ioc-version=2\n", encoding="utf-8")
            generated_dir = repo_root / "robot_platform" / "runtime" / "generated" / "stm32h7_ctrl_board_raw"
            generated_dir.mkdir(parents=True, exist_ok=True)
            (generated_dir / "freshness_manifest.json").write_text(
                json.dumps(
                    {
                        "source": {"path": str(source_ioc), "sha256": "outdated-source-sha"},
                        "generated": {"path": str(generated_dir), "tree_sha256": "outdated-tree-sha"},
                    }
                ),
                encoding="utf-8",
            )
            (generated_dir / "Src").mkdir()
            (generated_dir / "Src" / "main.c").write_text("int main(void) { return 1; }\n", encoding="utf-8")

            rc, stdout = self._run_main_with_stdout(repo_root, ["platform_cli", "build", "hw_seed"])

            self.assertEqual(rc, 1)
            payload = json.loads(stdout.getvalue())
            self.assertEqual(payload["stage"], "generated_artifact_freshness")
            self.assertEqual(payload["reason"], "stale_generated_artifacts")
            self.assertEqual(payload["source_ioc"], str(source_ioc))

    def _run_main_with_stdout(self, repo_root: Path, argv: list[str]) -> tuple[int, io.StringIO]:
        stdout = io.StringIO()
        with (
            mock.patch("robot_platform.tools.platform_cli.main._repo_root", return_value=repo_root),
            mock.patch("robot_platform.tools.platform_cli.main.sys.argv", argv),
            mock.patch("robot_platform.tools.platform_cli.main.sys.stdout", stdout),
            mock.patch("robot_platform.tools.platform_cli.main._build_hw_seed", return_value=0),
        ):
            return main(), stdout


if __name__ == "__main__":
    unittest.main()
