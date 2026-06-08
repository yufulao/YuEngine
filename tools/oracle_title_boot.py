#!/usr/bin/env python3
"""
Prepare non-invasive oracle capture for original title boot.

Default commands inspect and snapshot local files only. The `launch` subcommand exists for an
explicit user-driven run and does not patch or bypass platform checks.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any


CAPTURE_TOOLS = [
    "procmon",
    "procmon64",
    "apitrace",
    "renderdoccmd",
    "pixwin",
    "procdump",
]
SMALL_HASH_LIMIT = 128 * 1024 * 1024


def default_game_root() -> Path:
    return Path(__file__).resolve().parents[2]


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def file_record(path: Path, root: Path, hash_large: bool = False) -> dict[str, Any]:
    stat = path.stat()
    should_hash = hash_large or stat.st_size <= SMALL_HASH_LIMIT
    return {
        "path": str(path),
        "relative_path": path.relative_to(root).as_posix() if path.is_relative_to(root) else str(path),
        "exists": True,
        "size": stat.st_size,
        "mtime": stat.st_mtime,
        "sha256": sha256_file(path) if should_hash else None,
        "sha256_skipped_reason": None if should_hash else "file larger than default hash limit",
    }


def missing_record(path: Path, root: Path) -> dict[str, Any]:
    return {
        "path": str(path),
        "relative_path": path.relative_to(root).as_posix() if path.is_relative_to(root) else str(path),
        "exists": False,
    }


def capture_tool_status() -> list[dict[str, Any]]:
    rows = []
    for tool in CAPTURE_TOOLS:
        found = shutil.which(tool)
        rows.append({"name": tool, "available": found is not None, "path": found})
    return rows


def candidate_state_dirs(game_root: Path) -> list[Path]:
    env = os.environ
    candidates = [
        game_root / "save",
        game_root / "bin" / "save",
        game_root / "config",
        game_root / "bin" / "config",
    ]
    userprofile = env.get("USERPROFILE")
    appdata = env.get("APPDATA")
    localappdata = env.get("LOCALAPPDATA")
    if userprofile:
        home = Path(userprofile)
        candidates.extend(
            [
                home / "Documents" / "My Games" / "Touhou New World",
                home / "Documents" / "Touhou New World",
                home / "Saved Games" / "Touhou New World",
            ]
        )
    if appdata:
        root = Path(appdata)
        candidates.extend(
            [
                root / "AnkakeSpa" / "Touhou New World",
                root / "Ankake Spa" / "Touhou New World",
                root / "Touhou New World",
                root / "TouhouNewWorld",
            ]
        )
    if localappdata:
        root = Path(localappdata)
        candidates.extend(
            [
                root / "AnkakeSpa" / "Touhou New World",
                root / "Ankake Spa" / "Touhou New World",
                root / "Touhou New World",
                root / "TouhouNewWorld",
            ]
        )
    deduped: list[Path] = []
    seen: set[str] = set()
    for path in candidates:
        key = str(path).lower()
        if key in seen:
            continue
        seen.add(key)
        deduped.append(path)
    return deduped


def critical_install_files(game_root: Path) -> list[Path]:
    files = [
        game_root / "bin" / "game.exe",
        game_root / "bin" / "ConfigTool.exe",
        game_root / "bin" / "AnkakeConfig.dll",
        game_root / "bin" / "steam_api64.dll",
        game_root / "resource" / "ak3.json",
        game_root / "resource" / "dialogs.json",
        game_root / "resource" / "fxData.json",
        game_root / "resource" / "data" / "info.db3",
    ]
    files.extend(sorted((game_root / "resource").glob("*.pak")))
    files.extend(sorted((game_root / "resource").glob("*.dat")))
    files.extend(sorted((game_root / "resource").glob("*.fx")))
    files.extend(sorted((game_root / "resource").glob("*.h")))
    return files


def inspect_game(game_root: Path) -> dict[str, Any]:
    game_root = game_root.resolve()
    bin_dir = game_root / "bin"
    resource_dir = game_root / "resource"
    pack_files = sorted(resource_dir.glob("*.pak")) + sorted(resource_dir.glob("*.dat"))
    state_dirs = candidate_state_dirs(game_root)

    return {
        "game_root": str(game_root),
        "bin_dir": str(bin_dir),
        "game_exe": str(bin_dir / "game.exe"),
        "game_exe_exists": (bin_dir / "game.exe").exists(),
        "config_tool_exists": (bin_dir / "ConfigTool.exe").exists(),
        "steam_api_exists": (bin_dir / "steam_api64.dll").exists(),
        "resource_dir_exists": resource_dir.exists(),
        "ak3_exists": (resource_dir / "ak3.json").exists(),
        "info_db3_exists": (resource_dir / "data" / "info.db3").exists(),
        "pack_files": [
            {"path": str(path), "name": path.name, "size": path.stat().st_size} for path in pack_files if path.exists()
        ],
        "state_dir_candidates": [
            {"path": str(path), "exists": path.exists(), "is_dir": path.is_dir()} for path in state_dirs
        ],
        "capture_tools": capture_tool_status(),
        "notes": [
            "No platform bypass is performed.",
            "File IO, D3D, audio, and screenshot traces require external capture tools or a user-driven launch.",
        ],
    }


def build_readiness(game_root: Path) -> dict[str, Any]:
    inspection = inspect_game(game_root)
    tool_rows = {row["name"]: row for row in inspection["capture_tools"]}
    file_io_tools = ["procmon", "procmon64"]
    render_tools = ["apitrace", "renderdoccmd", "pixwin"]
    dump_tools = ["procdump"]

    available_file_io = [name for name in file_io_tools if tool_rows.get(name, {}).get("available")]
    available_render = [name for name in render_tools if tool_rows.get(name, {}).get("available")]
    available_dump = [name for name in dump_tools if tool_rows.get(name, {}).get("available")]

    blockers: list[str] = []
    if not inspection["game_exe_exists"]:
        blockers.append("original game executable is missing")
    if not available_file_io:
        blockers.append("no file-IO capture tool found on PATH")
    if not available_render:
        blockers.append("no D3D/render capture tool found on PATH")
    blockers.append("no user-driven title boot run has been recorded in this workspace")

    return {
        "schema": "yuengine.oracle_title_boot.readiness.v1",
        "game_root": inspection["game_root"],
        "can_launch_original_exe": inspection["game_exe_exists"],
        "requires_user_driven_launch": True,
        "safe_to_bypass_platform": False,
        "file_io_capture_tools": available_file_io,
        "render_capture_tools": available_render,
        "dump_capture_tools": available_dump,
        "capture_tool_status": inspection["capture_tools"],
        "blockers": blockers,
        "non_blocked_preparation": [
            "install and state snapshots can be recorded",
            "runbook can be generated",
            "manual title-frame observations can be recorded after user-driven launch",
        ],
        "p1_complete": False,
    }


def snapshot_tree(path: Path, hash_large: bool, limit_files: int) -> list[dict[str, Any]]:
    if not path.exists() or not path.is_dir():
        return []
    records = []
    for child in sorted(p for p in path.rglob("*") if p.is_file()):
        if len(records) >= limit_files:
            records.append({"path": str(path), "truncated": True, "limit_files": limit_files})
            break
        records.append(file_record(child, path, hash_large=hash_large))
    return records


def build_snapshot(game_root: Path, hash_large: bool = False, state_file_limit: int = 500) -> dict[str, Any]:
    game_root = game_root.resolve()
    install_records = []
    for path in critical_install_files(game_root):
        install_records.append(file_record(path, game_root, hash_large=hash_large) if path.exists() else missing_record(path, game_root))

    state_records = []
    for state_dir in candidate_state_dirs(game_root):
        state_records.append(
            {
                "path": str(state_dir),
                "exists": state_dir.exists(),
                "files": snapshot_tree(state_dir, hash_large=hash_large, limit_files=state_file_limit),
            }
        )

    return {
        "schema": "yuengine.oracle_title_boot.snapshot.v1",
        "game_root": str(game_root),
        "hash_large_files": hash_large,
        "install_files": install_records,
        "state_dirs": state_records,
    }


def write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def write_runbook(path: Path, inspection: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    available = [tool["name"] for tool in inspection["capture_tools"] if tool["available"]]
    unavailable = [tool["name"] for tool in inspection["capture_tools"] if not tool["available"]]

    lines = [
        "# Oracle Title Boot Runbook",
        "",
        "Purpose: capture original title boot behavior without patching or bypassing platform checks.",
        "",
        "## Detected Install",
        "",
        f"- Game root: `{inspection['game_root']}`",
        f"- Game exe exists: `{inspection['game_exe_exists']}`",
        f"- Config tool exists: `{inspection['config_tool_exists']}`",
        f"- Steam API DLL exists: `{inspection['steam_api_exists']}`",
        f"- Pack files: `{len(inspection['pack_files'])}`",
        "",
        "## Capture Tool Status",
        "",
        f"- Available: `{', '.join(available) if available else 'none'}`",
        f"- Missing: `{', '.join(unavailable) if unavailable else 'none'}`",
        "",
        "## Safe Protocol",
        "",
        "1. Record pre-run snapshot:",
        "",
        "```powershell",
        "python tools\\oracle_title_boot.py snapshot --out build\\oracle_title_boot\\pre_title_snapshot.json",
        "```",
        "",
        "2. Launch the original game normally through Steam or use the explicit local launch command:",
        "",
        "```powershell",
        "python tools\\oracle_title_boot.py launch --duration 20 --out build\\oracle_title_boot\\launch_title.json",
        "```",
        "",
        "3. Do not patch executables, replace Steam DLLs, or force entitlement state.",
        "",
        "4. Capture or manually record:",
        "",
        "- first visible title frame screenshot/hash;",
        "- file/resource reads if Procmon or equivalent is available;",
        "- D3D9 first present/render events if apitrace/RenderDoc/PIX is available;",
        "- audio/BGM file or event if available;",
        "- save list state before and after title boot.",
        "",
        "5. Record post-run snapshot:",
        "",
        "```powershell",
        "python tools\\oracle_title_boot.py snapshot --out build\\oracle_title_boot\\post_title_snapshot.json",
        "```",
        "",
        "## Acceptance For P1",
        "",
        "- Three stable title boot runs.",
        "- Trace maps back to `script/menu/titlemenu.b64.sqasm`.",
        "- Title resource stems are observed or explicitly marked unobservable with current tools.",
        "- Save list query behavior is sampled or explicitly marked pending.",
        "",
        "## Current Limitation",
        "",
        "This environment currently lacks dedicated file-IO/render capture tools unless one is installed later.",
    ]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def cmd_inspect(args: argparse.Namespace) -> int:
    data = inspect_game(args.game_root)
    if args.out:
        write_json(args.out, data)
    print(json.dumps(data, indent=2, ensure_ascii=False))
    return 0


def cmd_snapshot(args: argparse.Namespace) -> int:
    data = build_snapshot(args.game_root, hash_large=args.hash_large, state_file_limit=args.state_file_limit)
    write_json(args.out, data)
    print(f"snapshot: {args.out}")
    return 0


def cmd_runbook(args: argparse.Namespace) -> int:
    inspection = inspect_game(args.game_root)
    write_runbook(args.out, inspection)
    print(f"runbook: {args.out}")
    return 0


def cmd_readiness(args: argparse.Namespace) -> int:
    data = build_readiness(args.game_root)
    if args.out:
        write_json(args.out, data)
    print(json.dumps(data, indent=2, ensure_ascii=False))
    return 0


def cmd_launch(args: argparse.Namespace) -> int:
    game_root = args.game_root.resolve()
    exe = game_root / "bin" / "game.exe"
    if not exe.exists():
        raise SystemExit(f"game.exe not found: {exe}")
    started_at = time.time()
    proc = subprocess.Popen([str(exe)], cwd=str(exe.parent))
    time.sleep(args.duration)
    status = proc.poll()
    result = {
        "schema": "yuengine.oracle_title_boot.launch.v1",
        "game_root": str(game_root),
        "exe": str(exe),
        "pid": proc.pid,
        "duration": args.duration,
        "started_at": started_at,
        "returncode_after_duration": status,
        "terminated_by_tool": False,
    }
    if args.terminate_after and status is None:
        proc.terminate()
        result["terminated_by_tool"] = True
        try:
            result["returncode_after_terminate"] = proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            result["returncode_after_terminate"] = None
    if args.out:
        write_json(args.out, result)
    print(json.dumps(result, indent=2, ensure_ascii=False))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-root", type=Path, default=default_game_root(), help="Touhou New World install root")
    sub = parser.add_subparsers(dest="command", required=True)

    inspect = sub.add_parser("inspect")
    inspect.add_argument("--out", type=Path)
    inspect.set_defaults(func=cmd_inspect)

    snapshot = sub.add_parser("snapshot")
    snapshot.add_argument("--out", type=Path, required=True)
    snapshot.add_argument("--hash-large", action="store_true")
    snapshot.add_argument("--state-file-limit", type=int, default=500)
    snapshot.set_defaults(func=cmd_snapshot)

    runbook = sub.add_parser("runbook")
    runbook.add_argument("--out", type=Path, required=True)
    runbook.set_defaults(func=cmd_runbook)

    readiness = sub.add_parser("readiness")
    readiness.add_argument("--out", type=Path)
    readiness.set_defaults(func=cmd_readiness)

    launch = sub.add_parser("launch")
    launch.add_argument("--duration", type=float, default=20.0)
    launch.add_argument("--terminate-after", action="store_true")
    launch.add_argument("--out", type=Path)
    launch.set_defaults(func=cmd_launch)

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
