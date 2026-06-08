#!/usr/bin/env python3
"""
Convert observed Squirrel `.sqasm` text into stable JSON IR.

The parser preserves the disassembly as structured data and can enrich calls/resources from
Project2's evidence graph when that SQLite file is available.
"""

from __future__ import annotations

import argparse
import ast
import json
import re
import sqlite3
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


FUNCTION_RE = re.compile(
    r"^\s*function\s+#(?P<ordinal>\d+)\s+"
    r"(?P<name>'[^']*'|None)\s+"
    r"source=(?P<source>'[^']*'|\S+)\s+"
    r"offset=(?P<offset>0x[0-9a-fA-F]+)"
)
INSTRUCTION_RE = re.compile(
    r"^\s*(?P<pc>\d+)\s+L(?P<line>\d+)\s+"
    r"(?P<op>\S+)(?P<operands>.*?)(?:\s+;\s+(?P<comment>.*))?$"
)
LITERAL_RE = re.compile(r"^\s*\[(?P<index>\d+)\]\s+(?P<value>.*)$")
LOCAL_RE = re.compile(r"^\s*(?P<register>r\d+)\s+(?P<name>'[^']*'|None)\s+ops=(?P<ops>.*)$")
ARGS_RE = re.compile(r"\ba(?P<index>[0-3])=\s*(?P<value>-?\d+)")
RESOURCE_EXTENSIONS = {
    ".b64",
    ".dds",
    ".fmp",
    ".ogg",
    ".rcm",
    ".se",
    ".sge",
}


def norm_path(value: str) -> str:
    return value.replace("\\", "/").strip().lower()


def parse_atom(raw: str) -> Any:
    raw = raw.strip()
    if raw == "None":
        return None
    try:
        return ast.literal_eval(raw)
    except (SyntaxError, ValueError):
        return raw


def parse_bool(value: str) -> bool:
    return value.strip().lower() == "true"


@dataclass
class FunctionIR:
    ordinal: int
    name: str | None
    source: str | None
    offset: str
    stack: int | None = None
    generator: bool | None = None
    varargs: bool | None = None
    parameters: list[str] = field(default_factory=list)
    literals: list[dict[str, Any]] = field(default_factory=list)
    locals: list[dict[str, Any]] = field(default_factory=list)
    instructions: list[dict[str, Any]] = field(default_factory=list)
    calls: list[dict[str, Any]] = field(default_factory=list)
    resource_refs: list[dict[str, Any]] = field(default_factory=list)
    closure_bindings: list[dict[str, Any]] = field(default_factory=list)
    state_candidates: list[str] = field(default_factory=list)

    def to_json(self) -> dict[str, Any]:
        return {
            "ordinal": self.ordinal,
            "name": self.name,
            "source": self.source,
            "offset": self.offset,
            "stack": self.stack,
            "generator": self.generator,
            "varargs": self.varargs,
            "parameters": self.parameters,
            "literals": self.literals,
            "locals": self.locals,
            "instructions": self.instructions,
            "calls": self.calls,
            "resource_refs": self.resource_refs,
            "closure_bindings": self.closure_bindings,
            "state_candidates": self.state_candidates,
        }


def parse_comment(comment: str | None) -> dict[str, Any]:
    if not comment:
        return {}

    result: dict[str, Any] = {}
    literal_refs: list[dict[str, Any]] = []
    function_refs: list[dict[str, Any]] = []

    for match in re.finditer(r"literal\[(?P<index>\d+)\]=(?P<value>'[^']*'|None|[^;]+)", comment):
        literal_refs.append(
            {
                "slot": f"literal[{match.group('index')}]",
                "index": int(match.group("index")),
                "value": parse_atom(match.group("value").strip()),
            }
        )

    for match in re.finditer(r"(?P<slot>literal\d+)=(?P<value>'[^']*'|None|[^;]+)", comment):
        literal_refs.append(
            {
                "slot": match.group("slot"),
                "index": None,
                "value": parse_atom(match.group("value").strip()),
            }
        )

    for match in re.finditer(r"function\[(?P<index>\d+)\]=(?P<value>'[^']*'|None|[^;]+)", comment):
        function_refs.append(
            {
                "index": int(match.group("index")),
                "value": parse_atom(match.group("value").strip()),
            }
        )

    target = re.search(r"target=(?P<target>\d+)", comment)
    if target:
        result["target"] = int(target.group("target"))
    if literal_refs:
        result["literal_refs"] = literal_refs
    if function_refs:
        result["function_refs"] = function_refs
    return result


def parse_instruction(line: str) -> dict[str, Any] | None:
    match = INSTRUCTION_RE.match(line)
    if not match:
        return None

    operands = match.group("operands") or ""
    args = {f"a{m.group('index')}": int(m.group("value")) for m in ARGS_RE.finditer(operands)}
    comment = match.group("comment")
    parsed = {
        "pc": int(match.group("pc")),
        "source_line": int(match.group("line")),
        "op": match.group("op"),
        "args": args,
        "comment": comment,
    }
    parsed.update(parse_comment(comment))
    return parsed


def infer_script_paths(input_path: Path, embedded_input: str | None) -> tuple[str | None, str]:
    if embedded_input:
        embedded = embedded_input.replace("\\", "/")
        if embedded.endswith(".sqasm"):
            return embedded.removesuffix(".sqasm"), embedded
        return embedded, embedded + ".sqasm"

    parts = [part.lower() for part in input_path.parts]
    if "scripts" in parts:
        index = parts.index("scripts")
        relative = "/".join(input_path.parts[index + 1 :]).replace("\\", "/")
        if relative.endswith(".sqasm"):
            return relative.removesuffix(".sqasm"), relative
        return relative, relative + ".sqasm"

    name = input_path.name.replace("\\", "/")
    if name.endswith(".sqasm"):
        return name.removesuffix(".sqasm"), name
    return name, name + ".sqasm"


def extract_calls_from_instructions(function: FunctionIR) -> list[dict[str, Any]]:
    calls: list[dict[str, Any]] = []
    for inst in function.instructions:
        if not inst["op"].startswith("_OP_PREPCALL"):
            continue
        literal_refs = inst.get("literal_refs") or []
        if not literal_refs:
            continue
        first = literal_refs[0]
        name = first.get("value")
        if not isinstance(name, str):
            continue
        calls.append(
            {
                "name": name,
                "literal_index": first.get("index"),
                "pc": inst["pc"],
                "source_line": inst["source_line"],
                "classification": "parsed-callsite",
                "source": "sqasm",
            }
        )
    return calls


def extract_closure_bindings(function: FunctionIR) -> list[dict[str, Any]]:
    bindings: list[dict[str, Any]] = []
    previous_literal: dict[str, Any] | None = None

    for inst in function.instructions:
        literal_refs = inst.get("literal_refs") or []
        if literal_refs:
            previous_literal = literal_refs[0]

        if inst["op"] != "_OP_CLOSURE":
            continue
        refs = inst.get("function_refs") or []
        if not refs:
            continue
        ref = refs[0]
        bindings.append(
            {
                "pc": inst["pc"],
                "source_line": inst["source_line"],
                "slot": previous_literal.get("value") if previous_literal else None,
                "function_ordinal": ref.get("index"),
                "function_name": ref.get("value"),
            }
        )
    return bindings


def extract_state_candidates(function: FunctionIR) -> list[str]:
    candidates: set[str] = set()
    if function.name and re.match(r"^(initState|state)\d+$", function.name):
        candidates.add(function.name)

    for literal in function.literals:
        value = literal.get("value")
        if not isinstance(value, str):
            continue
        if re.match(r"^(STATE_|TITLE_MENU_|SELECT_)", value):
            candidates.add(value)
        if value in {"_state", "_nextState", "_scene", "sceneChangeInit"}:
            candidates.add(value)
    return sorted(candidates)


def guess_resource_kind(value: Any) -> str | None:
    if not isinstance(value, str):
        return None
    if "/" not in value and "\\" not in value:
        return None
    normalized = value.replace("\\", "/")
    suffix = Path(normalized).suffix.lower()
    if suffix in RESOURCE_EXTENSIONS:
        return "resource_path"
    if normalized.startswith("../"):
        return None
    return "resource_stem"


def extract_resource_refs_from_literals(function: FunctionIR) -> list[dict[str, Any]]:
    refs: list[dict[str, Any]] = []
    for literal in function.literals:
        kind = guess_resource_kind(literal.get("value"))
        if not kind:
            continue
        literal["kind"] = kind
        refs.append(
            {
                "literal_index": literal["index"],
                "value": literal["value"],
                "kind": kind,
                "line_no": None,
                "manifest_path": None,
                "pack": None,
                "ext": Path(str(literal["value"]).replace("\\", "/")).suffix.lower() or None,
                "size": None,
                "source": "sqasm-heuristic",
            }
        )
    return refs


def parse_sqasm(path: Path) -> dict[str, Any]:
    embedded_input: str | None = None
    functions: list[FunctionIR] = []
    current: FunctionIR | None = None
    section: str | None = None

    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        stripped = raw_line.strip()
        if stripped.startswith("; input:"):
            embedded_input = stripped.split(":", 1)[1].strip()
            continue

        function_match = FUNCTION_RE.match(raw_line)
        if function_match:
            current = FunctionIR(
                ordinal=int(function_match.group("ordinal")),
                name=parse_atom(function_match.group("name")),
                source=parse_atom(function_match.group("source")),
                offset=function_match.group("offset"),
            )
            functions.append(current)
            section = None
            continue

        if current is None:
            continue

        if stripped.startswith("stack="):
            fields = dict(item.split("=", 1) for item in stripped.split() if "=" in item)
            current.stack = int(fields["stack"])
            current.generator = parse_bool(fields["generator"])
            current.varargs = parse_bool(fields["varargs"])
            continue

        if stripped.startswith("parameters="):
            current.parameters = list(parse_atom(stripped.split("=", 1)[1]))
            continue

        if stripped == "literals:":
            section = "literals"
            continue
        if stripped == "locals:":
            section = "locals"
            continue
        if stripped == "instructions:":
            section = "instructions"
            continue

        if section == "literals":
            literal_match = LITERAL_RE.match(raw_line)
            if literal_match:
                current.literals.append(
                    {
                        "index": int(literal_match.group("index")),
                        "value": parse_atom(literal_match.group("value")),
                        "raw": literal_match.group("value").strip(),
                    }
                )
            continue

        if section == "locals":
            local_match = LOCAL_RE.match(raw_line)
            if local_match:
                current.locals.append(
                    {
                        "register": local_match.group("register"),
                        "name": parse_atom(local_match.group("name")),
                        "ops": local_match.group("ops"),
                    }
                )
            continue

        if section == "instructions":
            instruction = parse_instruction(raw_line)
            if instruction:
                current.instructions.append(instruction)

    script, sqasm_path = infer_script_paths(path, embedded_input)
    for function in functions:
        function.calls = extract_calls_from_instructions(function)
        function.resource_refs = extract_resource_refs_from_literals(function)
        function.closure_bindings = extract_closure_bindings(function)
        function.state_candidates = extract_state_candidates(function)

    return {
        "format": "yuengine.sqir.v1",
        "script": script,
        "sqasm_path": sqasm_path,
        "input_file": str(path),
        "functions": [function.to_json() for function in functions],
    }


def default_db_path() -> Path:
    return Path(__file__).resolve().parents[2] / "Project2" / "research" / "evidence_graph.sqlite"


def find_script(conn: sqlite3.Connection, sqasm_path: str) -> sqlite3.Row | None:
    candidates = [norm_path(sqasm_path)]
    if not candidates[0].endswith(".sqasm"):
        candidates.append(candidates[0] + ".sqasm")
    if not candidates[0].endswith(".b64.sqasm"):
        candidates.append(candidates[0] + ".b64.sqasm")

    for candidate in candidates:
        row = conn.execute("SELECT * FROM scripts WHERE path_norm = ?", (candidate,)).fetchone()
        if row:
            return row
    like = f"%{candidates[0]}%"
    return conn.execute("SELECT * FROM scripts WHERE path_norm LIKE ? ORDER BY path LIMIT 1", (like,)).fetchone()


def enrich_from_db(ir: dict[str, Any], db_path: Path) -> None:
    if not db_path.exists():
        ir["evidence"] = {"db_path": str(db_path), "available": False}
        return

    conn = sqlite3.connect(str(db_path))
    conn.row_factory = sqlite3.Row
    try:
        script_row = find_script(conn, ir["sqasm_path"])
        if not script_row:
            ir["evidence"] = {"db_path": str(db_path), "available": True, "script_found": False}
            return

        ir["evidence"] = {
            "db_path": str(db_path),
            "available": True,
            "script_found": True,
            "script_id": script_row["id"],
            "path": script_row["path"],
            "input_path": script_row["input_path"],
            "function_count": script_row["function_count"],
            "instruction_count": script_row["instruction_count"],
            "literal_count": script_row["literal_count"],
            "call_count": script_row["call_count"],
            "resource_ref_count": script_row["resource_ref_count"],
        }

        function_rows = conn.execute(
            "SELECT id, ordinal, name, line_no, instruction_count FROM script_functions WHERE script_id = ?",
            (script_row["id"],),
        ).fetchall()
        function_id_to_ordinal = {row["id"]: row["ordinal"] for row in function_rows}
        function_by_ordinal = {fn["ordinal"]: fn for fn in ir["functions"]}
        for row in function_rows:
            fn = function_by_ordinal.get(row["ordinal"])
            if fn is None:
                continue
            fn["evidence"] = {
                "function_id": row["id"],
                "name": row["name"],
                "line_no": row["line_no"],
                "instruction_count": row["instruction_count"],
            }

        calls_by_function: dict[int, list[dict[str, Any]]] = defaultdict(list)
        call_rows = conn.execute(
            """
            SELECT function_id, call_name, literal_index, pc, source_line, line_no, classification
            FROM script_calls
            WHERE script_id = ?
            ORDER BY function_id, source_line, pc, call_name
            """,
            (script_row["id"],),
        ).fetchall()
        for row in call_rows:
            ordinal = function_id_to_ordinal[row["function_id"]]
            calls_by_function[ordinal].append(
                {
                    "name": row["call_name"],
                    "literal_index": row["literal_index"],
                    "pc": row["pc"],
                    "source_line": row["source_line"],
                    "line_no": row["line_no"],
                    "classification": row["classification"],
                    "source": "evidence_graph",
                }
            )

        resource_rows = conn.execute(
            """
            SELECT l.function_id, l.literal_index, l.value, l.kind, l.line_no,
                   r.path AS manifest_path, r.pack, r.ext, r.size
            FROM script_literals l
            LEFT JOIN resources r ON r.id = l.resource_id
            WHERE l.script_id = ?
              AND l.kind IN ('resource_path', 'resource_stem')
            ORDER BY l.function_id, l.line_no, l.literal_index
            """,
            (script_row["id"],),
        ).fetchall()
        resources_by_function: dict[int, list[dict[str, Any]]] = defaultdict(list)
        literal_kind_by_function: dict[tuple[int, int], str] = {}
        for row in resource_rows:
            ordinal = function_id_to_ordinal[row["function_id"]]
            item = {
                "literal_index": row["literal_index"],
                "value": row["value"],
                "kind": row["kind"],
                "line_no": row["line_no"],
                "manifest_path": row["manifest_path"],
                "pack": row["pack"],
                "ext": row["ext"],
                "size": row["size"],
            }
            if row["kind"] == "resource_stem":
                candidates = conn.execute(
                    """
                    SELECT path, pack, ext, size
                    FROM resources
                    WHERE path_norm LIKE ? || '%'
                      AND ext IN ('.dds', '.b64', '.fmp', '.se', '.ogg', '.sge', '.rcm')
                    ORDER BY path
                    LIMIT 80
                    """,
                    (norm_path(row["value"]),),
                ).fetchall()
                item["candidates"] = [
                    {"path": c["path"], "pack": c["pack"], "ext": c["ext"], "size": c["size"]} for c in candidates
                ]
            resources_by_function[ordinal].append(item)
            literal_kind_by_function[(ordinal, row["literal_index"])] = row["kind"]

        for fn in ir["functions"]:
            ordinal = fn["ordinal"]
            if ordinal in calls_by_function:
                fn["calls"] = calls_by_function[ordinal]
            if ordinal in resources_by_function:
                fn["resource_refs"] = resources_by_function[ordinal]
            for literal in fn["literals"]:
                kind = literal_kind_by_function.get((ordinal, literal["index"]))
                if kind:
                    literal["kind"] = kind
    finally:
        conn.close()


def summarize(ir: dict[str, Any]) -> dict[str, Any]:
    function_count = len(ir["functions"])
    instruction_count = sum(len(fn["instructions"]) for fn in ir["functions"])
    literal_count = sum(len(fn["literals"]) for fn in ir["functions"])
    call_count = sum(len(fn["calls"]) for fn in ir["functions"])
    resource_ref_count = sum(len(fn["resource_refs"]) for fn in ir["functions"])

    boundary_counter: Counter[str] = Counter()
    classification_counter: Counter[str] = Counter()
    for fn in ir["functions"]:
        for call in fn["calls"]:
            classification = call.get("classification") or "unknown"
            classification_counter[classification] += 1
            if classification in {"engine_boundary_candidate", "type_or_engine_candidate", "confirmed-native"}:
                boundary_counter[call["name"]] += 1

    state_candidates = sorted(
        {candidate for fn in ir["functions"] for candidate in fn["state_candidates"] if candidate}
    )

    summary = {
        "function_count": function_count,
        "instruction_count": instruction_count,
        "literal_count": literal_count,
        "call_count": call_count,
        "resource_ref_count": resource_ref_count,
        "classification_counts": dict(sorted(classification_counter.items())),
        "boundary_calls": dict(sorted(boundary_counter.items())),
        "state_candidates": state_candidates,
    }
    ir["summary"] = summary
    return summary


def output_paths(out_dir: Path, sqasm_path: str) -> tuple[Path, Path]:
    safe = sqasm_path.replace("\\", "/")
    json_path = out_dir / (safe + ".json")
    state_path = out_dir / (safe + ".state_machine.md")
    return json_path, state_path


def write_json(path: Path, ir: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(ir, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def write_state_machine(path: Path, ir: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    summary = ir.get("summary") or summarize(ir)
    boundary_calls = summary.get("boundary_calls", {})
    state_candidates = summary.get("state_candidates", [])

    lines = [
        f"# State Machine Candidates: {ir['sqasm_path']}",
        "",
        "## Summary",
        "",
        f"- Functions: {summary['function_count']}",
        f"- Instructions: {summary['instruction_count']}",
        f"- Calls: {summary['call_count']}",
        f"- Resource refs: {summary['resource_ref_count']}",
        "",
        "## State Candidates",
        "",
    ]
    if state_candidates:
        lines.extend(f"- `{name}`" for name in state_candidates)
    else:
        lines.append("(none)")

    lines.extend(["", "## Boundary Calls", ""])
    if boundary_calls:
        lines.extend("| Name | Count |")
        lines.extend(["| --- | ---: |"])
        for name, count in sorted(boundary_calls.items(), key=lambda item: (-item[1], item[0])):
            lines.append(f"| `{name}` | {count} |")
    else:
        lines.append("(none)")

    lines.extend(["", "## Functions With State Candidates", ""])
    found = False
    for fn in ir["functions"]:
        if not fn["state_candidates"]:
            continue
        found = True
        name = fn["name"] if fn["name"] is not None else "None"
        lines.append(f"- `#{fn['ordinal']} {name}`: " + ", ".join(f"`{c}`" for c in fn["state_candidates"]))
    if not found:
        lines.append("(none)")

    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("sqasm", type=Path, help="Path to a .sqasm disassembly")
    parser.add_argument("--db", type=Path, default=default_db_path(), help="Project2 evidence graph SQLite path")
    parser.add_argument("--no-db", action="store_true", help="Disable evidence graph enrichment")
    parser.add_argument("--out-dir", type=Path, help="Write JSON and state summary under this directory")
    parser.add_argument("--json-out", type=Path, help="Explicit JSON output path")
    parser.add_argument("--state-out", type=Path, help="Explicit state-machine markdown output path")
    parser.add_argument("--stdout", action="store_true", help="Print JSON to stdout")
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    if not args.sqasm.exists():
        parser.error(f"sqasm not found: {args.sqasm}")

    ir = parse_sqasm(args.sqasm)
    if not args.no_db:
        enrich_from_db(ir, args.db)
    summary = summarize(ir)

    if args.out_dir:
        json_path, state_path = output_paths(args.out_dir, ir["sqasm_path"])
        write_json(json_path, ir)
        write_state_machine(state_path, ir)
    if args.json_out:
        write_json(args.json_out, ir)
    if args.state_out:
        write_state_machine(args.state_out, ir)
    if args.stdout or not (args.out_dir or args.json_out or args.state_out):
        print(json.dumps(ir, indent=2, ensure_ascii=False))
    else:
        print(
            "sqir: "
            f"{ir['sqasm_path']} -> "
            f"{summary['function_count']} functions, "
            f"{summary['instruction_count']} instructions, "
            f"{summary['call_count']} calls, "
            f"{summary['resource_ref_count']} resource refs"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
