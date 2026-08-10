# Remote-control refactoring plan — phase 4H

## Scope and preservation rule

This plan covers only `RemoteControlServer` and its immediate boundary to the
existing `MessengerClass`/`DataManagementSetClass` graph.  The TCP wire format,
the loopback-only binding policy, message order, native value representation,
and the existing data-container observations are compatibility contracts.  No
protocol correction is part of this plan.  In particular, unsafe malformed
frame paths remain exclusions until separately characterized and approved.

## Current responsibility and object graph

`RemoteControlServer` is a `QWidget` that combines all of the following:

1. binds a value-member `QTcpServer` to the first free loopback port starting
   at 4080 and exposes the selected port as `m_port - 1`;
2. accepts a connection and stores the most recently accepted
   `QTcpSocket*` in `tcpServerConnection`;
3. owns the per-current-connection receive state (`DataBuffer`,
   `NextDataBuffer`, `ReceivedID`, `IsHeaderReceived`, and unused byte/debug
   counters);
4. parses native binary requests, reads the non-owning
   `std::map<QString, ToFormMapper*>*` supplied by the caller, emits `set`
   requests, and writes `get` replies; and
5. connects its `MessageSender(QString, QString, InterfaceData)` signal in
   `MainWindow` to `MessengerClass::MessageTransmitter`.

`MainWindow` creates the server with `GetLogic()->GetContainerPointer()` and
manually deletes it in its destructor.  The server neither owns the container
map nor the mappers.  `tcpServer` is a value member and is destroyed with the
server.  The source does not explicitly close, delete, reparent, or
`deleteLater()` accepted sockets; `tcpServerConnection` is only a raw pointer
to the latest accepted socket and is overwritten by the next connection.
Consequently, the exact accepted-socket ownership and stale-pointer lifetime
must remain a Qt transport concern until a dedicated ownership slice has
characterized it more deeply.  `TCP_001` and `TCP_007` show clean test-process
shutdown, but do not prove a multi-client ownership policy.

The downstream dispatch is intentionally indirect:

```text
TCP client -> RemoteControlServer parse/read -> MessageSender("set", id, data)
           -> MessengerClass::MessageTransmitter
           -> MessageReceiver intents, then exactly one Messenger MessageSender
           -> manager/widget/plugin connections
```

For a remote `set`, the server works on a copied `InterfaceData` and emits it;
the server does not itself update the referenced mapper.  The observed manager
update therefore depends on the already-characterized Messenger signal graph,
not on the socket class.

## Framing, parsing, dispatch and byte contracts

For safe frames covered by `TCP_001`–`TCP_007`, a request is native little
endian and has no line ending:

```text
uint32 totalSize
char[3] command                 // "set" or "get", Latin-1
uint32 idLength
uint32 payloadLength
char[idLength] id including NUL
char[payloadLength] payload
```

`HeaderReceived()` appends `readAll()` to `DataBuffer`, waits until the
declared frame size is present, preserves a remainder in `NextDataBuffer`, and
then processes complete frames in arrival order.  The server uses
`QString::fromLatin1`; UTF-8 is not a declared protocol conversion.  Numeric
payloads are native `double` bytes.  String/list/selection payloads are
Latin-1 bytes.  The current implementation uses native reinterpret-casts and
does not validate a buffer before reading its first `uint32`; short, zero,
oversized, inconsistent-length and unaligned frames are therefore deliberately
not contracts.

`set` emits one `MessageSender` for an existing mapped ID; `get` writes a reply
directly to the currently stored socket:

```text
byte type + uint32 elements + payload
type 0: one numeric double, or concatenated time then data doubles
type 1: element count and element-count*8 string storage bytes
```

Unknown `get` IDs return either matching map keys joined by `|` as type 1, or
the five zero bytes (`type=0`, `elements=0`).  A null map yields the same empty
reply.  Unknown commands emit no server signal and have no bounded reply in
the tested interval.

The intended extraction must preserve native endianness, NUL accounting,
Latin-1 conversion, type byte, string padding, key-suggestion output, direct
reply order, and the lack of line terminators.  It must not silently replace
this format with JSON, Qt streams, UTF-8, network byte order, or a new command
grammar.

## Existing evidence

| Test ID | Observed contract | Refactoring protection |
| --- | --- | --- |
| `TCP_001` | Bind at the first available loopback port at/above 4080; client connects; destruction ends the client connection. | Server lifecycle and selected-port behavior. |
| `TCP_002` | Numeric `set` emits `set`, ID and numeric payload but leaves mapper data unchanged; numeric `get` reply is exactly 13 native bytes. | Numeric parse, emission and reply serialization. |
| `TCP_003` | String/list `set` emits truncated final-byte data; one-byte and unmatched GUI selections remain unchanged; string `get` has type-1/8-byte storage. | Legacy string and selection behavior. |
| `TCP_004` | Pair vectors reply as time values followed by data values; unknown IDs return matching keys or empty five-byte reply. | Data-shape and lookup-reply behavior. |
| `TCP_005` | A four-byte header fragment emits nothing; later completion emits once; coalesced frames retain order. | Buffering and complete-frame ordering. |
| `TCP_006` | Null map has empty `get` reply; unknown command has neither signal nor tested reply. | Null-map and unknown-command behavior. |
| `TCP_007` | Mid-frame disconnect followed by repeated and overlapping clients leaves the later complete request usable in the tested scenario. | Connection replacement/reuse baseline. |
| `TCP_008` | Safe length boundaries, bounded long ID, NUL payload behavior, coalesced `get` order and all current response categories are byte-exact. | Protocol decode/encode extraction. |

The file-level 3F evidence is 87.41% lines, 87.30% executed branches, 52.38%
branches taken at least once and 76.30% calls for
`RemoteControlServer.cpp`.  It is local evidence for this source file, not
project coverage.

## Proposed internal `RemoteControlProtocol` boundary

The first extraction target is a private, QObject-free value class, for
example `RemoteControl/RemoteControlProtocol.h/.cpp`.  It must have no
`QTcpSocket`, `QTcpServer`, `QWidget`, `MessengerClass`, manager, mapper
ownership, plugin, or GUI dependency.  Its narrow responsibilities are:

* retain incomplete raw bytes and split them into complete **safe** frames;
* decode the existing request header into a typed request value without changing
  its native-format interpretation;
* encode the existing `get` reply byte arrays from already supplied typed
  values; and
* classify only the existing three-byte command bytes (`set`, `get`, or
  unknown) without emitting a signal or changing data.

The protocol helper must return values such as `NeedMoreBytes`, `CompleteFrame`
and `UnknownCommand`; it must not prescribe new malformed-frame recovery.  In
particular, retaining a remainder and preserving parse order is more important
than making its API broad.  Mapper lookup and `InterfaceData` conversion can
remain in the server in the first slice, which keeps the boundary value-only
and rollbackable.

`RemoteControlServer` must remain the QObject/Qt transport adapter: port
selection/listening, accepting clients, socket signal connections, the current
socket association, asynchronous writes, connection/error/disconnect lifetime,
and emission of the existing `MessageSender` signal.  `MainWindow` keeps the
server-to-Messenger connection; `MessengerClass` remains the sole manager/UI
signal dispatcher.

## Small, rollbackable slices

1. **4H.1 — frame buffering helper.** Add a private value-only splitter for
   complete versus incomplete safe frame bytes.  Leave all request decoding,
   mapper lookup, signal emission and reply construction in the server.
   Protect with `TCP_005`, plus direct helper vectors added beforehand.  Roll
   back by restoring `DataBuffer`/`NextDataBuffer` handling only.
2. **4H.2 — request decode/response encode helper.** Characterize remaining
   valid header-length and reply-type vectors first, then move exact byte
   decoding and `get` response serialization to the helper.  Server still
   supplies mapper-derived values and performs dispatch/writes.  Rollback is
   limited to the protocol helper call sites.
3. **4H.3 — explicit per-connection state.** After additional multi-client and
   disconnect-lifetime tests, introduce a private state object associated with
   each socket.  Do not change listener binding or public server API.  This
   slice is deferred because the current single raw socket semantics are only
   partially characterized.
4. **4H.4 — error/lifetime hardening, separately approved.** Characterize and
   then replace the obsolete Qt-5 error connection and decide socket cleanup.
   This is not behavior-neutral and must not be bundled with 4H.1/4H.2.

Each implementation slice retains the existing `RemoteControlServer` header,
signal signature, MainWindow connection, and byte vectors; it runs the
existing TCP suite before and after its own focused helper tests.  No plugin,
Messenger, UI, or DataManagement ownership extraction belongs in these slices.

## Concrete test gaps before later slices

* Safe header fragments other than exactly four bytes, exact length-boundary
  fragments, and complete frames that arrive with a remainder are only partly
  sampled.
* The current test covers two coalesced `set`s but not coalesced `get` replies,
  interleaved partial frames from distinct live clients, or response routing
  after a second connection replaces the raw current-socket pointer.
* No contract establishes accepted-socket deletion timing, server close/listen
  failure, port exhaustion, actual socket-error signal delivery, backpressure,
  write failure, or timeout policy.
* Unicode/non-Latin-1 behavior, cross-endian interoperability, numeric
  alignment assumptions, enormous payloads, all declared-length mismatch
  paths, null mapper values, and malformed/zero frames are intentionally
  excluded as unsafe or platform-dependent.
* `set` is verified at the server/Messenger boundary; the wider manager/UI
  propagation is covered by DataManagement contracts rather than duplicated
  here.

## Known defects and risk boundaries

* **Lost final string/StringList byte:** `set` calculates a payload substring
  with `Size - 1`.  `TCP_003` records the resulting truncation.  It is a defect
  candidate, not an intended protocol normalization target for 4H.1.
* **Qt 6 error signal connection:** the source uses the Qt-5-style
  `QTcpSocket::error(QAbstractSocket::SocketError)` signal signature.  Qt 6
  reports this as an invalid connection warning.  `displayError` is a no-op,
  and the phase-3F suite did not prove any actual socket-error handling
  behavior.  The warning and the unproven behavior must remain distinct.
* **Other recorded legacy behavior:** `set` emits but does not directly modify
  the container, and a one-byte GUI-selection payload remains unchanged.  Both
  are established observations, not reasons to change the protocol helper.

Fragmentation and multi-message behavior are particularly sensitive to the
single shared buffer/current-socket design.  Encoding and native-memory layout
are external compatibility risks.  Socket lifetime is a separate ownership
risk because a later accepted socket overwrites the raw pointer while replies
always target that pointer.

## Measurable acceptance and rollback criteria

Before any production extraction, add direct pure-helper tests for every new
safe frame vector and keep `TCP_001`–`TCP_007` unchanged.  A slice is accepted
only if identical test vectors preserve selected-port behavior, byte-exact
replies, emitted command/ID/payload order, mapper non-mutation on `set`, and
the documented no-reply behavior for unknown commands.  No test must codify an
unsafe dereference as a desired result.  If a difference appears, remove the
new helper/delegation and restore the pre-slice server path; no fixture, public
API, message contract, or protocol format migration is involved in rollback.

## Recommended first implementation slice

Implement **4H.1: a private QObject-free safe-frame buffering splitter only**.
First add direct, bounded vectors for partial header/body, one complete frame,
and one complete-plus-remainder byte sequence; then delegate only the existing
`DataBuffer`/`NextDataBuffer` split decision.  Keep header dereferencing,
command parsing, `InterfaceData` conversion, mapper access, socket writes and
Messenger emission in `RemoteControlServer`.  This is the smallest reversible
boundary with direct `TCP_005` protection and does not decide malformed-frame,
socket-ownership, encoding, or legacy payload-defect semantics.

## 4H.1 implementation record

**Completed 2026-08-10.** `RemoteControlFrameSplitter` is a private value
helper that stores only raw `QByteArray` input and separates a complete native
size-prefixed frame from an incomplete remainder. It has no QObject, socket,
Messenger, mapper, command, ID, type, or payload dependency. The server
continues to decode the returned bytes, access the map, emit `MessageSender`
and write replies. A focused remainder vector supplements `TCP_005` with one
complete frame followed by a partial second frame and confirms that the latter
is returned unchanged only after its remaining bytes arrive.

The focused remote-control target and the incremental Release compile check
passed. The server's public constructor, `GetPort`, `MessageSender` signal and
private Qt slots were not changed. No string/list conversion, socket error
connection, encoding, dispatch or ownership behavior was changed.

`TCP_008` subsequently adds safe byte-level vectors before the planned 4H.2
decode/encode extraction. Its 4096-byte ID is the largest bounded deterministic
vector in this suite, not a claimed wire-format limit. It preserves the
documented trailing-NUL truncation for string/list `set`, container
non-mutation, one-byte selection behavior and the invalid Qt-6 error-signal
connection as unresolved defect candidates.
