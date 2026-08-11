# Remote-control hardening 5A

## 5A.1 characterization scope

Production code was not changed. `RemoteControlContractTests` records the
current Qt-6 signal and accepted-server-socket behavior using loopback-only,
bounded event-loop waits. The test-only `private`-visibility seam is compiled
only into this contract target; it observes the private `QTcpServer` member and
its QObject children without changing any production header or ownership.

## Observed Qt-6 signal contract

The locally installed Qt 6.9.2 `QAbstractSocket` header exposes
`errorOccurred(QAbstractSocket::SocketError)`. Its `QTcpSocket` metaobject has
that signal and does not have the historical
`error(QAbstractSocket::SocketError)` signal. `TCP_013` triggers the existing
server accept path and deterministically captures Qt's runtime warning:

`QObject::connect: No such signal QTcpSocket::error(QAbstractSocket::SocketError)`

The warning demonstrates that the current legacy connection is invalid under
Qt 6. It does not demonstrate any desired error message or recovery behavior:
`displayError` remains a no-op and the obsolete connection never invokes it.

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
| `TCP_013` | Qt-6 metaobject lacks historical `error(...)`, exposes `errorOccurred(...)`, and the existing accept path warns. |
| `TCP_014` | Accepted socket is a `QTcpServer` child, survives client disconnect, and a fresh connection is accepted. |
| `TCP_015` | Repeated disconnected accepted sockets remain until server destruction; all observed QPointers null on destruction. |
| `TCP_016` | Not implemented: a real socket-error delivery test would depend on OS/network timing and cannot prove the unconnected legacy slot safely. |

The focused suite passed with exit code 0: 16 test functions plus Qt Test
initialization/cleanup, reported as 18 passed, 0 failed, 0 skipped in 2268 ms.
This is the only technical verification performed for 5A.1.

## Exclusions and decision preparation

No test deliberately creates a stale server-socket access, port exhaustion,
unbounded wait, use-after-free, malformed frame or independent multi-client
session. Socket error injection remains unverified because it is not
deterministic under the current public contract and production connection.

### A. Qt-6 signal repair (not implemented)

Replace the historical string-based `error(...)` connection with a typed
`errorOccurred(QAbstractSocket::SocketError)` connection. A subsequent approved
slice must first decide the observable error policy: message, signal, logging,
close/retry behavior and any interaction with the current connection state.
This characterization does not prescribe that policy.

### B. Accepted-socket lifetime (not implemented)

Choose whether disconnected server-side sockets remain children until server
destruction, as observed, or are released through controlled `deleteLater()`.
The decision must characterize effects on `QTcpServer` children, QPointer
observers, repeated connections and any current-socket reference. It must not
be bundled with independent multi-client behavior.
