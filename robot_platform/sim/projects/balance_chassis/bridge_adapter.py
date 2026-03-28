from __future__ import annotations

from robot_platform.sim.backends.sitl_contract import SitlBackendAdapter
from robot_platform.sim.core.protocol import ImuSample, MotorFeedback


def create_default_imu_sample() -> ImuSample:
    return ImuSample()


def integrate_toy_motor_state(
    *,
    position: float,
    velocity: float,
    torque: float,
) -> tuple[float, float]:
    next_velocity = velocity + torque * 0.001
    next_position = position + next_velocity * 0.001
    return next_position, next_velocity


def create_motor_feedback(*, motor_id: int, position: float, velocity: float, torque: float) -> MotorFeedback:
    return MotorFeedback(
        motor_id=motor_id,
        position=position,
        velocity=velocity,
        torque=torque,
    )


SITL_BACKEND_ADAPTER = SitlBackendAdapter(
    create_default_imu_sample=create_default_imu_sample,
    integrate_toy_motor_state=integrate_toy_motor_state,
    create_motor_feedback=create_motor_feedback,
)
