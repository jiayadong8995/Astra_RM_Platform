from __future__ import annotations

from pathlib import Path

from robot_platform.sim.core.runner import run_profile_session
from robot_platform.sim.projects.balance_chassis.profile import BALANCE_CHASSIS_PROFILE


def run_smoke_session(*, repo_root: Path, duration_s: float = 3.0) -> int:
    return run_profile_session(
        repo_root=repo_root,
        profile=BALANCE_CHASSIS_PROFILE,
        duration_s=duration_s,
    )
