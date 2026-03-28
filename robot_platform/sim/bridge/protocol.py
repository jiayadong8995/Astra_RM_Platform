from __future__ import annotations

from dataclasses import dataclass
import struct


BRIDGE_PROTOCOL_VERSION = 1

SITL_IP = "127.0.0.1"
IMU_PORT = 9001
MOTOR_FB_PORT = 9002
MOTOR_CMD_PORT = 9003


@dataclass(frozen=True)
class ImuSample:
    gyro_x: float = 0.0
    gyro_y: float = 0.0
    gyro_z: float = 0.0
    accel_x: float = 0.0
    accel_y: float = 0.0
    accel_z: float = 9.81
    temperature: float = 26.0

    def encode(self) -> bytes:
        return struct.pack(
            "fffffff",
            self.gyro_x,
            self.gyro_y,
            self.gyro_z,
            self.accel_x,
            self.accel_y,
            self.accel_z,
            self.temperature,
        )


@dataclass(frozen=True)
class MitMotorCommand:
    motor_id: int
    position: float
    velocity: float
    kp: float
    kd: float
    torque: float

    @classmethod
    def decode(cls, payload: bytes) -> "MitMotorCommand":
        _, motor_id, position, velocity, kp, kd, torque = struct.unpack("<IIfffff", payload[:28])
        return cls(
            motor_id=motor_id,
            position=position,
            velocity=velocity,
            kp=kp,
            kd=kd,
            torque=torque,
        )


@dataclass(frozen=True)
class WheelCurrentCommand:
    motor1_current: float
    motor2_current: float

    @classmethod
    def decode(cls, payload: bytes) -> "WheelCurrentCommand":
        _, motor1_current, motor2_current = struct.unpack("<Iff", payload[:12])
        return cls(motor1_current=motor1_current, motor2_current=motor2_current)


@dataclass(frozen=True)
class MotorFeedback:
    motor_id: int
    position: float
    velocity: float
    torque: float

    def encode(self) -> bytes:
        return struct.pack("<Ifff", self.motor_id, self.position, self.velocity, self.torque)


@dataclass
class BridgeStats:
    imu_packets_sent: int = 0
    mit_commands_seen: int = 0
    wheel_commands_seen: int = 0
    motor_feedback_sent: int = 0
