"""Minimal CLI for the current robot_platform bootstrap stage."""

import json
import subprocess
import sys
from pathlib import Path

from robot_platform.sim.runner import run_sitl_session
from robot_platform.tools.cubemx_backend.main import run_codegen


HELP = {
    "generate": "Run STM32CubeMX CLI code generation into runtime/generated.",
    "build": "Configure and build the current hardware or SITL target.",
    "flash": "Flash hardware firmware target.",
    "debug": "Start hardware debug session.",
    "replay": "Run log replay.",
    "sim": "Run simulation scenario.",
    "test": "Unified regression entry.",
}


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def _run(cmd: list[str], cwd: Path) -> int:
    print(f"+ {' '.join(str(part) for part in cmd)}", flush=True)
    completed = subprocess.run(cmd, cwd=cwd)
    return completed.returncode


def _parse_sim_args(args: list[str]) -> tuple[str, float, bool]:
    scenario = "sitl"
    duration_s = 3.0
    skip_build = False

    i = 0
    while i < len(args):
        arg = args[i]
        if arg == "--duration":
            if i + 1 >= len(args):
                raise ValueError("missing value for --duration")
            try:
                duration_s = float(args[i + 1])
            except ValueError as exc:
                raise ValueError("invalid value for --duration") from exc
            if duration_s <= 0.0:
                raise ValueError("--duration must be positive")
            i += 2
            continue
        if arg == "--skip-build":
            skip_build = True
            i += 1
            continue
        if arg.startswith("--"):
            raise ValueError(f"unknown sim option: {arg}")
        if scenario != "sitl":
            raise ValueError(f"unexpected extra sim argument: {arg}")
        scenario = arg
        i += 1

    return scenario, duration_s, skip_build


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
    build_dir = repo_root / "build" / "robot_platform_sitl_make"
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


def _run_sim(scenario: str, *, duration_s: float = 3.0, skip_build: bool = False) -> int:
    if scenario != "sitl":
        summary = {
            "sim_mode": "sitl",
            "requested_scenario": scenario,
            "status": "unsupported_scenario",
            "supported_scenarios": ["sitl"],
        }
        print(json.dumps(summary, indent=2, ensure_ascii=False))
        return 2

    if not skip_build:
        rc = _build_sitl("balance_chassis_sitl")
        if rc != 0:
            return rc
    return run_sitl_session(repo_root=_repo_root(), duration_s=duration_s)


def _run_tests(scope: str) -> int:
    repo_root = _repo_root()
    if scope != "sim":
        print(f"unsupported test scope for now: {scope}", file=sys.stderr)
        return 2
    return _run(
        [
            sys.executable,
            "-m",
            "unittest",
            "robot_platform.sim.tests.test_runner",
            "robot_platform.tools.platform_cli.tests.test_main",
            "-v",
        ],
        cwd=repo_root,
    )


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
            "sitl": "balance_chassis_sitl",
        }
        if mode not in supported:
            print(f"unsupported build mode for now: {mode}", file=sys.stderr)
            return 2
        if mode == "sitl":
            return _build_sitl(supported[mode])
        return _build_hw_seed(supported[mode])

    if cmd == "sim":
        try:
            scenario, duration_s, skip_build = _parse_sim_args(args[1:])
        except ValueError as exc:
            print(str(exc), file=sys.stderr)
            return 2
        return _run_sim(scenario, duration_s=duration_s, skip_build=skip_build)

    if cmd == "test":
        scope = args[1] if len(args) > 1 else "sim"
        return _run_tests(scope)

    print(f"command placeholder: {cmd}")
    print(HELP[cmd])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
