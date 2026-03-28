from __future__ import annotations

import unittest

from robot_platform.sim.runner import (
    _build_smoke_result,
    _detect_runtime_error,
    _extract_bridge_metadata,
    _summarize_bridge_stats,
    _summarize_runtime_boundary,
    _summarize_validation_targets,
    _summarize_smoke_health,
)
from robot_platform.sim.projects.balance_chassis.profile import BALANCE_CHASSIS_PROFILE


class RunnerMetadataTests(unittest.TestCase):
    def test_extract_bridge_metadata_prefers_events_and_deduplicates_stats(self) -> None:
        metadata = _extract_bridge_metadata(
            [
                '[BridgeEvent] {"type":"protocol_version","payload":{"bridge_protocol_version":1}}',
                '[BridgeEvent] {"type":"runtime_boundary","payload":{"inputs":["ins_data","chassis_cmd"],"outputs":["chassis_state","leg_left","leg_right"],"transitional":["chassis_observe"]}}',
                '[BridgeEvent] {"type":"transport_ports","payload":{"imu":9001,"motor_fb":9002,"motor_cmd":9003}}',
                '[BridgeEvent] {"type":"stats","payload":{"imu_sent":1,"mit_seen":0,"wheel_seen":0,"fb_sent":0}}',
                "[Bridge] stats imu_sent=1 mit_seen=0 wheel_seen=0 fb_sent=0",
                '[BridgeEvent] {"type":"stats","payload":{"imu_sent":5,"mit_seen":2,"wheel_seen":1,"fb_sent":2}}',
                "[Bridge] stats imu_sent=5 mit_seen=2 wheel_seen=1 fb_sent=2",
            ]
        )

        self.assertEqual(metadata["bridge_protocol"], {"bridge_protocol_version": 1})
        self.assertEqual(
            metadata["runtime_boundary"],
            {
                "inputs": ["ins_data", "chassis_cmd"],
                "outputs": ["chassis_state", "leg_left", "leg_right"],
                "transitional": ["chassis_observe"],
            },
        )
        self.assertEqual(metadata["transport_ports"], {"imu": 9001, "motor_fb": 9002, "motor_cmd": 9003})
        self.assertEqual(
            metadata["bridge_stats_samples"],
            [
                {"imu_sent": 1, "mit_seen": 0, "wheel_seen": 0, "fb_sent": 0},
                {"imu_sent": 5, "mit_seen": 2, "wheel_seen": 1, "fb_sent": 2},
            ],
        )

    def test_detect_runtime_error_catches_bridge_and_sitl_tracebacks(self) -> None:
        self.assertTrue(_detect_runtime_error({"bridge_output": ["Traceback (most recent call last):"]}))
        self.assertTrue(_detect_runtime_error({"sitl_output": ["Traceback (most recent call last):"]}))
        self.assertFalse(_detect_runtime_error({"bridge_output": ["[Bridge] stats imu_sent=1"], "sitl_output": []}))


class RunnerSummaryTests(unittest.TestCase):
    def test_balance_chassis_profile_declares_runtime_boundaries(self) -> None:
        self.assertEqual(BALANCE_CHASSIS_PROFILE.name, "balance_chassis")
        self.assertEqual(BALANCE_CHASSIS_PROFILE.runtime_input_boundary.topics, ("ins_data", "chassis_cmd"))
        self.assertEqual(
            BALANCE_CHASSIS_PROFILE.runtime_output_boundary.topics,
            ("chassis_state", "leg_left", "leg_right"),
        )
        self.assertTrue(BALANCE_CHASSIS_PROFILE.smoke_expectations.require_bridge_stats_observed)
        self.assertEqual(
            tuple(target.name for target in BALANCE_CHASSIS_PROFILE.validation_targets),
            ("chassis_state_summary", "leg_output_pair"),
        )

    def test_summarize_bridge_stats_computes_delta_and_rate(self) -> None:
        summary = {
            "elapsed_s": 2.0,
            "bridge_stats_samples": [
                {"imu_sent": 1, "mit_seen": 0, "wheel_seen": 0, "fb_sent": 0},
                {"imu_sent": 9, "mit_seen": 4, "wheel_seen": 1, "fb_sent": 3},
            ],
        }

        _summarize_bridge_stats(summary)

        self.assertEqual(
            summary["bridge_stats_summary"],
            {
                "sample_count": 2,
                "first": {"imu_sent": 1, "mit_seen": 0, "wheel_seen": 0, "fb_sent": 0},
                "last": {"imu_sent": 9, "mit_seen": 4, "wheel_seen": 1, "fb_sent": 3},
                "delta": {"imu_sent": 8, "mit_seen": 4, "wheel_seen": 1, "fb_sent": 3},
                "rate_per_s": {"imu_sent": 4.0, "mit_seen": 2.0, "wheel_seen": 0.5, "fb_sent": 1.5},
            },
        )

    def test_smoke_result_reports_startup_error_as_primary_failure(self) -> None:
        summary = {
            "status": "bridge_exited_early",
            "elapsed_s": 0.1,
            "sitl_exit_code": -15,
            "bridge_startup_error": {"message": "[Errno 1] Operation not permitted"},
            "bridge_stats_last": {"imu_sent": 0, "mit_seen": 0, "wheel_seen": 0, "fb_sent": 0},
            "smoke_health": {"passed": False, "failures": ["session_status_ok"]},
        }

        _build_smoke_result(summary)

        self.assertEqual(summary["smoke_result"]["primary_failure"], "bridge_startup_error")
        self.assertEqual(summary["smoke_result"]["failure_detail"], "[Errno 1] Operation not permitted")
        self.assertTrue(summary["smoke_result"]["sitl_remained_alive"])

    def test_smoke_health_recomputes_failure_after_runtime_error_status(self) -> None:
        summary = {
            "status": "bridge_runtime_error",
            "bridge_protocol_declared": {"bridge_protocol_version": 1},
            "runtime_boundary_declared": {"inputs": [], "outputs": [], "transitional": []},
            "transport_ports_declared": {"imu": 9001, "motor_fb": 9002, "motor_cmd": 9003},
            "bridge_protocol": {"bridge_protocol_version": 1},
            "runtime_boundary": {"inputs": [], "outputs": [], "transitional": []},
            "transport_ports": {"imu": 9001, "motor_fb": 9002, "motor_cmd": 9003},
            "bridge_startup_complete": {"threads": ["imu"]},
            "bridge_stats_last": {"imu_sent": 10, "mit_seen": 0, "wheel_seen": 0, "fb_sent": 0},
            "sitl_output": ["Starting FreeRTOS POSIX Scheduler..."],
        }

        _summarize_runtime_boundary(summary)
        _summarize_smoke_health(summary, BALANCE_CHASSIS_PROFILE)

        self.assertFalse(summary["smoke_health"]["passed"])
        self.assertIn("session_status_ok", summary["smoke_health"]["failures"])

    def test_smoke_health_keeps_missing_motor_activity_as_warning(self) -> None:
        summary = {
            "status": "ok",
            "sitl_exit_code": -15,
            "bridge_protocol_declared": {"bridge_protocol_version": 1},
            "runtime_boundary_declared": {"inputs": [], "outputs": [], "transitional": []},
            "transport_ports_declared": {"imu": 9001, "motor_fb": 9002, "motor_cmd": 9003},
            "bridge_protocol": {"bridge_protocol_version": 1},
            "runtime_boundary": {"inputs": [], "outputs": [], "transitional": []},
            "transport_ports": {"imu": 9001, "motor_fb": 9002, "motor_cmd": 9003},
            "bridge_startup_complete": {"threads": ["imu"]},
            "bridge_stats_last": {"imu_sent": 10, "mit_seen": 0, "wheel_seen": 0, "fb_sent": 0},
            "sitl_output": ["Starting FreeRTOS POSIX Scheduler..."],
        }

        _summarize_runtime_boundary(summary)
        _summarize_smoke_health(summary, BALANCE_CHASSIS_PROFILE)
        _build_smoke_result(summary)

        self.assertTrue(summary["smoke_health"]["passed"])
        self.assertEqual(summary["smoke_health"]["warnings"], ["motor_command_seen", "motor_feedback_active"])
        self.assertEqual(summary["smoke_result"]["warnings"], ["motor_command_seen", "motor_feedback_active"])

    def test_validation_targets_are_declared_for_reporting(self) -> None:
        summary: dict[str, object] = {}

        _summarize_validation_targets(summary, BALANCE_CHASSIS_PROFILE)

        self.assertEqual(
            summary["validation_summary"],
            {
                "declared_count": 2,
                "required_count": 2,
                "observed_count": 0,
                "pending_count": 2,
            },
        )
        self.assertEqual(
            summary["validation_targets_status"],
            [
                {
                    "name": "chassis_state_summary",
                    "kind": "runtime_output",
                    "source_topics": ["chassis_state"],
                    "description": "Primary chassis runtime state summary exposed to sim/report consumers.",
                    "required_for_smoke": True,
                    "status": "declared_only",
                    "observed": False,
                },
                {
                    "name": "leg_output_pair",
                    "kind": "runtime_output",
                    "source_topics": ["leg_left", "leg_right"],
                    "description": "Left/right leg outputs that capture the main control result for the current robot profile.",
                    "required_for_smoke": True,
                    "status": "declared_only",
                    "observed": False,
                },
            ],
        )

    def test_bridge_failure_requires_sitl_to_remain_alive(self) -> None:
        summary = {
            "status": "bridge_exited_early",
            "sitl_exit_code": -15,
            "bridge_protocol_declared": {"bridge_protocol_version": 1},
            "runtime_boundary_declared": {"inputs": [], "outputs": [], "transitional": []},
            "transport_ports_declared": {"imu": 9001, "motor_fb": 9002, "motor_cmd": 9003},
            "sitl_output": ["Starting FreeRTOS POSIX Scheduler..."],
        }

        _summarize_smoke_health(summary, BALANCE_CHASSIS_PROFILE)

        self.assertTrue(summary["smoke_health"]["sitl_remained_alive"])
        self.assertIn("sitl_remained_alive", summary["smoke_health"]["required_checks"])
        self.assertNotIn("sitl_remained_alive", summary["smoke_health"]["failures"])


if __name__ == "__main__":
    unittest.main()
