#ifndef REMOTECONTROLFRAMESPLITTER_H
#define REMOTECONTROLFRAMESPLITTER_H

#include <QByteArray>
#include <cstdint>

// Private value helper for the existing native-size TCP frame boundary.  It
// deliberately has no knowledge of commands, IDs, payloads, sockets or data
// managers; interpretation remains in RemoteControlServer.
class RemoteControlFrameSplitter
{
public:
    enum class FrameResult { Incomplete, Complete, InvalidPrefix };

    static constexpr uint32_t MinimumFrameSize = 16;
    static constexpr uint32_t MaxFrameSize = 1024 * 1024;

    void Append(const QByteArray& bytes);

    // Safe framing primitive for new callers. InvalidPrefix consumes the
    // buffered prefix by clearing the buffer, so callers cannot spin on the
    // same invalid totalSize. The server intentionally still uses its legacy
    // bool API until the separately approved delegation slice.
    FrameResult TakeFrame(QByteArray* frame);

    // Legacy server API. Its behavior is kept untouched until the server is
    // explicitly migrated to TakeFrame().
    bool TakeCompleteFrame(QByteArray* frame);
    void Clear();

private:
    QByteArray Buffer;
};

#endif // REMOTECONTROLFRAMESPLITTER_H
