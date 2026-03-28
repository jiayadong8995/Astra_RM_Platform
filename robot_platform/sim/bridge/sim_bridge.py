from robot_platform.sim.backends.sitl_bridge import main as run_sitl_bridge


def main() -> int:
    return run_sitl_bridge(default_project="balance_chassis")


if __name__ == "__main__":
    raise SystemExit(main())
