from __future__ import annotations

from dataclasses import dataclass
from typing import Callable

from robot_platform.sim.core.protocol import ImuSample, MotorFeedback


CreateDefaultImuSample = Callable[[], ImuSample]
IntegrateToyMotorState = Callable[..., tuple[float, float]]
CreateMotorFeedback = Callable[..., MotorFeedback]


@dataclass(frozen=True)
class SitlBackendAdapter:
    create_default_imu_sample: CreateDefaultImuSample
    integrate_toy_motor_state: IntegrateToyMotorState
    create_motor_feedback: CreateMotorFeedback
