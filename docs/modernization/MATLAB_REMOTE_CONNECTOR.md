# MATLAB remote connector checkpoint

## Scope and baseline

The previously untracked `MatlabRemoteConnector` used a Boost.Asio background
thread, raw owning pointers, manual buffers and global maps. Before changing
that implementation, `MatlabRemoteConnectorContractTests` was added at the
exported C ABI and real loopback TCP boundary. The unchanged implementation
built with MSYS2 MINGW64 GCC 15.2.0 and passed the test in 0.21 seconds.

The baseline vectors establish:

- unmangled calls to `Connect`, `Disconnect`, `ReceiveDoubleData`,
  `ReadReceivedDoubleData`, `ReadReceivedStringData`, `SendDoubleData` and
  `SendStringData`;
- byte-exact native request framing for numeric/text `set` and `get`;
- synchronous numeric and padded-string response decoding;
- the legacy zero return from both `Send*` functions.

## Refactoring

The implementation now uses one RAII Winsock client per port, scoped Windows
locks, `std::unique_ptr` ownership and `std::vector` receive/frame storage.
Boost, the asynchronous worker and manual `new[]`/`delete` pairs are removed.
The public C header's original exports, localhost endpoint, approximately
two-second connect deadline, synchronous reads, native byte order and valid
wire bytes are unchanged.

The additive `IsConnected` export lets the MATLAB wildcard orchestration
distinguish an empty result from a transport that was discarded after timeout,
partial reply or peer close. The connector now therefore has eight exported
names; the request and response wire format itself is unchanged.

Unsafe null inputs and repeated disconnects now return safely. Responses larger
than the LabAnalyser server's existing 1 MiB encoded reply limit are rejected
before allocation. These paths were not safe baseline contracts.

The final focused run passed both the loopback DLL contract and the MATLAB
package-source contract (2/2 CTests). `objdump` confirmed all
eight exported names. Static MinGW runtime linkage leaves only `KERNEL32.dll`,
`msvcrt.dll` and `WS2_32.dll` as imports.

## Build, install and CI

`MatlabRemoteConnector/CMakeLists.txt` is both standalone and included by the
root Windows CMake build. It builds `TCPClient.dll`, registers the focused CTest
and installs the exact MATLAB package file set. `build-msys2.ps1` also builds
and runs the connector test as part of the established qmake application flow.

For `-Deploy`, the install prefix is `<DeployDir>/LabAnalyser`, producing:

```text
LabAnalyser/
`-- +LabAnalyser/
    |-- Connect.m
    |-- Disconnect.m
    |-- ExReceive.m
    |-- ExSendDouble.m
    |-- ExSendString.m
    |-- Get.m
    |-- Set.m
    |-- TCPClient.dll
    |-- TCPClient.h
    `-- TestRemote.m
```

The Windows GitHub workflow checks this exact file set before uploading the
existing `LabAnalyser-windows-release` artifact. Generated binaries remain
ignored and are never committed.

The MATLAB wrappers resolve `TCPClient.dll` and `TCPClient.h` from
`mfilename('fullpath')`, so loading does not depend on MATLAB's current working
directory. The package-source CTest protects this lookup and the eight header
declarations.

## Limits

No MATLAB installation was available, so MATLAB `loadlibrary` execution and an
actual MATLAB-to-LabAnalyser session remain unverified. The loopback contract
does exercise the same exported DLL calls and wire format. LabAnalyser itself
still handles only its most recently accepted client as current.

## Wildcard get

Before the approved extension, `get` treated `*` as an ordinary character and
returned an empty padded-string response for `*` and `*Buffer*`; this is
recorded by the pre-change `TCP_032` baseline vector. The server now interprets
only requests containing `*` as case-sensitive, whole-ID glob patterns. `*`
matches zero or more characters. It returns matching DataManagement IDs in the
existing sorted map order using the existing `|`-separated text response.
Requests without `*`, including the legacy unknown-ID substring search, retain
their prior behavior.

`LabAnalyser.Get(pattern)` detects that response, fetches every matched exact
ID and returns a `containers.Map<char, any>`. Each map value is a struct with
`x` and `y`, preserving the established scalar, string and split measurement
decoding of an exact `Get`. A no-match wildcard yields an empty map. Failure in
the list request or any following exact request raises
`LabAnalyser:ConnectionLost`, and the caller must reconnect.

The encoded match list remains subject to the existing 1 MiB server reply
limit. Because `|` is the established list delimiter, a DataManagement ID that
itself contains `|` cannot be represented unambiguously by this extension.

## I/O timeout hardening

The pre-change fault-injection vector proved that a connected but silent peer
kept `ReceiveDoubleData()` blocked until the peer closed after 2.6 seconds; the
client had no response deadline. The approved hardening applies one absolute
two-second deadline to each complete send or receive operation on a nonblocking
Winsock socket. Timeout, peer close, send failure and partial replies discard
the port state and close the socket, preventing a late reply from becoming the
next request's response.

The focused contract now covers a silent peer, a reply that stops after its
header and first numeric element, immediate peer close, prompt calls after a
discarded connection, and all prior successful byte vectors. MATLAB observes
the existing zero/empty result and must reconnect before the next operation;
no C export or wire byte changed.
