# Remote-control hardening 5A

## 5A.1 characterization scope

5A.1 left production code unchanged. 5A.2a is an explicitly approved,
minimal Qt-6 error-connection correction. `RemoteControlContractTests` records
the current accepted-server-socket behavior using loopback-only, bounded
event-loop waits. The test-only `private`-visibility seam is compiled
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
message, recovery, socket close, deletion, reparenting or lifetime policy.

## Observed accepted-socket ownership and lifetime

`RemoteControlServer` contains a value `QTcpServer`. On acceptance it obtains
the pending `QTcpSocket`; Qt parents that socket to this `QTcpServer`.
`TCP_014` shows that after client disconnect both endpoints become
`UnconnectedState`, while the accepted server socket remains a live child. A
fresh client is accepted and adds a second child.

`TCP_015` repeats three connect/disconnect cycles. Each accepted socket remains
an `UnconnectedState` child until the server is destroyed; the observed child
count grows from one to three. `QPointer` observers become null when destruction
of `RemoteControlServer` destroys its value `QTcpServer` and these children.
No accepted socket is manually deleted, reparented or dereferenced after its
`QPointer` becomes null.

## Test mapping and evidence

| Test ID | Observed result |
| --- | --- |
| `TCP_013` | Qt-6 metaobject lacks historical `error(...)`, exposes `errorOccurred(...)`, and the typed accept path has no legacy warning. |
| `TCP_014` | Accepted socket is a `QTcpServer` child, survives client disconnect, and a fresh connection is accepted. |
| `TCP_015` | Repeated disconnected accepted sockets remain until server destruction; all observed QPointers null on destruction. |
| `TCP_016` | Not implemented: a real socket-error delivery test would depend on OS/network timing and cannot prove the unconnected legacy slot safely. |

The 5A.1 focused suite passed with exit code 0: 16 test functions plus Qt Test
initialization/cleanup, reported as 18 passed, 0 failed, 0 skipped in 2268 ms.
5A.2a reruns only this focused suite and its incremental Release compile check.

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

### B. Accepted-socket lifetime (not implemented)

Choose whether disconnected server-side sockets remain children until server
destruction, as observed, or are released through controlled `deleteLater()`.
The decision must characterize effects on `QTcpServer` children, QPointer
observers, repeated connections and any current-socket reference. It must not
be bundled with independent multi-client behavior.
