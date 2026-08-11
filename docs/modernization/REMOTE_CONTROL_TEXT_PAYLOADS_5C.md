# Remote-control text payloads 5C

## 5C.2a legacy characterization

`payloadLength` is the exact number of bytes after the NUL-terminated ID. The
test frame builder does not append a payload terminator. For the three text
branches, the existing server uses:

```cpp
QString::fromLatin1(decoded.Payload.left(decoded.Payload.size() - 1))
```

Thus it always removes exactly the final byte. Qt 6 clamps `left(-1)` to an
empty array. The result below is the current observable baseline, not an
approved desired contract.

| Payload bytes | Declared `payloadLength` | Current QString / one QStringList element |
| --- | ---: | --- |
| empty | 0 | empty |
| `A` | 1 | empty |
| `AB` | 2 | `A` |
| `AB 00` | 3 | `AB` |
| `A 00 B 00` | 4 | `A`, embedded NUL, `B` |
| `A 00 00` | 3 | `A`, one remaining NUL |
| `E4 00` | 2 | U+00E4 through existing Latin-1 decoding |

`TCP_027` checks every value for both `QString` and `QStringList`: exactly one
`MessageSender("set", id, InterfaceData)` event, actual type, QChar length and
contents, and unchanged backing container. A QStringList payload has no list
separator or multi-element encoding: the server always constructs a fresh
single-element list.

`TCP_028` records the related but separate Selection outcome for choices
`{"a", "b"}`. Nonterminated `b` is shortened to empty, fails membership and
emits the unchanged `a`; `b 00` becomes `b` and is accepted; `x 00` and an
empty payload leave `a`. Every case still emits exactly one `set` signal. The
Selection behavior combines byte loss with an additional domain membership
rule, so it remains a separately approval-required change.

## Get contract and asymmetry

String, StringList and GuiSelection replies are currently encoded as one type
byte `01`, a native `uint32_t` element count, then `elements * 8` zero-padded
bytes. `elements` is `strlen(value.toStdString().c_str()) + 1`; the first
region contains a NUL terminator. Existing `TCP_003` and `TCP_008` assert
ASCII bytes, count and padding, including that QStringList serializes only its
first element.

Set and get are not symmetric: set consumes the frame payload through
`QString::fromLatin1`, while get forms a UTF-8 `std::string` before `strlen`
and padding. Embedded NULs are retained by the set conversion but truncate the
get representation. This phase does not change encoding or claim a UTF-8
protocol contract.

## History and approved QString/QStringList correction

The Git history contains no specification that `payloadLength` always includes
a NUL terminator. The 2021 initial implementation (`5767435`) passed the full
calculated String/Selection payload to `fromLatin1`; the 2025-02-18 change
`915d32d` introduced `Size - 1` for String, StringList and Selection under the
unrelated message “updated DataManagement/DataManagementClass.cpp”. The later
protocol extraction preserved that behavior. This is therefore an
unversioned, inconsistent legacy implementation rather than demonstrated wire
intent.

**5C.2b is an explicitly approved behavior correction.**
`RemoteControlProtocol::RemoveOptionalTrailingNul()` is a pure helper that
returns a new byte array, preserves its input and removes exactly one byte only
when that actual final byte is NUL. `RemoteControlServer` uses it only for the
QString and QStringList `set` branches. Therefore empty payloads remain empty,
nonterminated `A` and `AB` remain complete, embedded NULs remain data, and
`A 00 00` retains the first trailing NUL. Conversion remains
`QString::fromLatin1()` and QStringList remains a newly built list containing
exactly one element. TCP_027 carries the same vectors from the baseline commit
`3244b4f` with their approved new values; TCP_029 directly proves helper
purity and all trailing-NUL cases.

GuiSelection is intentionally excluded: it still calls `left(size - 1)` and
then performs the existing membership check. Thus TCP_028 remains unchanged:
bare `b` does not select `b`, while `b 00` does. Get replies, framing, native
byte order and the existing get/set asymmetry are also unchanged.

## 5C.3 approved GuiSelection correction

**5C.3 is an explicitly approved behavior correction.** The GuiSelection set
branch now uses the same `RemoveOptionalTrailingNul()` helper as QString and
QStringList. It therefore preserves a bare `b`, removes exactly one final NUL
from `b 00`, retains embedded NULs, and continues to decode with Latin-1.
The existing membership condition remains the only state gate: only a text
exactly present in the existing choice list becomes the current selection.
Nonmembers, empty input, `b 00 00`, and embedded-NUL nonmembers retain the
preexisting current value. Every structurally valid set frame still emits one
MessageSender event, even when membership rejects it; the backing container is
still not directly mutated. TCP_028 covers bare/trailing/double/embedded NUL
and an U+00E4 Latin-1 choice with the shared testfixture cleanup.

StringList stays one element, and get/set asymmetry remains intentionally open.

## Evidence

The baseline characterization in `3244b4f` completed with exit code 0. The
5C.2b focused verification records the new approved QString/QStringList
semantics separately; no compatibility claim is made for GuiSelection or get
symmetry.
