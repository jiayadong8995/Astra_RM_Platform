from __future__ import annotations

import random
from dataclasses import asdict
from math import degrees, radians
from pathlib import Path

import yaml

from robot_platform.sim.physics.simple_balance import (
    BalanceParams,
    BalanceState,
    ControllerParams,
    SimpleBalanceController,
    SimpleBalancePlant,
)
from robot_platform.sim.reports.report_writer import write_report


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def _default_scenario_dir() -> Path:
    return _repo_root() / "sim" / "scenarios"


def _default_report_dir() -> Path:
    return _repo_root() / "sim" / "reports"


def _load_yaml(path: Path) -> dict:
    return yaml.safe_load(path.read_text(encoding="utf-8"))


def _scenario_path(name_or_path: str) -> Path:
    raw = Path(name_or_path)
    if raw.exists():
        return raw
    return _default_scenario_dir() / f"{name_or_path}.yaml"


def run_physics_scenario(name_or_path: str) -> dict:
    scenario_path = _scenario_path(name_or_path)
    scenario = _load_yaml(scenario_path)

    dt = float(scenario["dt_s"])
    duration = float(scenario["duration_s"])
    steps = int(duration / dt)

    initial = scenario["initial"]
    disturbance = scenario["disturbance"]
    noise = scenario["noise"]
    checks = scenario["checks"]

    plant = SimpleBalancePlant(
        BalanceParams(**scenario["plant"]),
        BalanceState(
            theta=radians(float(initial["theta_deg"])),
            theta_dot=radians(float(initial["theta_dot_deg_s"])),
            x=float(initial["x_m"]),
            x_dot=float(initial["x_dot_m_s"]),
        ),
    )
    controller = SimpleBalanceController(ControllerParams(**scenario["controller"]))

    max_abs_theta_deg = 0.0
    max_abs_x_m = 0.0
    trace: list[dict] = []

    for step in range(steps):
        t = step * dt
        disturbance_input = 0.0
        if disturbance["start_s"] <= t <= disturbance["end_s"]:
            disturbance_input = float(disturbance["torque_bias"])

        measured_theta = plant.state.theta + radians(random.gauss(0.0, float(noise["imu_theta_deg_std"])))
        measured_state = BalanceState(
            theta=measured_theta,
            theta_dot=plant.state.theta_dot,
            x=plant.state.x,
            x_dot=plant.state.x_dot,
        )
        torque = controller.step(measured_state, theta_target=0.0, x_target=0.0)
        state = plant.step(torque, disturbance_input, dt)

        abs_theta_deg = abs(degrees(state.theta))
        max_abs_theta_deg = max(max_abs_theta_deg, abs_theta_deg)
        max_abs_x_m = max(max_abs_x_m, abs(state.x))

        if step % max(1, int(0.01 / dt)) == 0:
            trace.append(
                {
                    "t": round(t, 4),
                    "theta_deg": round(degrees(state.theta), 4),
                    "theta_dot_deg_s": round(degrees(state.theta_dot), 4),
                    "x_m": round(state.x, 4),
                    "x_dot_m_s": round(state.x_dot, 4),
                    "torque_cmd": round(torque, 4),
                }
            )

    final_theta_deg = abs(degrees(plant.state.theta))
    passed = (
        max_abs_theta_deg <= float(checks["max_abs_theta_deg"])
        and final_theta_deg <= float(checks["final_abs_theta_deg"])
        and max_abs_x_m <= float(checks["max_abs_x_m"])
    )

    result = {
        "scenario": scenario["name"],
        "description": scenario.get("description", ""),
        "passed": passed,
        "metrics": {
            "max_abs_theta_deg": round(max_abs_theta_deg, 4),
            "final_abs_theta_deg": round(final_theta_deg, 4),
            "max_abs_x_m": round(max_abs_x_m, 4),
        },
        "checks": checks,
        "controller": asdict(ControllerParams(**scenario["controller"])),
        "plant": asdict(BalanceParams(**scenario["plant"])),
        "trace": trace,
    }

    report_path = _default_report_dir() / f"{scenario['name']}.json"
    write_report(report_path, result)
    result["report_path"] = str(report_path)
    return result
