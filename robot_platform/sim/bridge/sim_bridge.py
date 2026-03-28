from __future__ import annotations

import json
import socket
import sys
import threading
import time

from robot_platform.sim.bridge.protocol import (
    BRIDGE_PROTOCOL_VERSION,
    BridgeStats,
    IMU_PORT,
    MOTOR_CMD_PORT,
    MOTOR_FB_PORT,
    SITL_IP,
    ImuSample,
    MitMotorCommand,
    MotorFeedback,
    WheelCurrentCommand,
)
from robot_platform.sim.runtime_io import (
    RUNTIME_INPUT_BOUNDARY,
    RUNTIME_OUTPUT_BOUNDARY,
    RUNTIME_TRANSITIONAL_TOPICS,
)


def emit_event(event_type: str, payload: dict[str, object]) -> None:
    print(f"[BridgeEvent] {json.dumps({'type': event_type, 'payload': payload}, ensure_ascii=False)}")


def create_socket(*, bind_port: int | None = None) -> socket.socket:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    if bind_port is not None:
        sock.bind((SITL_IP, bind_port))
    return sock


def imu_thread(sock: socket.socket, stats: BridgeStats) -> None:
    sample = ImuSample()
    while True:
        sock.sendto(sample.encode(), (SITL_IP, IMU_PORT))
        stats.imu_packets_sent += 1
        time.sleep(0.001)


def motor_thread(sock_cmd: socket.socket, sock_fb: socket.socket, stats: BridgeStats) -> None:
    motor_states = {1: [0.0, 0.0], 2: [0.0, 0.0], 3: [0.0, 0.0], 4: [0.0, 0.0]}

    print(f"[Bridge] Listening for motor commands on UDP {MOTOR_CMD_PORT}...")
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

            # Current bridge behavior is intentionally minimal:
            # integrate torque into a toy velocity/position state and return feedback.
            motor_states[cmd.motor_id][1] += cmd.torque * 0.001
            motor_states[cmd.motor_id][0] += motor_states[cmd.motor_id][1] * 0.001

            feedback = MotorFeedback(
                motor_id=cmd.motor_id,
                position=motor_states[cmd.motor_id][0],
                velocity=motor_states[cmd.motor_id][1],
                torque=cmd.torque,
            )
            sock_fb.sendto(feedback.encode(), (SITL_IP, MOTOR_FB_PORT))
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
        time.sleep(1.0)


def main() -> int:
    emit_event("protocol_version", {"bridge_protocol_version": BRIDGE_PROTOCOL_VERSION})
    runtime_boundary = {
        "inputs": list(RUNTIME_INPUT_BOUNDARY.topics),
        "outputs": list(RUNTIME_OUTPUT_BOUNDARY.topics),
        "transitional": list(RUNTIME_TRANSITIONAL_TOPICS.topics),
    }
    transport_ports = {
        "imu": IMU_PORT,
        "motor_fb": MOTOR_FB_PORT,
        "motor_cmd": MOTOR_CMD_PORT,
    }

    emit_event("runtime_boundary", runtime_boundary)
    emit_event("transport_ports", transport_ports)

    try:
        imu_sock = create_socket()
        motor_cmd_sock = create_socket(bind_port=MOTOR_CMD_PORT)
        motor_fb_sock = create_socket()
    except OSError as exc:
        emit_event("startup_error", {"message": str(exc)})
        print(f"[Bridge] Failed to initialize UDP sockets: {exc}", file=sys.stderr)
        return 1

    stats = BridgeStats()
    print(
        "[Bridge] Runtime boundary "
        f"inputs={RUNTIME_INPUT_BOUNDARY.topics} "
        f"outputs={RUNTIME_OUTPUT_BOUNDARY.topics} "
        f"transitional={RUNTIME_TRANSITIONAL_TOPICS.topics}"
    )
    print(f"[Bridge] Transport ports imu={IMU_PORT} motor_fb={MOTOR_FB_PORT} motor_cmd={MOTOR_CMD_PORT}")
    print("[Bridge] Starting IMU mock thread (1000Hz)...")
    t_imu = threading.Thread(target=imu_thread, args=(imu_sock, stats), daemon=True)
    t_imu.start()

    print("[Bridge] Starting motor bridge thread...")
    t_motor = threading.Thread(target=motor_thread, args=(motor_cmd_sock, motor_fb_sock, stats), daemon=True)
    t_motor.start()

    print("[Bridge] Starting stats thread...")
    t_stats = threading.Thread(target=stats_thread, args=(stats,), daemon=True)
    t_stats.start()
    emit_event("startup_complete", {"threads": ["imu", "motor", "stats"]})

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
