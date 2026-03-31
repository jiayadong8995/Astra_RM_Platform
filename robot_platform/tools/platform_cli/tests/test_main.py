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
    _parse_verify_phase1_args,
    main,
    _run_verify_phase1,
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
