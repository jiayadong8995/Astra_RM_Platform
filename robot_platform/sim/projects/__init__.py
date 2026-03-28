from __future__ import annotations

from collections.abc import Callable

from robot_platform.sim.projects.balance_chassis import BALANCE_CHASSIS_PROFILE, run_smoke_session
from robot_platform.sim.core.profile import SimProjectProfile


SmokeRunner = Callable[..., int]

SIM_PROJECTS: dict[str, dict[str, object]] = {
    BALANCE_CHASSIS_PROFILE.name: {
        "profile": BALANCE_CHASSIS_PROFILE,
        "smoke_runner": run_smoke_session,
    }
}


def get_project_names() -> tuple[str, ...]:
    return tuple(SIM_PROJECTS.keys())


def get_project_smoke_runner(project: str) -> SmokeRunner | None:
    entry = SIM_PROJECTS.get(project)
    if not isinstance(entry, dict):
        return None
    runner = entry.get("smoke_runner")
    if callable(runner):
        return runner
    return None


def get_project_profile(project: str) -> SimProjectProfile | None:
    entry = SIM_PROJECTS.get(project)
    if not isinstance(entry, dict):
        return None
    profile = entry.get("profile")
    if isinstance(profile, SimProjectProfile):
        return profile
    return None
