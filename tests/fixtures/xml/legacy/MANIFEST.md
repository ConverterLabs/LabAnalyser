# Anonymized legacy experiment fixtures

## Provenance and scope

These files are structurally faithful, anonymized copies of a corpus supplied
externally on 2026-08-10. They do not originate from a repository commit or
tag. The source XML has no schema or release-version field, so no LabAnalyser
release can be assigned reliably. Timestamp-like form names range from 2024 to
2026, but are not release evidence.

The source files were read only. The original directory is outside this
repository and is never used as a test input.

## File manifest

| Source (external) | Original SHA-256 | Fixture | Fixture SHA-256 | Notes |
| --- | --- | --- | --- | --- |
| source experiment A | `9205CAADB30BF50DF05EE471627583398907AAF064559C4FF26EB70D0919CDA9` | `legacy-small.LAexp` | `87E538A5B305BDFEF146C8A9261FA99731FFD96D8F9CBEC3488D7FA11F637279` | 2 forms, 1 device, 3 figure windows, 80 widgets, 29 connections |
| source experiment B | `1A127647528D5F33804ECF1167CBB70D55B7929C25559C34F7F0E7B8A7EBBEEA` | `legacy-multiform.LAexp` | `DDD7FD2F6D084497D97EF5181A2E5183BB95107D8D1204FE588E6B6D31996CF7` | 5 forms, 1 device, 2 figure windows, 420 widgets, 75 connections |
| source experiment C | `4352A890DF4D0FE1ABD2324B5E3336F582155FCAFC7AD71DD3678FC217093E5D` | `legacy-2cells.LAexp` | `576CB885EA2FDF4762E4B9E4CE65B412B442CF6642E5320AB0278823F152854A` | 6 forms, 1 device, 11 figure windows, 901 widgets, 91 connections |
| first referenced device descriptor | `A88601B11065365F894C29EE8A7B19EE588FB906F1980DD99A0800B6A94CBF5E` | `Device_legacy-a.LAdev` | `2DA4A61EE2256C2107F486492E3EF3BE21E97A609D627F191FAD70A8BA4A545B` | used by the first and third experiment |
| second referenced device descriptor | `F9EAE297A31ABD6DB1511103B6F323911A44F9BF94278BC6BB0BDB02EE330380` | `Device_legacy-b.LAdev` | `AB6B7E85972BC98D9D93556CD211BC77790F3E8F107FCAF06C5E32C177903DD3` | used by the second experiment |

## Anonymization rules

- Absolute user-profile paths retain their absolute Windows form but use a
  neutral user and project hierarchy.
- Relative paths retain their relative form, including the two-parent path in
  the second experiment.
- Private network addresses use TEST-NET-1 documentation addresses.
- Host- and device-descriptor names use neutral `legacy-*` names.
- The historical proprietary plugin reference is replaced by the deliberately
  absent `HistoricalPluginMissing.dll`.
- Project- and installation-specific directory components are replaced by
  neutral components. XML elements, attributes, ordering, nesting, optional
  omissions, widget/connection counts, figure-window structure and form file
  names are retained.
- The Base64 `State` payloads are retained byte-for-byte: the source scan found
  no user-profile path or username in their decoded UTF-16/UTF-8 views.

`XML_LEGACY_001` through `XML_LEGACY_004` copy fixtures only to
`QTemporaryDir` locations. `XML_LEGACY_005` alters only its temporary device
descriptor's `DevicePlugin` attribute to point to the runtime-built compatible
test plugin. Every legacy test hashes the committed fixture before and after
execution.
