from __future__ import annotations

import unittest

from tools.api_surface import SERVICE_MODULES


class ApiSurfaceTest(unittest.TestCase):
    def test_key_services_have_module_names(self) -> None:
        for service in [
            "Save/Profile/Scenario Service",
            "Scene And Stage Service",
            "Actor And Task Service",
            "Camera Service",
            "UI And 2D Render Service",
        ]:
            with self.subTest(service=service):
                self.assertIn(service, SERVICE_MODULES)
                self.assertTrue(SERVICE_MODULES[service].startswith("engine/"))


if __name__ == "__main__":
    unittest.main()
