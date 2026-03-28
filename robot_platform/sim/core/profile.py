from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class RuntimeTopicBoundary:
    role: str
    topics: tuple[str, ...]


@dataclass(frozen=True)
class TransportPorts:
    imu: int
    motor_fb: int
    motor_cmd: int


@dataclass(frozen=True)
class SimProjectProfile:
    name: str
    sitl_target: str
    sitl_binary_name: str
    report_name: str
    bridge_module: str
    bridge_protocol_version: int
    runtime_input_boundary: RuntimeTopicBoundary
    runtime_output_boundary: RuntimeTopicBoundary
    runtime_transitional_topics: RuntimeTopicBoundary
    transport_ports: TransportPorts

