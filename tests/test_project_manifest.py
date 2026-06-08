from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from tools.project_manifest import validate_manifest


class ProjectManifestTest(unittest.TestCase):
    def test_validate_minimal_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            project = root / "project"
            scripts = root / "scripts"
            resources = root / "resource"
            data = root / "data"
            project.mkdir()
            scripts.mkdir()
            resources.mkdir()
            data.mkdir()
            (scripts / "preload.b64.sqasm").write_text("", encoding="utf-8")
            (scripts / "script" / "menu").mkdir(parents=True)
            (scripts / "script" / "menu" / "titlemenu.b64.sqasm").write_text("", encoding="utf-8")
            (resources / "menu" / "title").mkdir(parents=True)
            (resources / "menu" / "title" / "title_back_en.dds").write_bytes(b"dds")
            (data / "info.db3").write_bytes(b"sqlite")
            pack_manifest = project / "rpack_manifest.json"
            pack_manifest.write_text(json.dumps([{"path": "hud/titlemenu/title_demo.dds"}]), encoding="utf-8")

            manifest = {
                "schema": "YuEngine.Project/0.1",
                "project_id": "sample",
                "display_name": "Sample",
                "role": "sample_oracle",
                "engine_profile": "test",
                "language": {"default": "en", "supported": ["en"]},
                "mounts": [
                    {"type": "loose", "path": "../resource"},
                    {"type": "pack_manifest", "path": "rpack_manifest.json"},
                ],
                "script_roots": [{"type": "sqasm", "path": "../scripts"}],
                "startup": {
                    "preload_scripts": ["preload.b64"],
                    "entry_module": "script/menu/titlemenu.b64",
                    "entry_function": "setupProc",
                },
                "data": {"info_db": "../data/info.db3"},
                "runtime": {
                    "renderer": "d3d9_compatible",
                    "script_vm": "squirrel_2_2_4",
                    "audio": "none",
                    "save_policy": "local_profile",
                },
                "resources": {
                    "required": [
                        {"kind": "stem", "path": "menu/title/title_back"},
                        {"kind": "path", "path": "hud/titlemenu/title_demo.dds"},
                    ]
                },
            }
            manifest_path = project / "project.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

            result = validate_manifest(manifest_path)

        self.assertTrue(result.ok, result.errors)
        self.assertIn("entry_module", result.resolved)


if __name__ == "__main__":
    unittest.main()
