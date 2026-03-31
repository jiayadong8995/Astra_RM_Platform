from __future__ import annotations

from robot_platform.sim.core.profile import SimProjectProfile, ValidationStatus


def build_validation_status(
    summary: dict[str, object],
    profile: SimProjectProfile,
) -> list[ValidationStatus]:
    observed_topics: set[str] = set()
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

    status: list[ValidationStatus] = []
    for target in profile.validation_targets:
        observed_source_topics = sorted(topic for topic in target.source_topics if topic in observed_topics)
        is_observed = len(observed_source_topics) == len(target.source_topics)
        target_status = "observed" if is_observed else "declared_only"
        if observed_source_topics and not is_observed:
            target_status = "partial"
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
    return status
