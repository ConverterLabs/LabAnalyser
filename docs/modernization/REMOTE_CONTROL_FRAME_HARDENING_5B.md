# Remote-control frame hardening 5B

## 5B.2a validation primitives

This explicitly approved first security slice adds private, QObject-free
validation primitives only. `RemoteControlServer` still uses the legacy
`TakeCompleteFrame()` and `DecodeCompleteFrame()` paths; no server socket,
Messenger, mapper, payload or reply behavior changes in this commit.

`RemoteControlFrameSplitter::TakeFrame()` returns one of:

| Result | Condition | Buffer effect |
| --- | --- | --- |
| `Incomplete` | Fewer than four prefix bytes, or a plausible 16-byte to 1 MiB frame whose bytes are not all present | Preserved unchanged |
| `Complete` | A complete frame whose native `uint32_t totalSize` is within bounds | One frame removed, later bytes preserved |
| `InvalidPrefix` | `totalSize < 16` or `totalSize > 1,048,576` | Entire current buffer cleared |

The clear is intentional: direct callers cannot loop repeatedly on the same
invalid prefix. It is not yet the approved server action. The later server
delegation must additionally discard the current connection after an
`InvalidPrefix`.

`RemoteControlProtocol::DecodeValidatedFrame()` reads all `uint32_t` fields
with bounded `memcpy`, preserves native host byte order, and returns
`DecodeStatus::Invalid` unless the frame is at least 16 bytes, its declared
size equals `frame.size()`, `idLength >= 1`, the overflow-safe
`15 + idLength + payloadLength` sum equals the declared size, and the last ID
byte is NUL. A structurally valid unknown three-byte command is `Valid` with
`Command::Unknown`, not `Invalid`. `HasNumericSetPayload()` makes the later
minimum eight-byte numeric-set prerequisite available without using it in the
server yet.

## Direct safety evidence

| Test ID | Result |
| --- | --- |
| `TCP_017` | 0--3 buffered bytes are `Incomplete`; both input buffer and caller output remain unchanged. |
| `TCP_018` | Native prefixes 0 through 15 are `InvalidPrefix`, clear once, then become `Incomplete` rather than looping. |
| `TCP_019` | A plausible bounded partial frame remains buffered as `Incomplete`. |
| `TCP_020` | Exactly 1 MiB is plausible without allocating a payload; 1 MiB + 1 is `InvalidPrefix`. |
| `TCP_021` | Valid known and unknown commands are distinguished from zero ID length, missing ID NUL, declared-size mismatch, inconsistent sum and oversized field values; the prepared numeric-set check rejects a seven-byte payload. |

The focused `RemoteControlContractTests` build and run both exited with code
0. It now has 22 test functions (`TCP_001..TCP_021` plus the legacy
frame-splitter test); Qt Test reports 24 passing lifecycle/test entries and no
failures. The incremental Release compilation of `RemoteControlFrameSplitter`
and `RemoteControlProtocol` also exited 0. Existing warnings in untouched
`RemoteControlServer.cpp` remain outside this slice.

## Deferred server decision

The next approved server slice must delegate only after the direct primitive
contract is stable: `InvalidPrefix` clears current state and closes the current
connection; a complete structurally invalid frame is dropped with no
`MessageSender` emission or reply while the connection stays open. It must not
alter valid `TCP_001..TCP_016` bytes or the valid-unknown-command ignore
contract. String/StringList loss, Selection/container mutation, independent
multi-client sessions, rate limiting, authentication, TLS and protocol-version
work remain outside 5B.2a.
