#include "RemoteControlFrameSplitter.h"

#include <cstring>

void RemoteControlFrameSplitter::Append(const QByteArray& bytes)
{
    Buffer.append(bytes);
}

RemoteControlFrameSplitter::FrameResult RemoteControlFrameSplitter::TakeFrame(QByteArray* frame)
{
    if (Buffer.size() < int(sizeof(uint32_t)))
        return FrameResult::Incomplete;

    uint32_t frameSize = 0;
    std::memcpy(&frameSize, Buffer.constData(), sizeof(frameSize));
    if (frameSize < MinimumFrameSize || frameSize > MaxFrameSize)
    {
        Buffer.clear();
        return FrameResult::InvalidPrefix;
    }

    if (static_cast<quint64>(Buffer.size()) < frameSize)
        return FrameResult::Incomplete;

    *frame = Buffer.left(static_cast<qsizetype>(frameSize));
    Buffer.remove(0, static_cast<qsizetype>(frameSize));
    return FrameResult::Complete;
}

bool RemoteControlFrameSplitter::TakeCompleteFrame(QByteArray* frame)
{
    if (Buffer.size() < int(sizeof(uint32_t)))
        return false;

    uint32_t frameSize = 0;
    std::memcpy(&frameSize, Buffer.constData(), sizeof(frameSize));
    if (static_cast<quint64>(Buffer.size()) < frameSize)
        return false;

    *frame = Buffer.left(static_cast<qsizetype>(frameSize));
    Buffer.remove(0, static_cast<qsizetype>(frameSize));
    return true;
}

void RemoteControlFrameSplitter::Clear()
{
    Buffer.clear();
}
