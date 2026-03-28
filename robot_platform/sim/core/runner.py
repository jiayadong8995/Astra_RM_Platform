from __future__ import annotations

import ast
import json
import re
import subprocess
import sys
import time
from pathlib import Path

from robot_platform.sim.core.profile import SimProjectProfile, ValidationStatus
from robot_platform.sim.reports.report_writer import write_report


def _normalize_stats_sample(payload: dict[str, object]) -> dict[str, int] | None:
    try:
        return {key: int(value) for key, value in payload.items()}
    except (AttributeError, TypeError, ValueError):
        return None


def _append_stats_sample(samples: list[dict[str, int]], sample: dict[str, int] | None) -> None:
    if sample is None:
        return
    if samples and samples[-1] == sample:
        return
    samples.append(sample)


def _append_runtime_output_observation(
    observations: list[dict[str, object]],
    observation: dict[str, object] | None,
) -> None:
    if not isinstance(observation, dict):
        return
    topic = observation.get("topic")
    if not isinstance(topic, str) or not topic:
        return
    observations.append(observation)


def _parse_key_value_line(line: str, prefix: str) -> dict[str, object] | None:
    if not line.startswith(prefix):
        return None

    payload = line[len(prefix):].strip()
    fields: dict[str, object] = {}
    for token in payload.split():
        key, sep, value = token.partition("=")
        if not sep:
            continue
        fields[key] = value
    return fields


def _extract_bridge_metadata(lines: list[str]) -> dict[str, object]:
    metadata: dict[str, object] = {}
    stats_samples: list[dict[str, int]] = []
    runtime_output_observations: list[dict[str, object]] = []
    boundary_pattern = re.compile(
        r"^\[Bridge\] Runtime boundary "
        r"inputs=(?P<inputs>\(.+?\)) "
        r"outputs=(?P<outputs>\(.+?\)) "
        r"transitional=(?P<transitional>\(.+?\))$"
    )

    for line in lines:
        if line.startswith("[BridgeEvent] "):
            try:
                event = json.loads(line[len("[BridgeEvent] "):])
            except json.JSONDecodeError:
                continue

            event_type = event.get("type")
            payload = event.get("payload")
            if not isinstance(payload, dict):
                continue

            if event_type == "runtime_boundary":
                metadata["runtime_boundary"] = payload
            elif event_type == "transport_ports":
                metadata["transport_ports"] = payload
            elif event_type == "protocol_version":
                metadata["bridge_protocol"] = payload
            elif event_type == "startup_error":
                metadata["bridge_startup_error"] = payload
            elif event_type == "startup_complete":
                metadata["bridge_startup_complete"] = payload
            elif event_type == "stats":
                _append_stats_sample(stats_samples, _normalize_stats_sample(payload))
            elif event_type == "runtime_output_observation":
                _append_runtime_output_observation(runtime_output_observations, payload)
            continue

        boundary_match = boundary_pattern.match(line)
        if boundary_match is not None:
            metadata["runtime_boundary"] = {
                "inputs": list(ast.literal_eval(boundary_match.group("inputs"))),
                "outputs": list(ast.literal_eval(boundary_match.group("outputs"))),
                "transitional": list(ast.literal_eval(boundary_match.group("transitional"))),
            }
            continue

        if line.startswith("[Bridge] Transport ports "):
            fields = _parse_key_value_line(line, "[Bridge] Transport ports ")
            if fields is not None:
                metadata["transport_ports"] = {key: int(value) for key, value in fields.items()}
            continue

        if line.startswith("[Bridge] stats "):
            fields = _parse_key_value_line(line, "[Bridge] stats ")
            if fields is not None:
                _append_stats_sample(stats_samples, _normalize_stats_sample(fields))

    if stats_samples:
        metadata["bridge_stats_samples"] = stats_samples
        metadata["bridge_stats_last"] = stats_samples[-1]
    if runtime_output_observations:
        metadata["runtime_output_observations"] = runtime_output_observations

    return metadata


def _summarize_bridge_stats(summary: dict[str, object]) -> None:
    stats_samples = summary.get("bridge_stats_samples")
    elapsed_s = float(summary.get("elapsed_s", 0.0) or 0.0)
    if not isinstance(stats_samples, list) or not stats_samples:
        return

    first_sample = stats_samples[0]
    last_sample = stats_samples[-1]
    if not isinstance(first_sample, dict) or not isinstance(last_sample, dict):
        return

    counter_keys = ("imu_sent", "mit_seen", "wheel_seen", "fb_sent")
    deltas = {
        key: int(last_sample.get(key, 0)) - int(first_sample.get(key, 0))
        for key in counter_keys
    }
    rates_per_s = {
        key: round(value / elapsed_s, 2) if elapsed_s > 0.0 else None
        for key, value in deltas.items()
    }
    summary["bridge_stats_summary"] = {
        "sample_count": len(stats_samples),
        "first": {key: int(first_sample.get(key, 0)) for key in counter_keys},
        "last": {key: int(last_sample.get(key, 0)) for key in counter_keys},
        "delta": deltas,
        "rate_per_s": rates_per_s,
    }


def _summarize_runtime_boundary(summary: dict[str, object]) -> None:
    declared_protocol = summary.get("bridge_protocol_declared")
    observed_protocol = summary.get("bridge_protocol")
    if isinstance(declared_protocol, dict) and isinstance(observed_protocol, dict):
        summary["bridge_protocol_match"] = declared_protocol == observed_protocol

    declared = summary.get("runtime_boundary_declared")
    observed = summary.get("runtime_boundary")
    if isinstance(declared, dict) and isinstance(observed, dict):
        summary["runtime_boundary_match"] = declared == observed

    declared_ports = summary.get("transport_ports_declared")
    observed_ports = summary.get("transport_ports")
    if isinstance(declared_ports, dict) and isinstance(observed_ports, dict):
        summary["transport_ports_match"] = declared_ports == observed_ports


def _sitl_remained_alive(summary: dict[str, object]) -> bool:
    sitl_exit_code = summary.get("sitl_exit_code")
    if sitl_exit_code in (-15, -9):
        return True
    return False


def _default_validation_status_builder(
    summary: dict[str, object],
    profile: SimProjectProfile,
) -> list[ValidationStatus]:
    del summary
    validation_status: list[ValidationStatus] = []
    for target in profile.validation_targets:
        validation_status.append(
            {
                "name": target.name,
                "kind": target.kind,
                "source_topics": list(target.source_topics),
                "description": target.description,
                "required_for_smoke": target.required_for_smoke,
                "status": "declared_only",
                "observed": False,
            }
        )
    return validation_status


def _summarize_validation_targets(summary: dict[str, object], profile: SimProjectProfile) -> None:
    status_builder = profile.validation_status_builder or _default_validation_status_builder
    validation_status = status_builder(summary, profile)
    summary["validation_targets_status"] = validation_status
    runtime_output_observations = summary.get("runtime_output_observations")
    summary["validation_summary"] = {
        "declared_count": len(profile.validation_targets),
        "required_count": sum(1 for target in profile.validation_targets if target.required_for_smoke),
        "observed_count": sum(
            1
            for item in validation_status
            if isinstance(item, dict) and bool(item.get("observed"))
        ),
        "pending_count": sum(
            1
            for item in validation_status
            if isinstance(item, dict) and not bool(item.get("observed"))
        ),
        "runtime_output_observation_count": len(runtime_output_observations)
        if isinstance(runtime_output_observations, list)
        else 0,
    }


def _summarize_smoke_health(summary: dict[str, object], profile: SimProjectProfile) -> None:
    sitl_lines = summary.get("sitl_output", [])
    bridge_stats = summary.get("bridge_stats_last")
    expectations = profile.smoke_expectations

    health: dict[str, object] = {
        "sitl_scheduler_started": False,
        "sitl_remained_alive": _sitl_remained_alive(summary),
        "bridge_startup_complete": isinstance(summary.get("bridge_startup_complete"), dict),
        "bridge_runtime_boundary_observed": isinstance(summary.get("runtime_boundary"), dict),
        "bridge_transport_ports_observed": isinstance(summary.get("transport_ports"), dict),
        "bridge_stats_observed": isinstance(bridge_stats, dict),
    }

    if isinstance(sitl_lines, list):
        health["sitl_scheduler_started"] = any(
            "Starting FreeRTOS POSIX Scheduler" in line for line in sitl_lines
        )

    if isinstance(bridge_stats, dict):
        imu_sent = int(bridge_stats.get("imu_sent", 0))
        mit_seen = int(bridge_stats.get("mit_seen", 0))
        wheel_seen = int(bridge_stats.get("wheel_seen", 0))
        fb_sent = int(bridge_stats.get("fb_sent", 0))
        health["imu_stream_active"] = imu_sent > 0
        health["motor_command_seen"] = (mit_seen + wheel_seen) > 0
        health["motor_feedback_active"] = fb_sent > 0
    else:
        health["imu_stream_active"] = False
        health["motor_command_seen"] = False
        health["motor_feedback_active"] = False

    required_checks = {
        "session_status_ok": summary.get("status") == "ok",
        "bridge_protocol_declared": isinstance(summary.get("bridge_protocol_declared"), dict),
        "sitl_scheduler_started": bool(health["sitl_scheduler_started"]),
        "runtime_boundary_declared": isinstance(summary.get("runtime_boundary_declared"), dict),
        "transport_ports_declared": isinstance(summary.get("transport_ports_declared"), dict),
    }

    if summary.get("status") == "bridge_exited_early":
        required_checks["sitl_remained_alive"] = bool(health["sitl_remained_alive"])

    if summary.get("status") == "ok":
        if expectations.require_bridge_protocol_observed:
            required_checks["bridge_protocol_observed"] = isinstance(summary.get("bridge_protocol"), dict)
        if expectations.require_bridge_startup_complete:
            required_checks["bridge_startup_complete"] = bool(health["bridge_startup_complete"])
        if expectations.require_runtime_boundary_observed:
            required_checks["bridge_runtime_boundary_observed"] = bool(health["bridge_runtime_boundary_observed"])
        if expectations.require_transport_ports_observed:
            required_checks["bridge_transport_ports_observed"] = bool(health["bridge_transport_ports_observed"])
        if expectations.require_bridge_stats_observed:
            required_checks["bridge_stats_observed"] = bool(health["bridge_stats_observed"])
        if expectations.require_imu_stream_active:
            required_checks["imu_stream_active"] = bool(health["imu_stream_active"])

    if "bridge_protocol_match" in summary:
        required_checks["bridge_protocol_match"] = bool(summary["bridge_protocol_match"])
    if "runtime_boundary_match" in summary:
        required_checks["runtime_boundary_match"] = bool(summary["runtime_boundary_match"])
    if "transport_ports_match" in summary:
        required_checks["transport_ports_match"] = bool(summary["transport_ports_match"])

    optional_checks: dict[str, bool] = {}
    if expectations.warn_on_missing_motor_command:
        optional_checks["motor_command_seen"] = bool(health["motor_command_seen"])
    if expectations.warn_on_missing_motor_feedback:
        optional_checks["motor_feedback_active"] = bool(health["motor_feedback_active"])

    failures = [name for name, passed in required_checks.items() if not passed]
    warnings = [name for name, passed in optional_checks.items() if not passed]
    health["required_checks"] = required_checks
    health["optional_checks"] = optional_checks
    health["checks"] = {**required_checks, **optional_checks}
    health["failures"] = failures
    health["warnings"] = warnings
    health["passed"] = len(failures) == 0

    summary["smoke_health"] = health


def _build_smoke_result(summary: dict[str, object]) -> None:
    health = summary.get("smoke_health")
    bridge_stats = summary.get("bridge_stats_last")
    bridge_stats_summary = summary.get("bridge_stats_summary")
    startup_error = summary.get("bridge_startup_error")
    validation_summary = summary.get("validation_summary")
    validation_targets_status = summary.get("validation_targets_status")
    result: dict[str, object] = {
        "status": summary.get("status"),
        "passed": False,
        "primary_failure": None,
        "failure_detail": None,
        "elapsed_s": summary.get("elapsed_s"),
        "sitl_remained_alive": _sitl_remained_alive(summary),
    }

    if isinstance(health, dict):
        failures = health.get("failures", [])
        warnings = health.get("warnings", [])
        passed = bool(health.get("passed", False))
        result["passed"] = passed
        if isinstance(warnings, list) and warnings:
            result["warnings"] = warnings
        if not passed and isinstance(startup_error, dict):
            result["primary_failure"] = "bridge_startup_error"
            result["failure_detail"] = startup_error.get("message")
        elif not passed and isinstance(failures, list) and failures:
            result["primary_failure"] = failures[0]
        elif not passed:
            result["primary_failure"] = summary.get("status")

    if isinstance(bridge_stats, dict):
        result["bridge_counters"] = {
            "imu_sent": int(bridge_stats.get("imu_sent", 0)),
            "mit_seen": int(bridge_stats.get("mit_seen", 0)),
            "wheel_seen": int(bridge_stats.get("wheel_seen", 0)),
            "fb_sent": int(bridge_stats.get("fb_sent", 0)),
        }
    if isinstance(bridge_stats_summary, dict):
        result["bridge_activity"] = bridge_stats_summary
    if isinstance(validation_summary, dict):
        result["validation"] = {
            "declared_count": int(validation_summary.get("declared_count", 0)),
            "required_count": int(validation_summary.get("required_count", 0)),
            "observed_count": int(validation_summary.get("observed_count", 0)),
            "pending_count": int(validation_summary.get("pending_count", 0)),
        }
    if isinstance(validation_targets_status, list):
        pending_required = [
            item.get("name")
            for item in validation_targets_status
            if isinstance(item, dict)
            and bool(item.get("required_for_smoke"))
            and item.get("status") != "observed"
        ]
        if pending_required:
            result["pending_validation_targets"] = pending_required

    summary["smoke_result"] = result


def _detect_runtime_error(summary: dict[str, object]) -> bool:
    bridge_lines = summary.get("bridge_output", [])
    sitl_lines = summary.get("sitl_output", [])
    bridge_has_error = isinstance(bridge_lines, list) and any(
        "Traceback" in line or "Failed to initialize UDP sockets" in line for line in bridge_lines
    )
    sitl_has_error = isinstance(sitl_lines, list) and any("Traceback" in line for line in sitl_lines)
    return bridge_has_error or sitl_has_error


def run_profile_session(
    *,
    repo_root: Path,
    profile: SimProjectProfile,
    duration_s: float = 3.0,
) -> int:
    build_dir = repo_root / "build" / "robot_platform_sitl_make"
    sitl_bin = build_dir / profile.sitl_binary_name
    report_path = repo_root / "build" / "sim_reports" / profile.report_name

    summary: dict[str, object] = {
        "sim_mode": "sitl",
        "sim_project": profile.name,
        "smoke_report_version": 1,
        "duration_s": duration_s,
        "report_path": str(report_path),
        "sitl_binary": str(sitl_bin),
        "sim_backend_module": profile.backend_module,
        "backend_command": [sys.executable, "-u", "-m", profile.backend_module, "--project", profile.name],
        "bridge_command": [sys.executable, "-u", "-m", profile.backend_module, "--project", profile.name],
        "sitl_command": [str(sitl_bin)],
        "bridge_protocol_declared": {
            "bridge_protocol_version": profile.bridge_protocol_version,
        },
        "runtime_boundary_declared": {
            "inputs": list(profile.runtime_input_boundary.topics),
            "outputs": list(profile.runtime_output_boundary.topics),
            "transitional": list(profile.runtime_transitional_topics.topics),
        },
        "transport_ports_declared": {
            "imu": profile.transport_ports.imu,
            "motor_fb": profile.transport_ports.motor_fb,
            "motor_cmd": profile.transport_ports.motor_cmd,
        },
        "validation_targets_declared": [
            {
                "name": target.name,
                "kind": target.kind,
                "source_topics": list(target.source_topics),
                "description": target.description,
                "required_for_smoke": target.required_for_smoke,
            }
            for target in profile.validation_targets
        ],
    }

    if not sitl_bin.exists():
        summary["status"] = "missing_sitl_binary"
        summary["error"] = f"Build `{profile.sitl_target}` before running `sim`."
        write_report(report_path, summary)
        print(f"sim report: {report_path}")
        return 2

    bridge_proc: subprocess.Popen[str] | None = None
    sitl_proc: subprocess.Popen[str] | None = None
    started_at = time.monotonic()

    try:
        bridge_proc = subprocess.Popen(
            [sys.executable, "-u", "-m", profile.backend_module, "--project", profile.name],
            cwd=repo_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        sitl_proc = subprocess.Popen(
            [str(sitl_bin)],
            cwd=repo_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        deadline = started_at + duration_s
        failure_reason = None
        while time.monotonic() < deadline:
            if bridge_proc.poll() is not None:
                failure_reason = "bridge_exited_early"
                break
            if sitl_proc.poll() is not None:
                failure_reason = "sitl_exited_early"
                break
            time.sleep(0.1)

        summary["elapsed_s"] = round(time.monotonic() - started_at, 3)
        summary["bridge_exit_code"] = bridge_proc.poll()
        summary["sitl_exit_code"] = sitl_proc.poll()

        if failure_reason is None:
            summary["status"] = "ok"
            rc = 0
        else:
            summary["status"] = failure_reason
            rc = 1

    finally:
        for proc in (sitl_proc, bridge_proc):
            if proc is None:
                continue
            if proc.poll() is None:
                proc.terminate()
                try:
                    proc.wait(timeout=2.0)
                except subprocess.TimeoutExpired:
                    proc.kill()
                    proc.wait(timeout=2.0)

        if sitl_proc is not None:
            sitl_output, _ = sitl_proc.communicate()
            summary["sitl_output"] = sitl_output.splitlines()
            summary["sitl_exit_code"] = sitl_proc.returncode
        if bridge_proc is not None:
            bridge_output, _ = bridge_proc.communicate()
            summary["bridge_output"] = bridge_output.splitlines()
            summary["bridge_exit_code"] = bridge_proc.returncode
            summary.update(_extract_bridge_metadata(summary["bridge_output"]))
        if summary.get("status") == "ok" and _detect_runtime_error(summary):
            summary["status"] = "bridge_runtime_error"
            rc = 1

        _summarize_runtime_boundary(summary)
        _summarize_bridge_stats(summary)
        _summarize_validation_targets(summary, profile)
        _summarize_smoke_health(summary, profile)
        _build_smoke_result(summary)

        write_report(report_path, summary)
        smoke_result = summary.get("smoke_result", {})
        if isinstance(smoke_result, dict):
            validation_suffix = ""
            validation = smoke_result.get("validation")
            if isinstance(validation, dict):
                observed = validation.get("observed_count")
                required = validation.get("required_count")
                validation_suffix = f" validation={observed}/{required}"
            print(
                "sim summary: "
                f"project={profile.name} "
                f"backend={profile.backend_module} "
                f"status={smoke_result.get('status')} "
                f"passed={smoke_result.get('passed')} "
                f"failure={smoke_result.get('primary_failure')}"
                f"{validation_suffix}"
            )
        print(f"sim report: {report_path}")

    return rc
