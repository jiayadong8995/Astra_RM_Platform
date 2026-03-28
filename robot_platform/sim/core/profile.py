from __future__ import annotations

from collections.abc import Callable
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
class SmokeExpectations:
    require_bridge_protocol_observed: bool = True
    require_bridge_startup_complete: bool = True
    require_runtime_boundary_observed: bool = True
    require_transport_ports_observed: bool = True
    require_bridge_stats_observed: bool = True
    require_imu_stream_active: bool = True
    warn_on_missing_motor_command: bool = True
    warn_on_missing_motor_feedback: bool = True


@dataclass(frozen=True)
class ValidationTarget:
    name: str
    kind: str
    source_topics: tuple[str, ...]
    description: str
    required_for_smoke: bool = False


ValidationStatus = dict[str, object]
ValidationStatusBuilder = Callable[[dict[str, object], "SimProjectProfile"], list[ValidationStatus]]


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
    smoke_expectations: SmokeExpectations
    validation_targets: tuple[ValidationTarget, ...] = ()
    validation_status_builder: ValidationStatusBuilder | None = None
