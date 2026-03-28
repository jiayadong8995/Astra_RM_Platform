from robot_platform.sim.core.protocol import (
    BRIDGE_PROTOCOL_VERSION,
    BridgeStats,
    ImuSample,
    MitMotorCommand,
    MotorFeedback,
    SITL_IP,
    WheelCurrentCommand,
)
from robot_platform.sim.projects.balance_chassis.profile import BALANCE_CHASSIS_PROFILE

IMU_PORT = BALANCE_CHASSIS_PROFILE.transport_ports.imu
MOTOR_FB_PORT = BALANCE_CHASSIS_PROFILE.transport_ports.motor_fb
MOTOR_CMD_PORT = BALANCE_CHASSIS_PROFILE.transport_ports.motor_cmd
