from pathlib import Path

from robot_platform.sim.core.runner import (
    _build_smoke_result,
    _detect_runtime_error,
    _extract_bridge_metadata,
    _sitl_remained_alive,
    _summarize_bridge_stats,
    _summarize_runtime_boundary,
    _summarize_validation_targets,
    _summarize_smoke_health,
)
from robot_platform.sim.projects.balance_chassis.smoke import run_smoke_session


def run_sitl_session(
    *,
    repo_root: Path,
    duration_s: float = 3.0,
) -> int:
    return run_smoke_session(
        repo_root=repo_root,
        duration_s=duration_s,
    )
