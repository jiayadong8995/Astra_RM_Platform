from __future__ import annotations

from robot_platform.sim.core.profile import RuntimeTopicBoundary, SimProjectProfile, TransportPorts
from robot_platform.sim.core.protocol import BRIDGE_PROTOCOL_VERSION


RUNTIME_INPUT_BOUNDARY = RuntimeTopicBoundary(
    role="official_input",
    topics=(
        "ins_data",
        "chassis_cmd",
    ),
)

RUNTIME_OUTPUT_BOUNDARY = RuntimeTopicBoundary(
    role="official_output",
    topics=(
        "chassis_state",
        "leg_left",
        "leg_right",
    ),
)

RUNTIME_TRANSITIONAL_TOPICS = RuntimeTopicBoundary(
    role="transitional_internal",
    topics=("chassis_observe",),
)

BALANCE_CHASSIS_PROFILE = SimProjectProfile(
    name="balance_chassis",
    sitl_target="balance_chassis_sitl",
    sitl_binary_name="balance_chassis_sitl",
    report_name="sitl_smoke.json",
    bridge_module="robot_platform.sim.bridge.sim_bridge",
    bridge_protocol_version=BRIDGE_PROTOCOL_VERSION,
    runtime_input_boundary=RUNTIME_INPUT_BOUNDARY,
    runtime_output_boundary=RUNTIME_OUTPUT_BOUNDARY,
    runtime_transitional_topics=RUNTIME_TRANSITIONAL_TOPICS,
    transport_ports=TransportPorts(
        imu=9001,
        motor_fb=9002,
        motor_cmd=9003,
    ),
)
