#!/usr/bin/env python3
"""
Validate YuEngine project manifests.

This is the P5 baseline: it proves a project can be described without hard-coding the original
game into runtime code.
"""

from __future__ import annotations

import argparse
import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


SCHEMA = "YuEngine.Project/0.1"
ALLOWED_MOUNT_TYPES = {"loose", "pack_manifest"}
ALLOWED_SCRIPT_ROOT_TYPES = {"sqasm"}
REQUIRED_TOP_LEVEL = {
    "schema",
    "project_id",
    "display_name",
    "role",
    "engine_profile",
    "language",
    "mounts",
    "script_roots",
    "startup",
    "data",
    "runtime",
}


@dataclass
class ValidationResult:
    manifest: str
    ok: bool = True
    errors: list[str] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)
    resolved: dict[str, Any] = field(default_factory=dict)

    def error(self, message: str) -> None:
        self.ok = False
        self.errors.append(message)

    def warn(self, message: str) -> None:
        self.warnings.append(message)

    def to_json(self) -> dict[str, Any]:
        return {
            "schema": "yuengine.project_manifest.validation.v1",
            "manifest": self.manifest,
            "ok": self.ok,
            "errors": self.errors,
            "warnings": self.warnings,
            "resolved": self.resolved,
        }


def norm_resource(value: str) -> str:
    return value.replace("\\", "/").strip().lower()


def load_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def resolve_project_path(project_root: Path, value: str) -> Path:
    return (project_root / value).resolve()


def validate_language(manifest: dict[str, Any], result: ValidationResult) -> None:
    language = manifest.get("language")
    if not isinstance(language, dict):
        result.error("language must be an object")
        return
    default = language.get("default")
    supported = language.get("supported")
    if not isinstance(default, str) or not default:
        result.error("language.default must be a non-empty string")
    if not isinstance(supported, list) or not all(isinstance(item, str) for item in supported):
        result.error("language.supported must be a string array")
    elif default not in supported:
        result.error("language.default must be present in language.supported")


def validate_mounts(project_root: Path, manifest: dict[str, Any], result: ValidationResult) -> list[dict[str, Any]]:
    mounts = manifest.get("mounts")
    resolved_mounts: list[dict[str, Any]] = []
    if not isinstance(mounts, list) or not mounts:
        result.error("mounts must be a non-empty array")
        return resolved_mounts

    for index, mount in enumerate(mounts):
        if not isinstance(mount, dict):
            result.error(f"mounts[{index}] must be an object")
            continue
        mount_type = mount.get("type")
        path_value = mount.get("path")
        if mount_type not in ALLOWED_MOUNT_TYPES:
            result.error(f"mounts[{index}].type unsupported: {mount_type}")
        if not isinstance(path_value, str) or not path_value:
            result.error(f"mounts[{index}].path must be a non-empty string")
            continue
        path = resolve_project_path(project_root, path_value)
        exists = path.exists()
        if not exists:
            result.error(f"mounts[{index}].path does not exist: {path}")
        if mount_type == "loose" and exists and not path.is_dir():
            result.error(f"mounts[{index}] loose path must be a directory: {path}")
        if mount_type == "pack_manifest" and exists and not path.is_file():
            result.error(f"mounts[{index}] pack_manifest path must be a file: {path}")
        resolved_mounts.append({"index": index, "id": mount.get("id"), "type": mount_type, "path": str(path)})
    return resolved_mounts


def validate_script_roots(project_root: Path, manifest: dict[str, Any], result: ValidationResult) -> list[dict[str, Any]]:
    roots = manifest.get("script_roots")
    resolved_roots: list[dict[str, Any]] = []
    if not isinstance(roots, list) or not roots:
        result.error("script_roots must be a non-empty array")
        return resolved_roots
    for index, root in enumerate(roots):
        if not isinstance(root, dict):
            result.error(f"script_roots[{index}] must be an object")
            continue
        root_type = root.get("type")
        path_value = root.get("path")
        if root_type not in ALLOWED_SCRIPT_ROOT_TYPES:
            result.error(f"script_roots[{index}].type unsupported: {root_type}")
        if not isinstance(path_value, str) or not path_value:
            result.error(f"script_roots[{index}].path must be a non-empty string")
            continue
        path = resolve_project_path(project_root, path_value)
        if not path.exists() or not path.is_dir():
            result.error(f"script_roots[{index}].path must exist as directory: {path}")
        resolved_roots.append({"index": index, "type": root_type, "path": str(path)})
    return resolved_roots


def script_candidates(root: Path, module: str) -> list[Path]:
    normalized = module.replace("\\", "/")
    candidates = [root / normalized]
    if not normalized.endswith(".sqasm"):
        candidates.append(root / (normalized + ".sqasm"))
    if normalized.endswith(".b64"):
        candidates.append(root / (normalized + ".sqasm"))
    return candidates


def resolve_script(script_roots: list[dict[str, Any]], module: str) -> Path | None:
    for root_info in script_roots:
        root = Path(root_info["path"])
        for candidate in script_candidates(root, module):
            if candidate.exists():
                return candidate.resolve()
    return None


def validate_startup(manifest: dict[str, Any], script_roots: list[dict[str, Any]], result: ValidationResult) -> None:
    startup = manifest.get("startup")
    if not isinstance(startup, dict):
        result.error("startup must be an object")
        return
    entry_module = startup.get("entry_module")
    entry_function = startup.get("entry_function")
    preload_scripts = startup.get("preload_scripts", [])
    if not isinstance(entry_module, str) or not entry_module:
        result.error("startup.entry_module must be a non-empty string")
    if not isinstance(entry_function, str) or not entry_function:
        result.error("startup.entry_function must be a non-empty string")
    if not isinstance(preload_scripts, list) or not all(isinstance(item, str) for item in preload_scripts):
        result.error("startup.preload_scripts must be a string array")

    resolved_preloads = []
    for script in preload_scripts if isinstance(preload_scripts, list) else []:
        resolved = resolve_script(script_roots, script)
        if resolved is None:
            result.error(f"startup preload script not found: {script}")
        else:
            resolved_preloads.append(str(resolved))

    if isinstance(entry_module, str):
        resolved_entry = resolve_script(script_roots, entry_module)
        if resolved_entry is None:
            result.error(f"startup.entry_module not found: {entry_module}")
        else:
            result.resolved["entry_module"] = str(resolved_entry)
    result.resolved["preload_scripts"] = resolved_preloads


def pack_manifest_resources(path: Path) -> set[str]:
    data = load_json(path)
    resources = set()
    if isinstance(data, list):
        for item in data:
            if isinstance(item, dict) and isinstance(item.get("path"), str):
                resources.add(norm_resource(item["path"]))
    return resources


def build_resource_index(resolved_mounts: list[dict[str, Any]]) -> dict[str, Any]:
    loose_roots: list[Path] = []
    pack_resources: set[str] = set()
    for mount in resolved_mounts:
        path = Path(mount["path"])
        if mount["type"] == "loose" and path.exists():
            loose_roots.append(path)
        if mount["type"] == "pack_manifest" and path.exists():
            pack_resources.update(pack_manifest_resources(path))
    return {"loose_roots": loose_roots, "pack_resources": pack_resources}


def resource_exists(index: dict[str, Any], resource_path: str) -> bool:
    normalized = norm_resource(resource_path)
    if normalized in index["pack_resources"]:
        return True
    for root in index["loose_roots"]:
        if (root / resource_path).exists():
            return True
    return False


def resource_stem_exists(index: dict[str, Any], resource_stem: str) -> bool:
    normalized = norm_resource(resource_stem)
    if any(path.startswith(normalized) for path in index["pack_resources"]):
        return True
    for root in index["loose_roots"]:
        parent = root / resource_stem
        if parent.exists():
            return True
        candidates = list((root / Path(resource_stem).parent).glob(Path(resource_stem).name + "*"))
        if candidates:
            return True
    return False


def validate_resources(manifest: dict[str, Any], index: dict[str, Any], result: ValidationResult) -> None:
    resources = manifest.get("resources", {})
    if not isinstance(resources, dict):
        result.error("resources must be an object when present")
        return
    required = resources.get("required", [])
    if not isinstance(required, list):
        result.error("resources.required must be an array")
        return

    resolved_required = []
    for idx, item in enumerate(required):
        if not isinstance(item, dict):
            result.error(f"resources.required[{idx}] must be an object")
            continue
        kind = item.get("kind")
        value = item.get("path")
        if kind not in {"path", "stem"}:
            result.error(f"resources.required[{idx}].kind unsupported: {kind}")
            continue
        if not isinstance(value, str) or not value:
            result.error(f"resources.required[{idx}].path must be a non-empty string")
            continue
        ok = resource_exists(index, value) if kind == "path" else resource_stem_exists(index, value)
        if not ok:
            result.error(f"required resource {kind} not resolved: {value}")
        resolved_required.append({"kind": kind, "path": value, "resolved": ok})
    result.resolved["required_resources"] = resolved_required


def validate_data(project_root: Path, manifest: dict[str, Any], result: ValidationResult) -> None:
    data = manifest.get("data")
    if not isinstance(data, dict):
        result.error("data must be an object")
        return
    resolved = {}
    for key, value in data.items():
        if value is None:
            continue
        if not isinstance(value, str):
            result.error(f"data.{key} must be a string path or null")
            continue
        path = resolve_project_path(project_root, value)
        if not path.exists():
            result.error(f"data.{key} path does not exist: {path}")
        resolved[key] = str(path)
    result.resolved["data"] = resolved


def validate_runtime(manifest: dict[str, Any], result: ValidationResult) -> None:
    runtime = manifest.get("runtime")
    if not isinstance(runtime, dict):
        result.error("runtime must be an object")
        return
    for key in ["renderer", "script_vm", "audio", "save_policy"]:
        if not isinstance(runtime.get(key), str) or not runtime.get(key):
            result.error(f"runtime.{key} must be a non-empty string")


def validate_manifest(path: Path) -> ValidationResult:
    path = path.resolve()
    manifest = load_json(path)
    result = ValidationResult(manifest=str(path))
    if not isinstance(manifest, dict):
        result.error("manifest root must be an object")
        return result

    missing = sorted(REQUIRED_TOP_LEVEL - set(manifest))
    for key in missing:
        result.error(f"missing top-level field: {key}")
    if manifest.get("schema") != SCHEMA:
        result.error(f"schema must be {SCHEMA}")

    project_root = path.parent
    validate_language(manifest, result)
    resolved_mounts = validate_mounts(project_root, manifest, result)
    script_roots = validate_script_roots(project_root, manifest, result)
    validate_startup(manifest, script_roots, result)
    validate_data(project_root, manifest, result)
    validate_runtime(manifest, result)
    validate_resources(manifest, build_resource_index(resolved_mounts), result)
    result.resolved["mounts"] = resolved_mounts
    result.resolved["script_roots"] = script_roots
    result.resolved["project_id"] = manifest.get("project_id")
    return result


def write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def cmd_validate(args: argparse.Namespace) -> int:
    result = validate_manifest(args.manifest)
    value = result.to_json()
    if args.json_out:
        write_json(args.json_out, value)
    if args.stdout or not args.json_out:
        print(json.dumps(value, indent=2, ensure_ascii=False))
    else:
        print(
            "project-manifest: "
            f"{Path(result.manifest).name} -> "
            f"{'ok' if result.ok else 'failed'} "
            f"({len(result.errors)} errors, {len(result.warnings)} warnings)"
        )
    return 0 if result.ok else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    validate = sub.add_parser("validate")
    validate.add_argument("manifest", type=Path)
    validate.add_argument("--json-out", type=Path)
    validate.add_argument("--stdout", action="store_true")
    validate.set_defaults(func=cmd_validate)
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
