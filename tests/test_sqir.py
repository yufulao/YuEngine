from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from tools.sqir import parse_sqasm, summarize


SAMPLE_SQASM = """; Squirrel 2.2.4 closure disassembly
; input: script/menu/titlemenu.b64

function #0 'main' source='../resource/script/menu/titleMenu.nut' offset=0x12
stack=4 generator=False varargs=False
parameters=['this']
literals:
  [0000] 'GetSaveList'
  [0001] 'menu/title/title_back'
  [0002] 'startGame4Menu'
locals:
  r0 'this' ops=0..3
instructions:
  0000    L61 _OP_PREPCALLK        a0=  2 a1=          0 a2=  0 a3=  3 ; literal[0]='GetSaveList'
  0001    L61 _OP_CALL             a0=  2 a1=          2 a2=  3 a3=  1
  0002    L62 _OP_LOAD             a0=  1 a1=          1 a2=  0 a3=  0 ; literal[1]='menu/title/title_back'
  0003    L63 _OP_CLOSURE          a0=  2 a1=          1 a2=  0 a3=  0 ; function[1]='startGame4Menu'

function #1 'startGame4Menu' source='../resource/script/menu/titleMenu.nut' offset=0x30
stack=3 generator=False varargs=False
parameters=['this']
literals:
  [0000] 'MakeNewGame'
  [0001] 'StartGame'
locals:
  r0 'this' ops=0..3
instructions:
  0000   L135 _OP_PREPCALLK        a0=  2 a1=          0 a2=  0 a3=  3 ; literal[0]='MakeNewGame'
  0001   L135 _OP_CALL             a0=  2 a1=          2 a2=  3 a3=  1
  0002   L140 _OP_PREPCALLK        a0=  2 a1=          1 a2=  0 a3=  3 ; literal[1]='StartGame'
  0003   L140 _OP_CALL             a0=  2 a1=          2 a2=  3 a3=  1
"""


class SqirTest(unittest.TestCase):
    def test_parse_functions_calls_and_closure_binding(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "titlemenu.b64.sqasm"
            path.write_text(SAMPLE_SQASM, encoding="utf-8")

            ir = parse_sqasm(path)
            summary = summarize(ir)

        self.assertEqual(ir["script"], "script/menu/titlemenu.b64")
        self.assertEqual(ir["sqasm_path"], "script/menu/titlemenu.b64.sqasm")
        self.assertEqual(summary["function_count"], 2)
        self.assertEqual(summary["instruction_count"], 8)

        calls = [call["name"] for fn in ir["functions"] for call in fn["calls"]]
        self.assertEqual(calls, ["GetSaveList", "MakeNewGame", "StartGame"])

        main = ir["functions"][0]
        self.assertEqual(main["resource_refs"][0]["value"], "menu/title/title_back")
        self.assertEqual(main["resource_refs"][0]["kind"], "resource_stem")
        self.assertEqual(main["closure_bindings"][0]["function_name"], "startGame4Menu")


if __name__ == "__main__":
    unittest.main()
