"""CubeMX backend for phase-0 code generation."""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path


def find_cubemx() -> str | None:
    env = os.environ.get("STM32CUBEMX_BIN")
    if env:
        return env
    local_default = "/home/xbd/.local/stm32cubemx/6.17.0/STM32CubeMX"
    if Path(local_default).exists():
        return local_default
    for candidate in ("STM32CubeMX", "stm32cubemx"):
        path = shutil.which(candidate)
        if path:
            return path
    return None


def write_codegen_script(script_path: Path, ioc_path: Path, out_dir: Path) -> None:
    script_path.write_text(
        "\n".join(
            [
                f'config load "{ioc_path}"',
                f'generate code "{out_dir}"',
                "exit",
                "",
            ]
        )
    )


def run_codegen(ioc_path: Path, out_dir: Path) -> int:
    cubemx = find_cubemx()
    if not cubemx:
        print("CubeMX CLI not found. Set STM32CUBEMX_BIN or install STM32CubeMX.", file=sys.stderr)
        return 3

    ioc_path = ioc_path.resolve()
    out_dir = out_dir.resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    script_dir = Path("/tmp/robot_platform_codegen")
    script_dir.mkdir(parents=True, exist_ok=True)
    script_path = script_dir / "generate_from_ioc.mxs"
    write_codegen_script(script_path, ioc_path, out_dir)

    env = os.environ.copy()
    env["JAVA_TOOL_OPTIONS"] = "-Duser.home=/home/xbd"

    print(f"CubeMX CLI: {cubemx}")
    print(f"IOC input: {ioc_path}")
    print(f"Output dir: {out_dir}")
    print(f"Script file: {script_path}")

    result = subprocess.run([cubemx, "-q", str(script_path)], env=env, check=False)
    return result.returncode


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: python -m robot_platform.tools.cubemx_backend.main <ioc_path> <output_dir>", file=sys.stderr)
        return 2

    ioc_path = Path(sys.argv[1]).resolve()
    out_dir = Path(sys.argv[2]).resolve()
    return run_codegen(ioc_path, out_dir)


if __name__ == "__main__":
    raise SystemExit(main())
