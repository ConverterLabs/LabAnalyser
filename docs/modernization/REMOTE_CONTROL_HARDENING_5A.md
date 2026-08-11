# Remote-control hardening 5A

## 5A.1 characterization and approved 5A.2 corrections

5A.1 left production code unchanged. 5A.2a is an explicitly approved,
minimal Qt-6 error-connection correction; 5A.2b is an explicitly approved,
controlled accepted-socket release correction. `RemoteControlContractTests`
uses loopback-only, bounded event-loop waits. The test-only
`private`-visibility seam is compiled
only into this contract target; it observes the private `QTcpServer` member and
its QObject children without changing any production header or ownership.

## Observed Qt-6 signal contract

The locally installed Qt 6.9.2 `QAbstractSocket` header exposes
`errorOccurred(QAbstractSocket::SocketError)`. Its `QTcpSocket` metaobject has
that signal and does not have the historical
`error(QAbstractSocket::SocketError)` signal. Before the 5A.2a correction,
`TCP_013` triggered the accept path and deterministically captured Qt's runtime
warning:

`QObject::connect: No such signal QTcpSocket::error(QAbstractSocket::SocketError)`

The approved 5A.2a change replaces only the invalid string-based connection
with `&QAbstractSocket::errorOccurred` connected to the existing
`RemoteControlServer::displayError`. The same focused test now confirms that
the historical signal is still absent, `errorOccurred` remains present, and no
such-signal warning occurs during acceptance. Its temporary Qt message handler
records only that known warning and forwards all other Qt messages to the prior
handler.

`displayError` deliberately remains a no-op: this correction adds no visible
message, recovery, socket close, reparenting or error-lifetime policy.

## Observed accepted-socket ownership and lifetime

`RemoteControlServer` contains a value `QTcpServer`. On acceptance it obtains
the pending `QTcpSocket`; Qt parents that socket to this `QTcpServer`. Before
the approved 5A.2b correction, `TCP_014..TCP_015` established that disconnected
sockets remained children until server destruction.

After the approved correction, the typed `disconnected()` connection runs this
order: `RemoteControlConnectionState::ResetIfCurrent(socket)` first clears the
current QPointer and its frame-splitter remainder only when the disconnecting
socket is current; then `socket->deleteLater()` schedules Qt-owned destruction.
The lambda is connected with `RemoteControlServer` as context, so it cannot run
against a destroyed server. No socket is manually deleted, reparented or owned
by a smart pointer.

`TCP_014` proves that a disconnected current socket becomes null through its
QPointer after deferred deletion, the current state and its remainder are
empty, and a fresh connection works. `TCP_015` proves repeated cycles do not
accumulate `QTcpServer` children and that server destruction remains safe while
a disconnect cleanup may be pending. `TCP_016` proves that disconnecting older
A releases only A; current B and B's frame state remain usable for `get`.

## 5A.3a TCP_007 testfixture cleanup stabilization

Before this test-only change, a 25-process TCP_007 diagnosis reproduced the
intermittent Qt warning `QObject: shared QObject was deleted directly` and
Windows access violation `0xC0000005` on the 21st isolated process. TCP_007
had waited only for client-side `UnconnectedState`; it could then destroy its
stack-owned server while accepted server-side sockets still had scheduled
`DeferredDelete` events.

TCP_007 now records each accepted `QTcpSocket` through the existing test-only
private visibility seam. Before its local `RemoteControlServer` ends, it waits
for every observed server-socket `QPointer` to become null and for the
`QTcpServer` child list to contain no `QTcpSocket`. These event-loop conditions
exercise the existing `disconnected()`/`deleteLater()` behavior; they neither
delete a socket directly nor change the production lifetime policy.

After the fixture change, 50 separate TCP_007 processes passed with a
three-second process bound, and the one full focused suite passed 31/31. This
stabilizes fixture teardown only. It does **not** prove that every production
server-shutdown ordering is safe; the previously observed Qt warning remains a
documented production-lifetime risk until a dedicated production shutdown
scenario identifies its object and cause.

## 5A.3c common RemoteControl fixture teardown

The later focused run exposed the same fixture boundary in `TCP_013`: it had
completed its Qt-6 metaobject assertion after only the client reached
`UnconnectedState`. The earlier intermittent observations are therefore
testfixture cleanup findings in both `TCP_007` and `TCP_013`, not evidence that
either their protocol assertions were wrong.

`finalizeRemoteServer()` is test-only. It snapshots currently accepted sockets
as `QPointer`s, requests disconnect only from the supplied loopback clients,
processes `DeferredDelete` through the event loop, and succeeds only after all
observed server sockets are null and `QTcpServer` has no `QTcpSocket` children.
It never deletes a server-side socket. Every TCP test that ends a real
loopback/server scope now calls it after its existing business assertions;
`TCP_001` is the deliberate exception because it characterizes direct server
destruction with a still-connected client, and `TCP_017..TCP_021`/`TCP_029`
use no server sockets.

The separate process-isolated production-shutdown harness compared immediate
server destruction from the disconnect signal (A) with destruction after
deferred deletion (B): 50/50 A and 50/50 B runs exited cleanly. It produced no
production correction. After the common fixture cleanup, 20/20 complete
RemoteControlContractTests processes passed without warnings, crashes or
timeouts. This improves test determinism only; it is not a proof of every
possible production shutdown interleaving.

## Test mapping and evidence

| Test ID | Observed result |
| --- | --- |
| `TCP_013` | Qt-6 metaobject lacks historical `error(...)`, exposes `errorOccurred(...)`, and the typed accept path has no legacy warning. |
| `TCP_014` | Current accepted socket is reset, its partial-frame state is cleared and its QPointer becomes null after `deleteLater()`; a fresh connection works. |
| `TCP_015` | Repeated disconnected accepted sockets are released and do not accumulate as `QTcpServer` children; server destruction remains safe. |
| `TCP_016` | Disconnecting older A releases A only; current B stays current and returns the existing byte-exact `get` reply. |
| `TCP_007` | The pre-existing fragmentation/reconnection vectors now also wait for all observed accepted sockets to complete deferred deletion before fixture server destruction. |

The 5A.2b focused suite passed with exit code 0: 17 test functions plus Qt Test
initialization/cleanup, reported as 19 passed, 0 failed, 0 skipped in 2520 ms.
It and the incremental Release compile check are the only technical
verification performed for this correction.

## Exclusions and decision preparation

No test deliberately creates a stale server-socket access, port exhaustion,
unbounded wait, use-after-free, malformed frame or independent multi-client
session. Socket error injection remains unverified because it is not
deterministic under the current public contract and production connection.

### A. Qt-6 signal repair (implemented only as approved connection correction)

The historical string-based `error(...)` connection is now replaced by the
typed `errorOccurred(QAbstractSocket::SocketError)` connection. A later approved
slice must still decide any observable error policy: message, signal, logging,
close/retry behavior and interaction with current connection state. 5A.2a does
not prescribe or implement that policy.

### B. Accepted-socket lifetime (implemented as approved controlled release)

The approved choice is controlled `deleteLater()` release after each server-side
socket's `disconnected()` signal. Current-state reset is conditional on pointer
identity; older-socket disconnects do not disturb the current socket or frame
state. Independent multi-client sessions, error-policy changes and any other
socket ownership changes remain outside this correction.
