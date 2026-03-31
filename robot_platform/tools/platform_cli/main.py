"""Minimal CLI for the current robot_platform bootstrap stage."""

import json
import subprocess
import sys
import time
from pathlib import Path

from robot_platform.sim.projects import get_project_names, get_project_profile, get_project_smoke_runner
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
    "verify": "Run report-driven verification flows.",
}


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def _run(cmd: list[str], cwd: Path) -> int:
    print(f"+ {' '.join(str(part) for part in cmd)}", flush=True)
    completed = subprocess.run(cmd, cwd=cwd)
    return completed.returncode


def _load_json_report(path: Path) -> dict[str, object] | None:
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None


def _parse_sim_args(args: list[str]) -> tuple[str, str, float, bool]:
    project = "balance_chassis"
    scenario = "sitl"
    duration_s = 3.0
    skip_build = False

    i = 0
    while i < len(args):
        arg = args[i]
        if arg == "--project":
            if i + 1 >= len(args):
                raise ValueError("missing value for --project")
            project = args[i + 1]
            i += 2
            continue
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

    return project, scenario, duration_s, skip_build


def _parse_verify_phase1_args(args: list[str]) -> tuple[str, Path, float]:
    if not args or args[0] != "phase1":
        raise ValueError("verify currently supports only `phase1`")

    project = "balance_chassis"
    report = Path("build/verification_reports/phase1_balance_chassis.json")
    smoke_duration = 1.0
    i = 1
    while i < len(args):
        arg = args[i]
        if arg == "--project":
            if i + 1 >= len(args):
                raise ValueError("missing value for --project")
            project = args[i + 1]
            i += 2
            continue
        if arg == "--report":
            if i + 1 >= len(args):
                raise ValueError("missing value for --report")
            report = Path(args[i + 1])
            i += 2
            continue
        if arg == "--smoke-duration":
            if i + 1 >= len(args):
                raise ValueError("missing value for --smoke-duration")
            try:
                smoke_duration = float(args[i + 1])
            except ValueError as exc:
                raise ValueError("invalid value for --smoke-duration") from exc
            if smoke_duration <= 0.0:
                raise ValueError("--smoke-duration must be positive")
            i += 2
            continue
        raise ValueError(f"unknown verify option: {arg}")

    return project, report, smoke_duration


def _generate_balance_chassis() -> int:
    repo_root = _repo_root()
    ioc_path = repo_root / "references" / "legacy" / "Astra_RM2025_Balance_legacy" / "Chassis" / "CtrlBoard-H7_IMU.ioc"
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


def _run_host_message_center_tests() -> int:
    repo_root = _repo_root()
    source_dir = repo_root / "robot_platform"
    build_dir = repo_root / "build" / "robot_platform_host_tests"
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
        "-DPLATFORM_TARGET_SIM=OFF",
        "-DPLATFORM_HOST_TESTS=ON",
        "-DPLATFORM_HOST_TEST_SANITIZERS=ON",
    ]
    build_cmd = [
        "cmake",
        "--build",
        str(build_dir),
        "--target",
        "test_message_center",
        "-j4",
    ]
    test_cmd = [
        "ctest",
        "--test-dir",
        str(build_dir),
        "--output-on-failure",
        "-R",
        "test_message_center",
    ]

    rc = _run(configure_cmd, cwd=repo_root)
    if rc != 0:
        return rc
    rc = _run(build_cmd, cwd=repo_root)
    if rc != 0:
        return rc
    return _run(test_cmd, cwd=repo_root)


def _run_stage(stage_name: str, runner) -> dict[str, object]:
    started = time.monotonic()
    rc = runner()
    duration_s = round(time.monotonic() - started, 3)
    return {
        "name": stage_name,
        "status": "passed" if rc == 0 else "failed",
        "exit_code": rc,
        "duration_s": duration_s,
    }


def _smoke_stage_status(smoke_report: dict[str, object] | None) -> tuple[str, str | None]:
    if not isinstance(smoke_report, dict):
        return "failed", "smoke_report_missing"

    startup_error = smoke_report.get("bridge_startup_error")
    if isinstance(startup_error, dict):
        message = startup_error.get("message")
        if isinstance(message, str) and ("Operation not permitted" in message or "Permission denied" in message):
            return "blocked", message

    smoke_result = smoke_report.get("smoke_result")
    if isinstance(smoke_result, dict):
        validation = smoke_result.get("validation")
        if isinstance(validation, dict):
            required = int(validation.get("required_count", 0))
            observed = int(validation.get("observed_count", 0))
            if required > 0 and observed < required:
                return "failed", f"runtime_output_not_observed:{observed}/{required}"
        if not bool(smoke_result.get("passed", False)):
            detail = smoke_result.get("failure_detail") or smoke_result.get("primary_failure")
            return "failed", str(detail) if detail is not None else "smoke_failed"

    return "passed", None


def _run_verify_phase1(project: str, report_path: Path, smoke_duration: float) -> int:
    repo_root = _repo_root()
    profile = get_project_profile(project)
    if profile is None:
        print(f"missing profile for project: {project}", file=sys.stderr)
        return 2

    resolved_report_path = report_path if report_path.is_absolute() else repo_root / report_path
    stages: list[dict[str, object]] = []

    build_stage = _run_stage("build_sitl", lambda: _build_sitl(profile.sitl_target))
    stages.append(build_stage)
    if build_stage["exit_code"] != 0:
        payload = {
            "verification_run_version": 1,
            "project": project,
            "overall_status": "failed",
            "failure_stage": "build_sitl",
            "failure_reason": "build_failed",
            "stages": stages,
        }
        resolved_report_path.parent.mkdir(parents=True, exist_ok=True)
        resolved_report_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"verification summary: project={project} overall_status=failed failure_stage=build_sitl")
        print(f"verification report: {resolved_report_path}")
        return 1

    host_stage = _run_stage("host_tests", _run_host_message_center_tests)
    stages.append(host_stage)
    if host_stage["exit_code"] != 0:
        payload = {
            "verification_run_version": 1,
            "project": project,
            "overall_status": "failed",
            "failure_stage": "host_tests",
            "failure_reason": "host_tests_failed",
            "stages": stages,
        }
        resolved_report_path.parent.mkdir(parents=True, exist_ok=True)
        resolved_report_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"verification summary: project={project} overall_status=failed failure_stage=host_tests")
        print(f"verification report: {resolved_report_path}")
        return 1

    smoke_report_path = repo_root / "build" / "sim_reports" / profile.report_name
    smoke_started = time.monotonic()
    smoke_rc = _run_sim(project, "sitl", duration_s=smoke_duration, skip_build=True)
    smoke_duration_s = round(time.monotonic() - smoke_started, 3)
    smoke_report = _load_json_report(smoke_report_path)
    smoke_status, smoke_failure = _smoke_stage_status(smoke_report)
    smoke_stage: dict[str, object] = {
        "name": "smoke",
        "status": smoke_status,
        "exit_code": smoke_rc,
        "duration_s": smoke_duration_s,
        "observed_outputs": [],
    }
    if isinstance(smoke_report, dict):
        observations = smoke_report.get("runtime_output_observations")
        if isinstance(observations, list):
            smoke_stage["observed_outputs"] = observations
        smoke_stage["smoke_report"] = str(smoke_report_path)
    if smoke_failure is not None:
        smoke_stage["failure_reason"] = smoke_failure
    stages.append(smoke_stage)

    overall_status = "passed"
    failure_stage = None
    failure_reason = None
    if smoke_status == "blocked":
        overall_status = "blocked"
        failure_stage = "smoke"
        failure_reason = smoke_failure
    elif smoke_status != "passed":
        overall_status = "failed"
        failure_stage = "smoke"
        failure_reason = smoke_failure or "smoke_failed"

    payload: dict[str, object] = {
        "verification_run_version": 1,
        "project": project,
        "overall_status": overall_status,
        "stages": stages,
    }
    if failure_stage is not None:
        payload["failure_stage"] = failure_stage
    if failure_reason is not None:
        payload["failure_reason"] = failure_reason

    resolved_report_path.parent.mkdir(parents=True, exist_ok=True)
    resolved_report_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(
        f"verification summary: project={project} overall_status={overall_status} "
        f"failure_stage={failure_stage}"
    )
    print(f"verification report: {resolved_report_path}")
    return 0 if overall_status == "passed" else 1


def _run_sim(project: str, scenario: str, *, duration_s: float = 3.0, skip_build: bool = False) -> int:
    supported_projects = list(get_project_names())
    if project not in supported_projects:
        summary = {
            "sim_mode": "sitl",
            "requested_project": project,
            "status": "unsupported_project",
            "supported_projects": supported_projects,
        }
        print(json.dumps(summary, indent=2, ensure_ascii=False))
        return 2

    if scenario != "sitl":
        summary = {
            "sim_mode": "sitl",
            "requested_project": project,
            "requested_scenario": scenario,
            "status": "unsupported_scenario",
            "supported_scenarios": ["sitl"],
        }
        print(json.dumps(summary, indent=2, ensure_ascii=False))
        return 2

    profile = get_project_profile(project)
    if profile is None:
        print(f"missing profile for project: {project}", file=sys.stderr)
        return 2

    if not skip_build:
        rc = _build_sitl(profile.sitl_target)
        if rc != 0:
            return rc

    smoke_runner = get_project_smoke_runner(project)
    if smoke_runner is None:
        print(f"missing smoke runner for project: {project}", file=sys.stderr)
        return 2

    return smoke_runner(repo_root=_repo_root(), duration_s=duration_s)


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
        print("supported commands: generate build flash debug replay sim test verify")
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
            project, scenario, duration_s, skip_build = _parse_sim_args(args[1:])
        except ValueError as exc:
            print(str(exc), file=sys.stderr)
            return 2
        return _run_sim(project, scenario, duration_s=duration_s, skip_build=skip_build)

    if cmd == "test":
        scope = args[1] if len(args) > 1 else "sim"
        return _run_tests(scope)

    if cmd == "verify":
        try:
            project, report_path, smoke_duration = _parse_verify_phase1_args(args[1:])
        except ValueError as exc:
            print(str(exc), file=sys.stderr)
            return 2
        return _run_verify_phase1(project, report_path, smoke_duration)

    print(f"command placeholder: {cmd}")
    print(HELP[cmd])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
