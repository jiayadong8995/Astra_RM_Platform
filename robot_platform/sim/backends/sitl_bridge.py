from __future__ import annotations

import argparse
import importlib
import json
import socket
import sys
import threading
import time
from typing import Sequence

from robot_platform.sim.backends.sitl_contract import SitlBackendAdapter
from robot_platform.sim.core.protocol import (
    BRIDGE_PROTOCOL_VERSION,
    BridgeStats,
    MitMotorCommand,
    SITL_IP,
    WheelCurrentCommand,
)
from robot_platform.sim.projects import get_project_profile

STATS_PERIOD_S = 0.1
RUNTIME_OUTPUT_PERIOD_S = 0.1


def emit_event(event_type: str, payload: dict[str, object]) -> None:
    print(f"[BridgeEvent] {json.dumps({'type': event_type, 'payload': payload}, ensure_ascii=False)}")


def _parse_args(argv: Sequence[str] | None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(prog="sitl_bridge", add_help=False)
    parser.add_argument("--project", required=True)
    return parser.parse_args(list(argv) if argv is not None else None)


def _load_bridge_adapter(module_name: str) -> SitlBackendAdapter:
    module = importlib.import_module(module_name)
    adapter = getattr(module, "SITL_BACKEND_ADAPTER", None)
    if not isinstance(adapter, SitlBackendAdapter):
        raise TypeError(f"{module_name} does not export SITL_BACKEND_ADAPTER")
    return adapter


def create_socket(*, bind_port: int | None = None) -> socket.socket:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    if bind_port is not None:
        sock.bind((SITL_IP, bind_port))
    return sock


def imu_thread(
    sock: socket.socket,
    stats: BridgeStats,
    *,
    imu_port: int,
    create_default_imu_sample,
) -> None:
    sample = create_default_imu_sample()
    while True:
        sock.sendto(sample.encode(), (SITL_IP, imu_port))
        stats.imu_packets_sent += 1
        time.sleep(0.001)


def motor_thread(
    sock_cmd: socket.socket,
    sock_fb: socket.socket,
    stats: BridgeStats,
    *,
    motor_fb_port: int,
    motor_cmd_port: int,
    integrate_toy_motor_state,
    create_motor_feedback,
) -> None:
    motor_states = {1: [0.0, 0.0], 2: [0.0, 0.0], 3: [0.0, 0.0], 4: [0.0, 0.0]}

    print(f"[Bridge] Listening for motor commands on UDP {motor_cmd_port}...")
    while True:
        data, _ = sock_cmd.recvfrom(1024)
        if len(data) < 4:
            continue

        cmd_type = int.from_bytes(data[:4], byteorder="little", signed=False)
        if cmd_type == 1 and len(data) >= 28:
            cmd = MitMotorCommand.decode(data)
            stats.mit_commands_seen += 1

            if cmd.motor_id not in motor_states:
                continue

            next_position, next_velocity = integrate_toy_motor_state(
                position=motor_states[cmd.motor_id][0],
                velocity=motor_states[cmd.motor_id][1],
                torque=cmd.torque,
            )
            motor_states[cmd.motor_id][0] = next_position
            motor_states[cmd.motor_id][1] = next_velocity

            feedback = create_motor_feedback(
                motor_id=cmd.motor_id,
                position=next_position,
                velocity=next_velocity,
                torque=cmd.torque,
            )
            sock_fb.sendto(feedback.encode(), (SITL_IP, motor_fb_port))
            stats.motor_feedback_sent += 1
        elif cmd_type == 2 and len(data) >= 12:
            WheelCurrentCommand.decode(data)
            stats.wheel_commands_seen += 1


def stats_thread(stats: BridgeStats) -> None:
    while True:
        snapshot = {
            "imu_sent": stats.imu_packets_sent,
            "mit_seen": stats.mit_commands_seen,
            "wheel_seen": stats.wheel_commands_seen,
            "fb_sent": stats.motor_feedback_sent,
        }
        emit_event("stats", snapshot)
        print(
            "[Bridge] stats "
            f"imu_sent={snapshot['imu_sent']} "
            f"mit_seen={snapshot['mit_seen']} "
            f"wheel_seen={snapshot['wheel_seen']} "
            f"fb_sent={snapshot['fb_sent']}"
        )
        time.sleep(STATS_PERIOD_S)


def runtime_output_thread(*, collect_runtime_output_observations) -> None:
    last_by_topic: dict[str, dict[str, object]] = {}
    while True:
        observations = collect_runtime_output_observations()
        if not isinstance(observations, tuple):
            observations = tuple(observations)

        for observation in observations:
            if not isinstance(observation, dict):
                continue
            topic = observation.get("topic")
            if not isinstance(topic, str) or not topic:
                continue
            if last_by_topic.get(topic) == observation:
                continue
            last_by_topic[topic] = observation
            emit_event("runtime_output_observation", observation)

        time.sleep(RUNTIME_OUTPUT_PERIOD_S)


def main(argv: Sequence[str] | None = None, *, default_project: str | None = None) -> int:
    if argv is None and default_project is not None:
        args = _parse_args(["--project", default_project])
    else:
        args = _parse_args(argv)

    profile = get_project_profile(args.project)
    if profile is None:
        print(f"[Bridge] Unknown sim project: {args.project}", file=sys.stderr)
        return 2

    try:
        adapter = _load_bridge_adapter(profile.bridge_adapter_module)
    except (ImportError, TypeError) as exc:
        emit_event("startup_error", {"message": str(exc)})
        print(f"[Bridge] Failed to load project adapter: {exc}", file=sys.stderr)
        return 2
    emit_event("protocol_version", {"bridge_protocol_version": BRIDGE_PROTOCOL_VERSION})
    runtime_boundary = {
        "inputs": list(profile.runtime_input_boundary.topics),
        "outputs": list(profile.runtime_output_boundary.topics),
        "transitional": list(profile.runtime_transitional_topics.topics),
    }
    transport_ports = {
        "imu": profile.transport_ports.imu,
        "motor_fb": profile.transport_ports.motor_fb,
        "motor_cmd": profile.transport_ports.motor_cmd,
    }

    emit_event("runtime_boundary", runtime_boundary)
    emit_event("transport_ports", transport_ports)

    try:
        imu_sock = create_socket()
        motor_cmd_sock = create_socket(bind_port=profile.transport_ports.motor_cmd)
        motor_fb_sock = create_socket()
    except OSError as exc:
        emit_event("startup_error", {"message": str(exc)})
        print(f"[Bridge] Failed to initialize UDP sockets: {exc}", file=sys.stderr)
        return 1

    stats = BridgeStats()
    print(
        "[Bridge] Runtime boundary "
        f"inputs={profile.runtime_input_boundary.topics} "
        f"outputs={profile.runtime_output_boundary.topics} "
        f"transitional={profile.runtime_transitional_topics.topics}"
    )
    print(
        "[Bridge] Transport ports "
        f"imu={profile.transport_ports.imu} "
        f"motor_fb={profile.transport_ports.motor_fb} "
        f"motor_cmd={profile.transport_ports.motor_cmd}"
    )
    print(f"[Bridge] Starting {profile.name} SITL backend...")
    t_imu = threading.Thread(
        target=imu_thread,
        args=(imu_sock, stats),
        kwargs={
            "imu_port": profile.transport_ports.imu,
            "create_default_imu_sample": adapter.create_default_imu_sample,
        },
        daemon=True,
    )
    t_imu.start()

    t_motor = threading.Thread(
        target=motor_thread,
        args=(motor_cmd_sock, motor_fb_sock, stats),
        kwargs={
            "motor_fb_port": profile.transport_ports.motor_fb,
            "motor_cmd_port": profile.transport_ports.motor_cmd,
            "integrate_toy_motor_state": adapter.integrate_toy_motor_state,
            "create_motor_feedback": adapter.create_motor_feedback,
        },
        daemon=True,
    )
    t_motor.start()

    t_stats = threading.Thread(target=stats_thread, args=(stats,), daemon=True)
    t_stats.start()
    t_runtime_outputs = threading.Thread(
        target=runtime_output_thread,
        kwargs={
            "collect_runtime_output_observations": adapter.collect_runtime_output_observations,
        },
        daemon=True,
    )
    t_runtime_outputs.start()
    emit_event(
        "startup_complete",
        {"threads": ["imu", "motor", "stats", "runtime_outputs"], "project": profile.name},
    )

    try:
        while True:
            time.sleep(1.0)
    except KeyboardInterrupt:
        return 0
    finally:
        imu_sock.close()
        motor_cmd_sock.close()
        motor_fb_sock.close()


if __name__ == "__main__":
    raise SystemExit(main())
