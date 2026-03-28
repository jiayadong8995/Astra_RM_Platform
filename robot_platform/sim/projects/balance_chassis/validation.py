from __future__ import annotations

from robot_platform.sim.core.profile import SimProjectProfile, ValidationStatus


def build_validation_status(
    summary: dict[str, object],
    profile: SimProjectProfile,
) -> list[ValidationStatus]:
    del summary
    status: list[ValidationStatus] = []
    for target in profile.validation_targets:
        status.append(
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
    return status
