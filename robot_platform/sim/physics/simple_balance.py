from __future__ import annotations

from dataclasses import dataclass
from math import copysign


@dataclass
class BalanceState:
    theta: float
    theta_dot: float
    x: float
    x_dot: float


@dataclass
class BalanceParams:
    gravity_gain: float
    torque_gain: float
    damping: float
    velocity_damping: float
    wheel_coupling: float


@dataclass
class ControllerParams:
    kp_theta: float
    kd_theta: float
    kp_x: float
    kd_x: float
    max_torque: float


def clamp(value: float, limit: float) -> float:
    if abs(value) <= limit:
        return value
    return copysign(limit, value)


class SimpleBalancePlant:
    def __init__(self, params: BalanceParams, initial: BalanceState):
        self.params = params
        self.state = initial

    def step(self, torque_cmd: float, disturbance: float, dt: float) -> BalanceState:
        p = self.params
        s = self.state

        theta_ddot = (
            p.gravity_gain * s.theta
            + p.torque_gain * torque_cmd
            - p.damping * s.theta_dot
            + disturbance
        )
        x_ddot = p.wheel_coupling * torque_cmd - p.velocity_damping * s.x_dot

        s.theta_dot += theta_ddot * dt
        s.theta += s.theta_dot * dt
        s.x_dot += x_ddot * dt
        s.x += s.x_dot * dt
        return BalanceState(s.theta, s.theta_dot, s.x, s.x_dot)


class SimpleBalanceController:
    def __init__(self, params: ControllerParams):
        self.params = params

    def step(self, state: BalanceState, theta_target: float, x_target: float) -> float:
        p = self.params
        torque = (
            p.kp_theta * (theta_target - state.theta)
            + p.kd_theta * (0.0 - state.theta_dot)
            + p.kp_x * (x_target - state.x)
            + p.kd_x * (0.0 - state.x_dot)
        )
        return clamp(torque, p.max_torque)
