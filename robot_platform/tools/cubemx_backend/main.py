"""CubeMX backend for phase-0 code generation."""

from __future__ import annotations

import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path


FRESHNESS_MANIFEST = "freshness_manifest.json"


def _sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _iter_generated_files(out_dir: Path) -> list[Path]:
    manifest_path = out_dir / FRESHNESS_MANIFEST
    files = [path for path in out_dir.rglob("*") if path.is_file() and path != manifest_path]
    files.sort(key=lambda path: path.relative_to(out_dir).as_posix())
    return files


def compute_generated_tree_sha256(out_dir: Path) -> str:
    entries: list[str] = []
    for path in _iter_generated_files(out_dir):
        rel = path.relative_to(out_dir).as_posix()
        entries.append(f"{rel}:{_sha256_file(path)}")
    return _sha256_bytes("\n".join(entries).encode("utf-8"))


def build_freshness_manifest(ioc_path: Path, out_dir: Path) -> dict[str, object]:
    resolved_ioc = ioc_path.resolve()
    resolved_out_dir = out_dir.resolve()
    generated_files = _iter_generated_files(resolved_out_dir)
    return {
        "manifest_version": 1,
        "source": {
            "path": str(resolved_ioc),
            "sha256": _sha256_file(resolved_ioc),
        },
        "generated": {
            "path": str(resolved_out_dir),
            "tree_sha256": compute_generated_tree_sha256(resolved_out_dir),
            "file_count": len(generated_files),
        },
    }


def write_freshness_manifest(ioc_path: Path, out_dir: Path) -> Path:
    manifest_path = out_dir.resolve() / FRESHNESS_MANIFEST
    payload = build_freshness_manifest(ioc_path, out_dir)
    manifest_path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return manifest_path


def load_freshness_manifest(out_dir: Path) -> dict[str, object] | None:
    manifest_path = out_dir.resolve() / FRESHNESS_MANIFEST
    if not manifest_path.exists():
        return None
    try:
        payload = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    return payload if isinstance(payload, dict) else None


def generated_artifacts_are_fresh(ioc_path: Path, out_dir: Path) -> tuple[bool, str, dict[str, object] | None]:
    manifest = load_freshness_manifest(out_dir)
    if manifest is None:
        return False, "missing_metadata", None

    expected = build_freshness_manifest(ioc_path, out_dir)
    source = manifest.get("source")
    generated = manifest.get("generated")
    if not isinstance(source, dict) or not isinstance(generated, dict):
        return False, "missing_metadata", manifest

    if source.get("path") != expected["source"]["path"]:
        return False, "stale_generated_artifacts", manifest
    if source.get("sha256") != expected["source"]["sha256"]:
        return False, "stale_generated_artifacts", manifest
    if generated.get("path") != expected["generated"]["path"]:
        return False, "stale_generated_artifacts", manifest
    if generated.get("tree_sha256") != expected["generated"]["tree_sha256"]:
        return False, "stale_generated_artifacts", manifest

    return True, "fresh", manifest


def find_cubemx() -> str | None:
    env = os.environ.get("STM32CUBEMX_BIN")
    if env:
        return env
    repo_local = Path(__file__).resolve().parents[3] / ".local_tools" / "stm32cubemx" / "6.17.0" / "STM32CubeMX"
    if repo_local.exists():
        return str(repo_local)
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


def write_runtime_ioc(source_ioc: Path, runtime_ioc: Path) -> None:
    text = source_ioc.read_text()
    # Prevent CubeMX from blocking CLI execution on a migration prompt for older IOC files.
    text = text.replace("ProjectManager.AskForMigrate=true", "ProjectManager.AskForMigrate=false")
    runtime_ioc.write_text(text)


def seed_updater_config(cubemx_home: Path, install_root: Path) -> None:
    updater_dir = cubemx_home / ".stm32cubemx" / "plugins" / "updater"
    updater_dir.mkdir(parents=True, exist_ok=True)
    repository_dir = cubemx_home / "STM32Cube" / "Repository"
    repository_dir.mkdir(parents=True, exist_ok=True)
    now_ms = int(time.time() * 1000)
    updater_ini = updater_dir / "updater.ini"
    updater_ini.write_text(
        "\n".join(
            [
                "[Data]",
                f"DataLastStamp={now_ms}",
                "",
                "[Path]",
                f"SoftwarePath={install_root}/",
                f"RepositoryPath={repository_dir}/",
                f"UpdaterPath={updater_dir}/",
                "",
                "[ReStart]",
                "StartResult=0",
                "SoftCopy=0",
                "",
                "[TimeDate]",
                "CheckType=0",
                f"LastCheckStamp={now_ms}",
                "IntervalDayCheck=9999",
                f"LastCheckConnectionStamp={now_ms}",
                "",
                "[Version]",
                "SoftType=0",
                "DbVersion=DB.6.0.170",
                "SoftVersion=MX.6.17.0",
                "",
                "[Proxy]",
                "Type=0",
                "Test=2",
                "Authentification=0",
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
    runtime_ioc_path = script_dir / ioc_path.name
    write_runtime_ioc(ioc_path, runtime_ioc_path)
    script_path = script_dir / "generate_from_ioc.mxs"
    write_codegen_script(script_path, runtime_ioc_path, out_dir)

    repo_root = Path(__file__).resolve().parents[3]
    cubemx_home = repo_root / ".cache" / "stm32_user_home"
    cubemx_home.mkdir(parents=True, exist_ok=True)
    seed_updater_config(cubemx_home, Path(cubemx).resolve().parent)

    env = os.environ.copy()
    env["HOME"] = str(cubemx_home)

    java_tool_options = [f"-Duser.home={cubemx_home}"]
    has_display = bool(env.get("DISPLAY") or env.get("WAYLAND_DISPLAY"))
    if not has_display:
        env["DISPLAY"] = ""
        java_tool_options.append("-Djava.awt.headless=true")

    env["JAVA_TOOL_OPTIONS"] = " ".join(java_tool_options)

    print(f"CubeMX CLI: {cubemx}")
    print(f"IOC input: {ioc_path}")
    print(f"Output dir: {out_dir}")
    print(f"Script file: {script_path}")

    result = subprocess.run([cubemx, "-q", str(script_path)], env=env, check=False)
    if result.returncode == 0:
        write_freshness_manifest(ioc_path, out_dir)
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
