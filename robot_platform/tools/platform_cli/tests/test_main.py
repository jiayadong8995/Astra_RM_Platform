from __future__ import annotations

import unittest

from robot_platform.tools.platform_cli.main import _parse_sim_args


class ParseSimArgsTests(unittest.TestCase):
    def test_defaults(self) -> None:
        self.assertEqual(_parse_sim_args([]), ("sitl", 3.0, False))

    def test_duration_and_skip_build(self) -> None:
        self.assertEqual(_parse_sim_args(["--duration", "1.5", "--skip-build"]), ("sitl", 1.5, True))

    def test_explicit_scenario(self) -> None:
        self.assertEqual(_parse_sim_args(["sitl", "--duration", "5"]), ("sitl", 5.0, False))

    def test_rejects_unknown_option(self) -> None:
        with self.assertRaisesRegex(ValueError, "unknown sim option"):
            _parse_sim_args(["--unknown"])

    def test_rejects_invalid_duration(self) -> None:
        with self.assertRaisesRegex(ValueError, "invalid value for --duration"):
            _parse_sim_args(["--duration", "abc"])

    def test_rejects_non_positive_duration(self) -> None:
        with self.assertRaisesRegex(ValueError, "--duration must be positive"):
            _parse_sim_args(["--duration", "0"])


if __name__ == "__main__":
    unittest.main()
