# Remote-control contracts — phase 3F

`RemoteControlContractTests` uses only loopback, bounded Qt event loops and
temporary clients. Construction listens on the first free port at or above
4080. Every test disconnects clients and destroys its server.

The observed native little-endian frame is `uint32 totalSize`, three Latin-1
command bytes, `uint32 idLength`, `uint32 payloadLength`, NUL-terminated ID and
payload. `get` replies are `[type][elements:uint32]` followed by native doubles
or `elements*8` string bytes; no line ending is used.

| IDs | Evidence |
| --- | --- |
| TCP_001 | lifecycle, port, loopback and shutdown |
| TCP_002 | numeric `set` signal and numeric `get` bytes |
| TCP_003 | string/list/selection set and string get |
| TCP_004 | vector and unknown-ID responses |
| TCP_005 | fragmentation, coalesced frames, signal order |
| TCP_006 | null map, unknown command, bounded no reply |
| TCP_007 | mid-frame disconnect, repeated/multiple clients |
| TCP_008 | safe request-size/length boundaries, NUL payload behavior, coalesced `get` reply order and all current reply categories |

`TCP_008` uses only complete, internally consistent native frames. It records a
minimal empty-ID `get` (`totalSize=16`, `idLength=1`, `payloadLength=0`) and a
minimal numeric `set` (`24`, `1`, `8`), plus a 4096-byte Latin-1 ID as the
largest bounded deterministic test vector; this is not a protocol maximum.
It checks byte-exact replies for every current numeric `InterfaceData`
alternative (signed/unsigned 8/16/32/64-bit values, float, double and bool),
QString, QStringList-first-element, GUI-selection value and paired double
vectors. String replies retain the observed type-1, element-count and
eight-byte-per-element padding. An embedded NUL in a string `set` payload is
preserved in the emitted value while the trailing NUL is excluded by the
existing `Size - 1` behavior. Coalesced `get` requests return their byte-exact
replies in request order; an unknown three-byte command with consistent fields
still has neither a signal nor a reply in the bounded interval.

The initial clean runner was stopped only by the hard 300-second tool limit
after cleaning its tree. The immediately following full runner completed that
already-cleaned tree successfully; this is not claimed as one successful
combined `-Clean` invocation. Fresh Release/Debug builds and the instrumented
suite passed.

| File | Lines | Branches executed | Taken once | Calls |
| --- | ---: | ---: | ---: | ---: |
| `RemoteControl/RemoteControlServer.cpp` | 87.41% (118/135) | 87.30% (220/252) | 52.38% (132/252) | 76.30% (103/135) |

Defect candidates: `set` emits but does not update the container; string and
string-list `set` lose the final byte; one-byte GUI selection remains unchanged;
the Qt-5 `QTcpSocket::error(...)` connection is invalid under Qt 6 and warns.
The tested error-slot body is a no-op; no broader error handling was proven.
Unsafe short/zero/oversized frames, null mapper pointers, port exhaustion and
real OS/network faults were not made contracts.
