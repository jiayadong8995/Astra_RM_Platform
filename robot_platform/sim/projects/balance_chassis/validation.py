from __future__ import annotations

from robot_platform.sim.core.profile import SimProjectProfile, ValidationStatus


def _append_reason(reasons: list[str], reason: str) -> None:
    if reason not in reasons:
        reasons.append(reason)


def _coerce_runtime_flag(value: object) -> bool | None:
    if isinstance(value, bool):
        return value
    if isinstance(value, int):
        return value != 0
    if isinstance(value, str):
        normalized = value.strip().lower()
        if normalized in {"1", "true", "yes", "on"}:
            return True
        if normalized in {"0", "false", "no", "off"}:
            return False
    return None


def build_validation_status(
    summary: dict[str, object],
    profile: SimProjectProfile,
) -> list[ValidationStatus]:
    observed_topics: set[str] = set()
    existing_summary = summary.get("validation_status_summary")
    safety_protection_reasons: list[str] = []
    observation_failure_reasons: list[str] = []
    control_failure_reasons: list[str] = []
    if isinstance(existing_summary, dict):
        for reason in existing_summary.get("safety_protection_reasons", []):
            if isinstance(reason, str) and reason:
                _append_reason(safety_protection_reasons, reason)
        for reason in existing_summary.get("observation_failure_reasons", []):
            if isinstance(reason, str) and reason:
                _append_reason(observation_failure_reasons, reason)
        for reason in existing_summary.get("control_failure_reasons", []):
            if isinstance(reason, str) and reason:
                _append_reason(control_failure_reasons, reason)
    runtime_output_observations = summary.get("runtime_output_observations")
    summary["runtime_output_observation_count"] = 0
    if isinstance(runtime_output_observations, list):
        summary["runtime_output_observation_count"] = len(runtime_output_observations)
        for item in runtime_output_observations:
            if not isinstance(item, dict):
                continue
            topic = item.get("topic")
            if isinstance(topic, str) and topic:
                observed_topics.add(topic)
            start_allowed = _coerce_runtime_flag(item.get("start"))
            control_enabled = _coerce_runtime_flag(item.get("control_enable"))
            actuator_enabled = _coerce_runtime_flag(item.get("actuator_enable"))
            if start_allowed is False:
                _append_reason(safety_protection_reasons, "stale_remote_input")
            if control_enabled is False:
                _append_reason(safety_protection_reasons, "control_enable_blocked")
            if actuator_enabled is False:
                _append_reason(safety_protection_reasons, "actuator_enable_blocked")
    else:
        _append_reason(observation_failure_reasons, "missing_runtime_observations")

    if safety_protection_reasons:
        _append_reason(safety_protection_reasons, "unsafe_actuator_verdict")

    status: list[ValidationStatus] = []
    for target in profile.validation_targets:
        observed_source_topics = sorted(topic for topic in target.source_topics if topic in observed_topics)
        is_observed = len(observed_source_topics) == len(target.source_topics)
        target_status = "observed" if is_observed else "declared_only"
        if observed_source_topics and not is_observed:
            target_status = "partial"
        if target.required_for_smoke and target_status != "observed":
            if summary["runtime_output_observation_count"] > 0:
                _append_reason(control_failure_reasons, "validation_target_failed")
            else:
                _append_reason(observation_failure_reasons, "missing_runtime_observations")
        status.append(
            {
                "name": target.name,
                "kind": target.kind,
                "source_topics": list(target.source_topics),
                "description": target.description,
                "required_for_smoke": target.required_for_smoke,
                "status": target_status,
                "observed": is_observed,
                "observed_source_topics": observed_source_topics,
            }
        )
    summary["validation_status_summary"] = {
        "observation_failure_reasons": observation_failure_reasons,
        "control_failure_reasons": control_failure_reasons,
        "safety_protection_reasons": safety_protection_reasons,
    }
    return status
