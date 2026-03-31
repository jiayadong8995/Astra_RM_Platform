from __future__ import annotations

from robot_platform.sim.core.profile import (
    RuntimeTopicBoundary,
    SimProjectProfile,
    SmokeExpectations,
    TransportPorts,
    ValidationTarget,
)
from robot_platform.sim.core.protocol import BRIDGE_PROTOCOL_VERSION
from robot_platform.sim.projects.balance_chassis.validation import build_validation_status


RUNTIME_INPUT_BOUNDARY = RuntimeTopicBoundary(
    role="official_input",
    topics=(
        "ins_data",
        "chassis_cmd",
    ),
)

RUNTIME_OUTPUT_BOUNDARY = RuntimeTopicBoundary(
    role="official_output",
    topics=("actuator_command",),
)

RUNTIME_TRANSITIONAL_TOPICS = RuntimeTopicBoundary(
    role="transitional_internal",
    topics=("chassis_observe",),
)

VALIDATION_TARGETS = (
    ValidationTarget(
        name="actuator_command_stream",
        kind="runtime_output",
        source_topics=("actuator_command",),
        description="Observed actuator-command traffic emitted by the current balance_chassis runtime path.",
        required_for_smoke=True,
    ),
)

BALANCE_CHASSIS_PROFILE = SimProjectProfile(
    name="balance_chassis",
    sitl_target="balance_chassis_sitl",
    sitl_binary_name="balance_chassis_sitl",
    report_name="sitl_smoke.json",
    backend_module="robot_platform.sim.backends.sitl_bridge",
    bridge_adapter_module="robot_platform.sim.projects.balance_chassis.bridge_adapter",
    bridge_protocol_version=BRIDGE_PROTOCOL_VERSION,
    runtime_input_boundary=RUNTIME_INPUT_BOUNDARY,
    runtime_output_boundary=RUNTIME_OUTPUT_BOUNDARY,
    runtime_transitional_topics=RUNTIME_TRANSITIONAL_TOPICS,
    transport_ports=TransportPorts(
        imu=9001,
        remote=9004,
        motor_fb=9002,
        motor_cmd=9003,
    ),
    smoke_expectations=SmokeExpectations(
        require_bridge_protocol_observed=True,
        require_bridge_startup_complete=True,
        require_runtime_boundary_observed=True,
        require_transport_ports_observed=True,
        require_bridge_stats_observed=True,
        require_imu_stream_active=True,
        warn_on_missing_motor_command=True,
        warn_on_missing_motor_feedback=True,
    ),
    validation_targets=VALIDATION_TARGETS,
    validation_status_builder=build_validation_status,
)
