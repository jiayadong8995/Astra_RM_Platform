from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class RuntimeTopicBoundary:
    role: str
    topics: tuple[str, ...]


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
