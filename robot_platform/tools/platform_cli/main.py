"""Minimal CLI for the current robot_platform bootstrap stage."""

import json
import subprocess
import sys
from pathlib import Path

from robot_platform.tools.cubemx_backend.main import run_codegen


HELP = {
    "generate": "Run STM32CubeMX CLI code generation into runtime/generated.",
    "build": "Configure and build the current GCC hardware seed target.",
    "flash": "Flash hardware firmware target.",
    "debug": "Start hardware debug session.",
    "replay": "Run log replay.",
    "sim": "Run simulation scenario.",
    "test": "Unified regression entry.",
}


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def _run(cmd: list[str], cwd: Path) -> int:
    print(f"+ {' '.join(str(part) for part in cmd)}")
    completed = subprocess.run(cmd, cwd=cwd)
    return completed.returncode


def _generate_balance_chassis() -> int:
    repo_root = _repo_root()
    ioc_path = repo_root / "Astra_RM2025_Balance" / "Chassis" / "CtrlBoard-H7_IMU.ioc"
    out_dir = repo_root / "robot_platform" / "runtime" / "generated" / "stm32h7_ctrl_board_raw"
    return run_codegen(ioc_path, out_dir)


def _build_hw_seed(target: str) -> int:
    repo_root = _repo_root()
    source_dir = repo_root / "robot_platform"
    build_dir = repo_root / "build" / "robot_platform_ninja"
    toolchain_file = source_dir / "cmake" / "toolchains" / "arm-none-eabi-gcc.cmake"

    configure_cmd = [
        "cmake",
        "-S",
        str(source_dir),
        "-B",
        str(build_dir),
        "-G",
        "Ninja",
        f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}",
    ]
    build_cmd = [
        "cmake",
        "--build",
        str(build_dir),
        "--target",
        target,
        "-j4",
    ]

    rc = _run(configure_cmd, cwd=repo_root)
    if rc != 0:
        return rc
    return _run(build_cmd, cwd=repo_root)


def _build_sitl(target: str) -> int:
    repo_root = _repo_root()
    source_dir = repo_root / "robot_platform"
    build_dir = repo_root / "build" / "robot_platform_sitl"
    toolchain_file = source_dir / "cmake" / "toolchains" / "linux-gcc.cmake"

    configure_cmd = [
        "cmake",
        "-S",
        str(source_dir),
        "-B",
        str(build_dir),
        "-G",
        "Unix Makefiles",
        f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}",
        "-DPLATFORM_TARGET_HW=OFF",
        "-DPLATFORM_TARGET_SIM=ON",
    ]
    build_cmd = [
        "cmake",
        "--build",
        str(build_dir),
        "--target",
        target,
        "-j4",
    ]

    rc = _run(configure_cmd, cwd=repo_root)
    if rc != 0:
        return rc
    return _run(build_cmd, cwd=repo_root)


def _run_sim(scenario: str) -> int:
    summary = {
        "sim_mode": "sitl",
        "requested_scenario": scenario,
        "status": "not_yet-integrated",
        "next_steps": [
            "python3 -m robot_platform.tools.platform_cli.main build sitl",
            "./build/robot_platform_sitl/balance_chassis_sitl",
            "python3 -m robot_platform.sim.bridge.sim_bridge",
        ],
    }
    print(json.dumps(summary, indent=2, ensure_ascii=False))
    return 2


def main() -> int:
    args = sys.argv[1:]
    if not args:
        print("robot_platform CLI")
        print("supported commands: generate build flash debug replay sim test")
        return 0

    cmd = args[0]
    known = set(HELP)
    if cmd not in known:
        print(f"unknown command: {cmd}", file=sys.stderr)
        return 2

    if cmd == "generate":
        project = args[1] if len(args) > 1 else "balance_chassis"
        if project != "balance_chassis":
            print(f"unsupported project for now: {project}", file=sys.stderr)
            return 2
        return _generate_balance_chassis()

    if cmd == "build":
        mode = args[1] if len(args) > 1 else "hw_elf"
        supported = {
            "hw_elf": "balance_chassis_hw_seed.elf",
            "hw_seed": "balance_chassis_bsp_seed",
            "legacy_obj": "balance_chassis_legacy_full_obj",
            "legacy_full": "balance_chassis_legacy_full.elf",
            "sitl": "balance_chassis_sitl",
        }
        if mode not in supported:
            print(f"unsupported build mode for now: {mode}", file=sys.stderr)
            return 2
        if mode == "sitl":
            return _build_sitl(supported[mode])
        return _build_hw_seed(supported[mode])

    if cmd == "sim":
        scenario = args[1] if len(args) > 1 else "standstill"
        return _run_sim(scenario)

    print(f"command placeholder: {cmd}")
    print(HELP[cmd])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
