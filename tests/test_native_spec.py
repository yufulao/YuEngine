from __future__ import annotations

import unittest

from tools.native_spec import owner_for


class NativeSpecTest(unittest.TestCase):
    def test_owner_rules_cover_key_title_and_mission_boundaries(self) -> None:
        expected = {
            "GetSaveList": "Save/Profile/Scenario Service",
            "StartGame": "Save/Profile/Scenario Service",
            "LoadStage": "Scene And Stage Service",
            "PushPlayerChara": "Actor And Task Service",
            "PushTaskGameCamera": "Camera Service",
            "GetFlag": "Event/Quest/Flag Service",
            "PlayBGM": "Audio Service",
            "GraphString": "UI And 2D Render Service",
        }
        for name, service in expected.items():
            with self.subTest(name=name):
                owner, level, _reason = owner_for(name)
                self.assertEqual(owner, service)
                self.assertEqual(level, "proposed_owner")


if __name__ == "__main__":
    unittest.main()
