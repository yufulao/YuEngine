from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from tools.oracle_title_boot import build_readiness, build_snapshot, inspect_game


class OracleTitleBootTest(unittest.TestCase):
    def test_inspect_and_snapshot_minimal_install(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "bin").mkdir()
            (root / "resource" / "data").mkdir(parents=True)
            (root / "bin" / "game.exe").write_bytes(b"MZ")
            (root / "bin" / "ConfigTool.exe").write_bytes(b"MZ")
            (root / "bin" / "steam_api64.dll").write_bytes(b"MZ")
            (root / "resource" / "ak3.json").write_text("{}", encoding="utf-8")
            (root / "resource" / "data" / "info.db3").write_bytes(b"sqlite")
            (root / "resource" / "rpack01.dat").write_bytes(b"pack")

            inspection = inspect_game(root)
            snapshot = build_snapshot(root)
            readiness = build_readiness(root)

        self.assertTrue(inspection["game_exe_exists"])
        self.assertTrue(inspection["steam_api_exists"])
        self.assertEqual(len(inspection["pack_files"]), 1)
        existing = [row for row in snapshot["install_files"] if row["exists"]]
        self.assertGreaterEqual(len(existing), 5)
        self.assertTrue(readiness["can_launch_original_exe"])
        self.assertFalse(readiness["safe_to_bypass_platform"])
        self.assertIn("no user-driven title boot run has been recorded in this workspace", readiness["blockers"])


if __name__ == "__main__":
    unittest.main()
