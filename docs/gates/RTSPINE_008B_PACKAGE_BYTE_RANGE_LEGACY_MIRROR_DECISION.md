# RTSPINE-008B: Package Byte-Range Legacy Mirror Decision

Status: Contract decision accepted
Related implementation: `ea4dc0f78fc12dc1366f999e218dd50f529e92d0`
Related focused QA blocker: `4b2ac3ca COMPLETE-FAIL`
Related docs task: `9d04a256-80d7-4675-9c8e-4a10a98d7d96`
Source baseline: `dd9f10e89b9540816e64d7213e9d5a410c8667f0`

## Decision

RTSPINE-008B accepts `archive_byte_offset` and `archive_byte_size` as the only
authoritative Package shipped-content pressure byte range.

`PackageEntryDescriptor::byte_offset`, `PackageEntryDescriptor::byte_size`,
`PackageLoadPlanRecord::byte_offset`, and `PackageLoadPlanRecord::byte_size`
may remain as legacy mirror fields while Streaming, RuntimeAsset, and existing
tests still consume them. They are compatibility mirrors only. They must not be
counted as shipped-content pressure evidence, pressure budget proof, or archive
range authority.

Physical deprecation or removal of the legacy mirror fields is deferred to a
later Streaming/RuntimeAsset bridge gate if that gate names the exact migration
surface and evidence rows.

## Evidence Rule

RTSPINE-008B focused QA must evaluate pressure evidence through:

- `archive_byte_offset`;
- `archive_byte_size`;
- Package registry validation before mutation;
- Package load-plan propagation of the archive range;
- Package artifact round-trip of the archive range.

Rows that only inspect `byte_offset` or `byte_size` can prove legacy mirror
compatibility, but they cannot prove RTSPINE-008B pressure coverage.

## Scope Boundary

This decision does not open:

- Streaming implementation changes;
- RuntimeAsset packaged-validation changes;
- Resource external payload or payload-window records;
- File/VFS ranged IO;
- Package hash or dependency integrity work;
- CMake, broad CTest, editor, Web, UI, external authoring, original-game
  adapters, or runtime visual evidence matrix edits.

RTSPINE-008C and later gates may use this decision as an input, but they must
name their own file surfaces and evidence thresholds before implementation.
RTSPINE-008C subsequently did that in a clearly disjoint Package-only lane and
is PASS at `origin/main@d18f1679ebd389ecec506055764602591f5b9ab6`: focused QA
task `ba135e38-b73e-4294-b449-97a04b33b982` reports `YuPackageTests` build PASS,
`^Package_` `35/35` PASS, exact new integrity rows `2/2` PASS, and no broad/full
CTest. This 008B decision still does not authorize File/VFS, Resource, or
RuntimeAsset packaged-validation work.
RTSPINE-008D later opened File/VFS ranged IO through a separate File-only gate
at `origin/main@c67e9710ab39f49ea01f0c194d2e5b44cbf3b97e`; focused QA task
`aebd28c5-f688-4ccc-abaf-1a3bd61879cb` reports `YuFileTests` build PASS,
`^File_` `23/23` PASS, ranged subset `4/4` PASS, and no broad/full CTest. That
008D evidence does not expand this 008B Package byte-range decision.
RTSPINE-008E later opened Resource payload window/reference budget through a
separate Resource-only gate at
`origin/main@8bb8eff9c98d2a0aa5050c5da6ad94049fa894be`; focused QA task
`b4fa51c3-aefc-4714-b5d8-062f8a933ac9` reports `YuResourceTests` build PASS,
Resource window/reference discovery exactly `7` rows, execution `7/7` PASS, and
no broad/full CTest. That 008E evidence does not expand this 008B Package
byte-range decision.
RTSPINE-008F later opened Package dependency closure and budgeted load plans
through a separate Package-only gate at
`origin/main@8509f7e1b6ba15e79c574357a465ddfff4d80e10`; focused QA task
`4f199c8e-99a4-43b4-a776-8960285ffdaf` reports `YuPackageTests` build PASS,
exact 008F rows `4/4` PASS, `^Package_` focused suite `39/39` PASS,
diff/hygiene/scope scan PASS, and no broad/full CTest. That 008F evidence keeps
`archive_byte_offset` and `archive_byte_size` as the authoritative pressure
range and does not authorize RuntimeAsset packaged validation or RTSPINE-008G/H.
