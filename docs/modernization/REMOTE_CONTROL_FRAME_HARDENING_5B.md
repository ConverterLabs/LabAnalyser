# Remote-control frame hardening 5B

## 5B.2a primitives and 5B.2b server activation

5B.2a added private, QObject-free validation primitives. 5B.2b is the
explicitly approved server activation: `RemoteControlServer` now consumes only
`TakeFrame()`/`DecodeValidatedFrame()` results. It does not alter native wire
byte order, valid response bytes, Messenger routing, mapper/container semantics
or the known valid String/StringList/Selection payload behavior.

`RemoteControlFrameSplitter::TakeFrame()` returns one of:

| Result | Condition | Buffer effect |
| --- | --- | --- |
| `Incomplete` | Fewer than four prefix bytes, or a plausible 16-byte to 1 MiB frame whose bytes are not all present | Preserved unchanged |
| `Complete` | A complete frame whose native `uint32_t totalSize` is within bounds | One frame removed, later bytes preserved |
| `InvalidPrefix` | `totalSize < 16` or `totalSize > 1,048,576` | Entire current buffer cleared |

The clear is intentional: callers cannot loop repeatedly on the same invalid
prefix. In the active server boundary, `InvalidPrefix` immediately calls
`abort()` on the still-present current socket and returns. The existing
`disconnected()` path remains solely responsible for `ResetIfCurrent()` and
deferred QObject deletion.

`RemoteControlProtocol::DecodeValidatedFrame()` reads all `uint32_t` fields
with bounded `memcpy`, preserves native host byte order, and returns
`DecodeStatus::Invalid` unless the frame is at least 16 bytes, its declared
size equals `frame.size()`, `idLength >= 1`, the overflow-safe
`15 + idLength + payloadLength` sum equals the declared size, and the last ID
byte is NUL. A structurally valid unknown three-byte command is `Valid` with
`Command::Unknown`, not `Invalid`. `HasNumericSetPayload()` enforces the
approved eight-byte numeric-set prerequisite before the server reads a
`double` with `memcpy`.

## Direct safety evidence

| Test ID | Result |
| --- | --- |
| `TCP_017` | 0--3 buffered bytes are `Incomplete`; both input buffer and caller output remain unchanged. |
| `TCP_018` | Native prefixes 0 through 15 are `InvalidPrefix`, clear once, then become `Incomplete` rather than looping. |
| `TCP_019` | A plausible bounded partial frame remains buffered as `Incomplete`. |
| `TCP_020` | Exactly 1 MiB is plausible without allocating a payload; 1 MiB + 1 is `InvalidPrefix`. |
| `TCP_021` | Valid known and unknown commands are distinguished from zero ID length, missing ID NUL, declared-size mismatch, inconsistent sum and oversized field values; the prepared numeric-set check rejects a seven-byte payload. |
| `TCP_022` | A complete structurally invalid frame produces no signal or reply; a following valid `get` on the same connection returns the existing exact bytes. |
| `TCP_023` | Prefixes 0, 15 and 1 MiB + 1 close the current connection without signal or reply, clear frame state, and permit a fresh valid connection. |
| `TCP_024` | Numeric `set` payloads 0--7 bytes are discarded without mutation, signal or reply; valid `set` and exact recovery `get` remain usable on that connection. |
| `TCP_025` | A structurally valid unknown command remains ignored without signal/reply and leaves the connection usable. |
| `TCP_026` | A coalesced complete structural-invalid frame is discarded; the following valid frame emits exactly one existing-order `set` signal. |

The focused `RemoteControlContractTests` build and run both exited with code
0. It now has 27 test functions (`TCP_001..TCP_026` plus the legacy
frame-splitter test). The incremental Release compilation is recorded for the
affected remote-control sources. The only observed warning is the pre-existing
unused `displayError` parameter; no unrelated cleanup was made.

## Active server behavior and remaining limits

`Incomplete` leaves received bytes in the splitter and performs no response or
signal. A complete structural-invalid frame is already bounded, is removed by
the splitter, and is dropped with no response/signal while the connection stays
open; later known frame boundaries from the same buffer remain processable.
Before each reply write the server reads the current `QPointer` again and
returns safely if it is absent. The old unaligned header and numeric-payload
pointer casts in `HeaderReceived()` were removed; `uint32_t` fields are decoded
by the protocol and a numeric `double` is copied only after the eight-byte
check.

The 1 MiB limit bounds frame buffering but does not yet limit otherwise valid
reply construction or application-level payload contents. String/StringList
last-byte loss, Selection/container mutation, independent multi-client
sessions, rate limiting, authentication, TLS and protocol-version work remain
outside 5B.

## Package C: encoded reply bound

`c69ffd3` extends the same private protocol boundary to outbound `get`
responses. `MaxEncodedReplySize` is 1 MiB including the legacy five-byte
type/count header. Checked `quint64` arithmetic rejects a reply before any
allocation, narrowing conversion or partial socket write. The server then
aborts the current socket and emits neither a reply nor a Messenger event; its
normal disconnect cleanup permits a fresh connection. Existing native-order
reply bytes at or below the limit are unchanged. `TCP_030` directly covers the
maximum padded reply and overflow-safe rejection; `TCP_031` covers loopback
abort and recovery. This does not introduce pagination, an input-format
change, a payload semantic change or an application-level reply policy.
