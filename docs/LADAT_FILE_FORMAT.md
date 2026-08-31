# LabAnalyser `.LAdat` file format (versions 1 and 2)

This document specifies the native LabAnalyser data archive written by
`LabDataArchive::ExportAll()` and read by `LabDataArchive::Import()`.
It is intended as the implementation guide for a compatible importer.  The
format is deliberately independent of a loaded device plugin: it contains
channels, scalar values, text, selections, parameters and time-series data.

## Scope and compatibility

- Extension: `.LAdat`.
- Current writer format: `LabAnalyserData`, version `2`.
- The reader remains compatible with version `1` archives.
- The file is a **binary container with a compact UTF-8 JSON header**.  It is
  not an XML, MAT or HDF5 file.
- Integer and floating-point bytes are little-endian in version 1.  Do not
  infer a network-byte-order conversion from this format.
- Strings are UTF-8 and are length-prefixed; they are not NUL-terminated.
- Unknown future versions must be rejected rather than guessed.

## File layout

All offsets below are byte offsets from the beginning of the file.

| Offset | Field | Encoding |
|---:|---|---|
| 0 | magic | ASCII bytes `LABANALYSER-LADAT-1\n` or `LABANALYSER-LADAT-2\n` (20 bytes, no terminator) |
| 20 | `headerSize` | unsigned 64-bit little-endian integer |
| 28 | JSON header | exactly `headerSize` UTF-8 bytes; compact JSON, no trailing terminator required |
| `28 + headerSize` | payload area | concatenated channel payloads |

`headerSize` is limited by the current reader to 16 MiB.  A channel's
`offset` is relative to the start of the payload area, not to the file start.
The importer validates every offset and stored payload length against the file
boundaries before seeking to the payload.

## JSON header

The exported header has this shape (whitespace shown only for readability):

```json
{
  "format": "LabAnalyserData",
  "version": 2,
  "createdUtc": "2026-08-28T10:15:30.123Z",
  "byteOrder": "littleEndian",
  "channels": [
    {
      "id": "Device::Buffered::Voltage",
      "dataType": "double",
      "category": "Data",
      "stateDependency": "",
      "alias": "Voltage",
      "min": -10.0,
      "max": 10.0,
      "valueType": "DataPair",
      "codec": "zstd",
      "offset": "0",
      "storedBytes": "31",
      "rawBytes": "48"
    }
  ]
}
```

The fields common to both versions are:

| Field | Type | Meaning |
|---|---|---|
| `format` | string | Must equal `LabAnalyserData`. |
| `version` | number | Must equal the version encoded by the magic. |
| `channels` | array | The ordered channel descriptors. |
| `id` | string | Original LabAnalyser channel/parameter identifier. |
| `dataType` | string | LabAnalyser `InterfaceData` data type; preserve it. |
| `category` | string | LabAnalyser container category; preserve it. |
| `stateDependency` | string | State dependency string; may be empty. |
| `alias` | string | User-visible alias; may be empty. |
| `min`, `max` | JSON number | Stored limits. |
| `valueType` | string | Selects the binary payload layout below. |
| `offset` | decimal string | Unsigned 64-bit payload-relative offset; strings avoid JSON-number precision loss. |

Version-specific storage fields:

| Version | Fields | Meaning |
|---|---|---|
| 1 | `bytes` | Raw payload length.  All payloads are uncompressed. |
| 2 | `codec`, `storedBytes`, `rawBytes` | `codec` is `none` or `zstd`; the two lengths are the stored and decoded payload lengths. |

The exporter writes channels in the lexical order of the DataManagement
container map.  An importer must not rely on this order for channel identity;
use `id`.

## Common binary primitives

| Primitive | Layout |
|---|---|
| `u64` | 8-byte unsigned little-endian integer |
| `f64` | IEEE-754 binary64 bits stored as a `u64` little-endian value |
| `f32` | IEEE-754 binary32 bits stored little-endian |
| `text` | `u64 byteCount`, followed by exactly `byteCount` UTF-8 bytes |

The active reader rejects an individual text length above `INT_MAX`, a
`DataPair` time or data count above 1,000,000,000 and a list/selection item
count above 1,000,000.  A new importer should enforce at least these limits
before allocating memory.

## Payload layouts

The `valueType` field determines the following exact payload content.

| `valueType` | Payload |
|---|---|
| `DataPair` | `u64 timeCount`, `u64 dataCount`, `f64 timeOffset`, then `timeCount` × `f64` time samples, then `dataCount` × `f64` data samples. Counts are intentionally independent; unequal vectors are representable. |
| `QString` | one `text` value |
| `QStringList` | `u64 itemCount`, then `itemCount` × `text` |
| `GuiSelection` | selected-item `text`, `u64 itemCount`, then `itemCount` × `text` (the list is the permitted selection set) |
| `bool` | one byte: `0` is false, any nonzero value is interpreted as true by the current reader |
| `int8_t`, `uint8_t` | one byte |
| `int16_t`, `uint16_t` | 2-byte little-endian integer |
| `int32_t`, `uint32_t` | 4-byte little-endian integer |
| `int64_t`, `uint64_t` | 8-byte little-endian integer |
| `float` | one `f32` |
| `double` and other current numeric fallback types | one `f64` |

For version 2, the exporter uses Zstandard level 3 only when a payload is at
least 4096 bytes and the compressed representation is strictly smaller.
Otherwise it stores the original payload with `codec: "none"`.  The current
reader limits both `storedBytes` and `rawBytes` to 1 GiB, verifies successful
decompression, and requires the decoded payload parser to consume all bytes.

## Current LabAnalyser import behaviour

The current application imports all channels even with no plugin loaded.  It
creates a new dataset root at import time:

```
Export_<input-file-base-name>_<UTC yyyyMMdd_HHmmss>::<stored id>
```

Within this root it publishes each decoded `InterfaceData` value, then restores
the channel's min/max values and alias.  Consequently an imported archive can
be plotted and analysed without reconnecting the source hardware.  The exact
timestamped root is intentionally not stable and must not be used as a
cross-file identifier.

The importer rejects a bad magic, oversized/invalid header, unsupported
format/version, invalid channel offsets or invalid payload encoding.  It does
not rewrite the source archive.

## Writer guidance

To produce a compatible version-2 archive:

1. Serialize each payload with the layouts above.
2. Optionally encode a payload with Zstandard and select `zstd` only if it is
   smaller than the raw representation; otherwise use `none`.
3. Put the cumulative, payload-relative offset, codec, stored length and raw
   length in each JSON channel descriptor as decimal strings.
4. Write the version-2 magic, the little-endian JSON byte count, the UTF-8 JSON header,
   then the payloads in descriptor order.
5. Write atomically when possible; LabAnalyser itself uses `QSaveFile`.

Do not change byte order or string encoding under version 2.  Retain the
version-1 reader whenever a writer is upgraded further.

## Reference implementation

The authoritative implementation is
[`src/Export/LabDataArchive.cpp`](../src/Export/LabDataArchive.cpp).  Import
and export are exposed through `UIDataManagementSetClass` and
`ProjectIoCoordinator`; the GUI actions are named **Export LabAnalyser Data**
and **Import LabAnalyser Data**.
