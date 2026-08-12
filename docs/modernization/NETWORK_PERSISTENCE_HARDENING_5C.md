# Network and persistence hardening — package C

## Completed focused slices

`c69ffd3` adds a 1 MiB inclusive encoded-reply ceiling at the private,
QObject-free `RemoteControlProtocol` boundary. Size arithmetic is checked in
`quint64` before an allocation, an `int` conversion or a wire `uint32_t`
conversion. String and vector replies that fit retain their native-byte-order
bytes exactly. The largest padded reply that can be represented by the legacy
eight-byte payload layout is 1,048,573 bytes (`5 + 131,071 * 8`); the 3-byte
remainder cannot encode a further element. An unsafe or over-limit reply makes
the server send no partial or empty success reply and `abort()` the current
socket; a later connection remains usable. `TCP_030` exercises the direct
encoder boundary and `TCP_031` exercises abort/recovery. No `get` path emits a
Messenger message, as before.

The package's XML slice adds the private, QObject-free
`XmlFigureDimensions::ParseAndValidate` before
`XmlExperimentReader::CreateFigureWindow()` creates GUI state. `Rows` and
`Cols` are each restricted to integral `0..32`; their product is restricted to
256. Invalid values raise deterministic reader errors and preserve the legacy
`true == error` convention without creating a figure, plot or registry entry.
Zero-row grids remain permitted for legacy compatibility. `XML_FIG_008` covers
direct boundaries and negative, nonnumeric, out-of-range and product-overflow
reader inputs; it also records accepted 32x8 and 0x32 dimensions without
constructing hundreds of plot widgets.

## Evidence and compatibility

`RemoteControlContractTests` passed after `c69ffd3` (exit 0). The resumed
`XmlExperimentContractTests` build and run passed with 23 Qt Test entries,
zero failures and exit 0; its runtime-built compatible plugin root was set for
`XML_LEGACY_005`. The test's expected offscreen Qt warnings are unchanged.
Legacy fixture hashes were neither edited nor normalized.

The package deliberately adds no arbitrary whole-file, XML-version, schema,
authentication, TLS, rate-limit or byte-order rule. Source review found no
other narrow parameter/XML integer or allocation path that could be bounded
without choosing a new file-format policy.

## Remaining risks

- Accepted figure geometry and later child processing can still leave the
  documented partial state; no rollback is introduced.
- Remote responses are bounded, but input frame limits, payload semantics and
  application-level data production remain separate contracts.
- Reply policy for unknown or future interface payload types and any
  application-level pagination remain unversioned protocol decisions.

GitHub CI is the package-level full checkpoint; no local clean, Debug,
coverage or static-analysis run was repeated for these focused slices.
