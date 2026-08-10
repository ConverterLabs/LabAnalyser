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
    void Append(const QByteArray& bytes);
    bool TakeCompleteFrame(QByteArray* frame);
    void Clear();

private:
    QByteArray Buffer;
};

#endif // REMOTECONTROLFRAMESPLITTER_H
