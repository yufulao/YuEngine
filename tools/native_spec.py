#!/usr/bin/env python3
"""
Build a native/API boundary spec from Script IR.

This is a tracking artifact generator. It assigns proposed service owners and records script
evidence, but it does not confirm native behavior or implementation status.
"""

from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

try:
    from sqir import default_db_path, enrich_from_db, parse_sqasm, summarize
except ImportError:  # pragma: no cover - used when imported as tools.native_spec
    from tools.sqir import default_db_path, enrich_from_db, parse_sqasm, summarize


BOUNDARY_CLASSES = {"engine_boundary_candidate", "type_or_engine_candidate", "confirmed-native"}


EXACT_OWNER: dict[str, tuple[str, str]] = {
    "CanShutdown": ("Platform Service", "service-map platform/shutdown API"),
    "ShutdownGame": ("Platform Service", "service-map platform/shutdown API"),
    "IsTrial": ("Platform Service", "service-map trial/demo API"),
    "IsFreeDemo": ("Platform Service", "service-map trial/demo API"),
    "IsOverDemo": ("Platform Service", "demo-state platform boundary"),
    "GetCountActiveDLC": ("Platform Service", "service-map DLC/entitlement API"),
    "IsEntitledSpirits": ("Platform Service", "service-map entitlement API"),
    "GetGraphResident": ("Resource Service", "service-map resource handle API"),
    "GetSaveList": ("Save/Profile/Scenario Service", "service-map save API"),
    "LoadAutoSave": ("Save/Profile/Scenario Service", "service-map save API"),
    "MakeNewGame": ("Save/Profile/Scenario Service", "service-map new-game API"),
    "StartGame": ("Save/Profile/Scenario Service", "service-map start-game API"),
    "GetScenarioKeys": ("Save/Profile/Scenario Service", "service-map scenario API"),
    "FindScenario": ("Save/Profile/Scenario Service", "service-map scenario API"),
    "SetDifficultyMode": ("Save/Profile/Scenario Service", "service-map difficulty API"),
    "HasClearSave": ("Save/Profile/Scenario Service", "service-map save/progression API"),
    "DeleteAutoSave": ("Save/Profile/Scenario Service", "autosave mutation API"),
    "IsSaveFull": ("Save/Profile/Scenario Service", "save-capacity API"),
    "LoadScene": ("Scene And Stage Service", "title scene transition boundary"),
    "NewGameScene": ("Scene And Stage Service", "title scene constructor boundary"),
    "OverwriteSaveScene": ("Scene And Stage Service", "title scene constructor boundary"),
    "TitleScene": ("Scene And Stage Service", "title scene constructor boundary"),
    "Loader": ("Scene And Stage Service", "service-map scene loader API"),
    "LoadStage": ("Scene And Stage Service", "service-map stage loader API"),
    "LoadEventsScriptViaMission": ("Scene And Stage Service", "service-map event script loader API"),
    "CallSetupEvents": ("Scene And Stage Service", "service-map setup event API"),
    "FadeIn": ("Scene And Stage Service", "scene transition API"),
    "FadeOut": ("Scene And Stage Service", "scene transition API"),
    "EnterTransition": ("Scene And Stage Service", "scene transition API"),
    "LeaveTransition": ("Scene And Stage Service", "scene transition API"),
    "PlayBGM": ("Audio Service", "service-map BGM API"),
    "PlaySE": ("Audio Service", "service-map SE API"),
    "FadeOutBGM": ("Audio Service", "audio fade API"),
    "PlayEffect": ("Actor And Task Service", "actor/effect playback API"),
    "PushPlayerChara": ("Actor And Task Service", "service-map player actor API"),
    "PushActor": ("Actor And Task Service", "service-map actor API"),
    "GetPlayer": ("Actor And Task Service", "service-map actor handle API"),
    "GetPchar": ("Actor And Task Service", "service-map player character API"),
    "GetCurrentPlayerName": ("Actor And Task Service", "player actor identity API"),
    "GetPlayerNameList": ("Actor And Task Service", "player actor identity API"),
    "SetPlayerControl": ("Actor And Task Service", "player control flag API"),
    "GetPlayerControl": ("Actor And Task Service", "player control flag API"),
    "SetPlayerPos": ("Actor And Task Service", "player transform API"),
    "SetPlayerAngleY": ("Actor And Task Service", "player transform API"),
    "GetPlayerPos": ("Actor And Task Service", "player transform API"),
    "ActorTutorial": ("Actor And Task Service", "tutorial actor API"),
    "WaitActor": ("Actor And Task Service", "actor wait/task API"),
    "LandPlayer": ("Actor And Task Service", "player landing API"),
    "PushTaskGameCamera": ("Camera Service", "service-map game camera task API"),
    "LoadRailCamera": ("Camera Service", "service-map rail camera API"),
    "SetEnableRailCamera": ("Camera Service", "service-map rail camera API"),
    "SetEnableAutoCameraAdjust": ("Camera Service", "service-map auto camera API"),
    "SetDefaultCameraState": ("Camera Service", "service-map default camera API"),
    "SetGameCameraIfNot": ("Camera Service", "mission camera guard API"),
    "EventClass": ("Event/Quest/Flag Service", "service-map event API"),
    "GetEventUnit": ("Event/Quest/Flag Service", "service-map event unit API"),
    "GetFlag": ("Event/Quest/Flag Service", "service-map flag API"),
    "ClearCurrentQuest": ("Event/Quest/Flag Service", "quest-state API"),
    "CallSetupPages": ("Event/Quest/Flag Service", "event/page setup API"),
    "ClearActorsAll": ("Actor And Task Service", "actor manager clear API"),
    "ClearEventsAll": ("Event/Quest/Flag Service", "event manager clear API"),
    "SetCheckPoint": ("Event/Quest/Flag Service", "checkpoint/progression API"),
    "EventVolume": ("Collision And Physics-Lite Service", "service-map event volume API"),
    "QuatToRotYDegree": ("Script Service", "script-visible math/value helper"),
    "GetPlaceParams": ("Scene And Stage Service", "stage placement API"),
    "GetMarkerFromRequest": ("Scene And Stage Service", "stage marker lookup API"),
    "LoadCharaPlace": ("Scene And Stage Service", "stage character placement API"),
    "SetFadeColor": ("Scene And Stage Service", "scene fade/transition color API"),
    "ResetMenuButtonHoldingTimes": ("Platform Service", "input/menu timing API"),
    "DeactiveJust": ("Platform Service", "input/action timing API"),
    "StepEffect": ("Actor And Task Service", "actor/effect task helper"),
    "TransformEffect": ("Actor And Task Service", "actor/effect transform helper"),
    "UpdateUnits": ("Event/Quest/Flag Service", "event unit update API"),
    "ColorFloat": ("UI And 2D Render Service", "service-map color/render API"),
    "DrawFrameUsual": ("UI And 2D Render Service", "service-map UI draw API"),
    "DrawRectUsual": ("UI And 2D Render Service", "service-map UI draw API"),
    "DrawString": ("UI And 2D Render Service", "service-map string draw API"),
    "DrawStringAlignRight": ("UI And 2D Render Service", "service-map string draw API"),
    "GetStringSize": ("UI And 2D Render Service", "service-map font metric API"),
    "GraphString": ("UI And 2D Render Service", "service-map graph string API"),
    "MenuObject": ("UI And 2D Render Service", "service-map menu object API"),
    "Vec2": ("Script Service", "script-visible math/value type binding"),
    "Vec3": ("Script Service", "script-visible math/value type binding"),
    "GetManager": ("Script Service", "script-visible manager accessor"),
}


PREFIX_OWNER: list[tuple[str, str, str]] = [
    ("Draw", "UI And 2D Render Service", "draw-call prefix"),
    ("Graph", "UI And 2D Render Service", "graph/render prefix"),
    ("GetGraph", "Resource Service", "resource-handle prefix"),
    ("Play", "Audio Service", "audio prefix"),
    ("FadeOutBGM", "Audio Service", "audio fade prefix"),
    ("LoadRailCamera", "Camera Service", "camera prefix"),
    ("SetEnableRailCamera", "Camera Service", "camera prefix"),
    ("SetDefaultCamera", "Camera Service", "camera prefix"),
    ("SetGameCamera", "Camera Service", "camera prefix"),
    ("PushTaskGameCamera", "Camera Service", "camera prefix"),
    ("GetPlayer", "Actor And Task Service", "actor/player prefix"),
    ("SetPlayer", "Actor And Task Service", "actor/player prefix"),
    ("PushActor", "Actor And Task Service", "actor prefix"),
    ("PushPlayer", "Actor And Task Service", "actor/player prefix"),
    ("Actor", "Actor And Task Service", "actor type prefix"),
    ("WaitActor", "Actor And Task Service", "actor wait prefix"),
    ("LoadStage", "Scene And Stage Service", "stage prefix"),
    ("LoadEvents", "Scene And Stage Service", "event script loader prefix"),
    ("LoadCharaPlace", "Scene And Stage Service", "stage placement prefix"),
    ("CallSetupEvents", "Scene And Stage Service", "scene setup prefix"),
    ("GetMarker", "Scene And Stage Service", "stage marker prefix"),
    ("GetPlace", "Scene And Stage Service", "stage placement prefix"),
    ("Event", "Event/Quest/Flag Service", "event prefix"),
    ("GetEvent", "Event/Quest/Flag Service", "event prefix"),
    ("GetFlag", "Event/Quest/Flag Service", "flag prefix"),
    ("SetFlag", "Event/Quest/Flag Service", "flag prefix"),
    ("FindQuest", "Event/Quest/Flag Service", "quest prefix"),
    ("ProceedQuest", "Event/Quest/Flag Service", "quest prefix"),
    ("SetCurrentQuest", "Event/Quest/Flag Service", "quest prefix"),
    ("ClearCurrentQuest", "Event/Quest/Flag Service", "quest prefix"),
    ("EnterTransition", "Scene And Stage Service", "transition prefix"),
    ("LeaveTransition", "Scene And Stage Service", "transition prefix"),
]


SERVICE_UNKNOWNS: dict[str, list[str]] = {
    "Platform Service": [
        "runtime source for platform/demo/entitlement state",
        "shutdown and input side effects",
    ],
    "Resource Service": [
        "resource handle lifetime",
        "path normalization and resident-cache behavior",
    ],
    "Script Service": [
        "bound object layout",
        "constructor/value-type argument semantics",
    ],
    "UI And 2D Render Service": [
        "draw command argument shape",
        "layer, coordinate, color, and font semantics",
    ],
    "Save/Profile/Scenario Service": [
        "argument shape",
        "return table/object schema",
        "save/profile/scenario mutation side effects",
    ],
    "Scene And Stage Service": [
        "load order and resource dependency graph",
        "transition and scene-lifecycle side effects",
    ],
    "Actor And Task Service": [
        "actor handle lifecycle",
        "player/control/task side effects",
    ],
    "Camera Service": [
        "camera task lifecycle",
        "rail/default/auto-adjust state semantics",
    ],
    "Collision And Physics-Lite Service": [
        "volume/collision shape",
        "physics-lite side effects",
    ],
    "Audio Service": [
        "audio id/path mapping",
        "channel, loop, and fade semantics",
    ],
    "Event/Quest/Flag Service": [
        "event unit/page schema",
        "flag and quest storage side effects",
    ],
}


@dataclass
class Site:
    script: str
    function: str
    source_line: int | None
    pc: int | None

    def label(self) -> str:
        line = "?" if self.source_line is None else str(self.source_line)
        pc = "?" if self.pc is None else str(self.pc)
        return f"{self.script}:{self.function}:L{line}:pc{pc}"


@dataclass
class BoundaryRow:
    name: str
    service_owner: str
    owner_level: str
    owner_reason: str
    classifications: Counter[str] = field(default_factory=Counter)
    call_sites: int = 0
    scripts: set[str] = field(default_factory=set)
    examples: list[Site] = field(default_factory=list)
    evidence_level: str = "E1"
    confirmed_native: str = "unknown"
    oracle_sampled: str = "no"
    argument_shape: str = "unknown"
    return_shape: str = "unknown"
    side_effects: str = "unknown"
    implementation_status: str = "not_started"
    residual_unknowns: list[str] = field(default_factory=list)

    def to_json(self) -> dict[str, Any]:
        return {
            "name": self.name,
            "service_owner": self.service_owner,
            "owner_level": self.owner_level,
            "owner_reason": self.owner_reason,
            "classifications": dict(self.classifications),
            "call_sites": self.call_sites,
            "scripts": sorted(self.scripts),
            "examples": [site.label() for site in self.examples],
            "evidence_level": self.evidence_level,
            "confirmed_native": self.confirmed_native,
            "oracle_sampled": self.oracle_sampled,
            "argument_shape": self.argument_shape,
            "return_shape": self.return_shape,
            "side_effects": self.side_effects,
            "implementation_status": self.implementation_status,
            "residual_unknowns": self.residual_unknowns,
        }


def owner_for(name: str) -> tuple[str, str, str]:
    if name in EXACT_OWNER:
        owner, reason = EXACT_OWNER[name]
        return owner, "proposed_owner", reason
    for prefix, owner, reason in PREFIX_OWNER:
        if name.startswith(prefix):
            return owner, "proposed_owner", reason
    return "Unassigned Service", "unassigned", "no owner rule matched"


def iter_boundary_calls(ir: dict[str, Any]) -> list[tuple[str, str, Site]]:
    rows: list[tuple[str, str, Site]] = []
    script = ir["sqasm_path"]
    for fn in ir["functions"]:
        function_name = fn["name"] if fn["name"] is not None else "None"
        for call in fn["calls"]:
            classification = call.get("classification") or "unknown"
            if classification not in BOUNDARY_CLASSES:
                continue
            rows.append(
                (
                    call["name"],
                    classification,
                    Site(
                        script=script,
                        function=function_name,
                        source_line=call.get("source_line"),
                        pc=call.get("pc"),
                    ),
                )
            )
    return rows


def build_spec(sqasm_paths: list[Path], db_path: Path | None) -> dict[str, Any]:
    row_by_name: dict[str, BoundaryRow] = {}
    scripts: list[dict[str, Any]] = []

    for path in sqasm_paths:
        ir = parse_sqasm(path)
        if db_path is not None:
            enrich_from_db(ir, db_path)
        summarize(ir)
        scripts.append(
            {
                "sqasm_path": ir["sqasm_path"],
                "input_file": str(path),
                "summary": ir["summary"],
            }
        )

        for name, classification, site in iter_boundary_calls(ir):
            if name not in row_by_name:
                owner, level, reason = owner_for(name)
                row_by_name[name] = BoundaryRow(
                    name=name,
                    service_owner=owner,
                    owner_level=level,
                    owner_reason=reason,
                    residual_unknowns=SERVICE_UNKNOWNS.get(owner, ["owner assignment requires review"]),
                )
            row = row_by_name[name]
            row.classifications[classification] += 1
            row.call_sites += 1
            row.scripts.add(site.script)
            if len(row.examples) < 4:
                row.examples.append(site)

    rows = sorted(row_by_name.values(), key=lambda row: (row.service_owner, row.name))
    service_counts = Counter(row.service_owner for row in rows)
    unassigned = [row.name for row in rows if row.service_owner == "Unassigned Service"]

    return {
        "format": "yuengine.native_boundary_spec.v1",
        "scope": "title_newgame_first_mission",
        "source_scripts": scripts,
        "policy": {
            "status": "candidate/spec baseline",
            "not_a_claim": [
                "does not confirm native behavior",
                "does not confirm argument or return shape",
                "does not confirm side effects",
                "does not mark P4 complete",
            ],
        },
        "summary": {
            "boundary_count": len(rows),
            "call_sites": sum(row.call_sites for row in rows),
            "service_counts": dict(sorted(service_counts.items())),
            "unassigned": unassigned,
        },
        "rows": [row.to_json() for row in rows],
    }


def markdown_table(rows: list[dict[str, Any]]) -> list[str]:
    lines = [
        "| API | Owner | Owner Level | Calls | Class | Evidence | Impl | Unknowns | Examples |",
        "| --- | --- | --- | ---: | --- | --- | --- | --- | --- |",
    ]
    for row in rows:
        classifications = ", ".join(f"{name}:{count}" for name, count in sorted(row["classifications"].items()))
        unknowns = "; ".join(row["residual_unknowns"][:2])
        examples = "<br>".join(row["examples"][:2])
        lines.append(
            "| "
            f"`{row['name']}` | "
            f"{row['service_owner']} | "
            f"{row['owner_level']} | "
            f"{row['call_sites']} | "
            f"{classifications} | "
            f"{row['evidence_level']} | "
            f"{row['implementation_status']} | "
            f"{unknowns} | "
            f"{examples} |"
        )
    return lines


def write_markdown(path: Path, spec: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    summary = spec["summary"]
    lines = [
        "# Native Boundary Spec: Title/New-Game/First-Mission",
        "",
        "Generated by `tools/native_spec.py` from Script IR plus Project2 evidence graph.",
        "",
        "This is a tracking artifact, not proof that native behavior is recovered.",
        "",
        "## Scope",
        "",
    ]
    lines.extend(f"- `{script['sqasm_path']}`" for script in spec["source_scripts"])
    lines.extend(
        [
            "",
            "## Guardrails",
            "",
            "- `owner_level=proposed_owner` means service ownership is proposed from names, service-map rules, and script context.",
            "- `confirmed_native=unknown` means no binary/oracle confirmation has been recorded in this table.",
            "- `oracle_sampled=no` means runtime behavior has not been sampled for the row.",
            "- `implementation_status=not_started` means runtime implementation must not silently stub this API.",
            "- This file does not mark P4 complete.",
            "",
            "## Summary",
            "",
            f"- Boundary APIs: {summary['boundary_count']}",
            f"- Boundary call sites: {summary['call_sites']}",
            f"- Unassigned owners: {len(summary['unassigned'])}",
            "",
            "Service counts:",
            "",
        ]
    )
    for service, count in summary["service_counts"].items():
        lines.append(f"- {service}: {count}")
    if summary["unassigned"]:
        lines.extend(["", "Unassigned:", ""])
        lines.extend(f"- `{name}`" for name in summary["unassigned"])
    lines.extend(["", "## Boundary Rows", ""])
    lines.extend(markdown_table(spec["rows"]))
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--script", action="append", type=Path, required=True, help="Input .sqasm path")
    parser.add_argument("--db", type=Path, default=default_db_path(), help="Evidence graph SQLite path")
    parser.add_argument("--no-db", action="store_true", help="Disable evidence graph enrichment")
    parser.add_argument("--json-out", type=Path, help="Write JSON spec")
    parser.add_argument("--md-out", type=Path, help="Write markdown spec")
    parser.add_argument("--stdout", action="store_true", help="Print JSON spec to stdout")
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    for path in args.script:
        if not path.exists():
            parser.error(f"script not found: {path}")

    spec = build_spec(args.script, None if args.no_db else args.db)
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(spec, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    if args.md_out:
        write_markdown(args.md_out, spec)
    if args.stdout or not (args.json_out or args.md_out):
        print(json.dumps(spec, indent=2, ensure_ascii=False))
    else:
        print(
            "native-spec: "
            f"{spec['summary']['boundary_count']} APIs, "
            f"{spec['summary']['call_sites']} call sites, "
            f"{len(spec['summary']['unassigned'])} unassigned"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
