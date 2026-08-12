#include "RemoteControlProtocol.h"

#include <cstring>
#include <limits>

namespace {
bool readUInt32Safely(const QByteArray& bytes, int offset, uint32_t* value)
{
    if (offset < 0 || offset > bytes.size() - int(sizeof(uint32_t)))
        return false;
    std::memcpy(value, bytes.constData() + offset, sizeof(*value));
    return true;
}

uint32_t readUInt32(const QByteArray& bytes, int offset)
{
    uint32_t value = 0;
    std::memcpy(&value, bytes.constData() + offset, sizeof(value));
    return value;
}

QByteArray encodeUInt32(uint32_t value)
{
    return QByteArray(reinterpret_cast<const char*>(&value), sizeof(value));
}

QByteArray encodeDouble(double value)
{
    return QByteArray(reinterpret_cast<const char*>(&value), sizeof(value));
}

constexpr quint64 kReplyHeaderSize = 1 + sizeof(uint32_t);
constexpr quint64 kDoubleSize = sizeof(double);
}

RemoteControlProtocol::DecodedFrame RemoteControlProtocol::DecodeValidatedFrame(const QByteArray& frame)
{
    DecodedFrame result;
    if (frame.size() < 16)
        return result;

    if (!readUInt32Safely(frame, 0, &result.TotalSize)
            || !readUInt32Safely(frame, 7, &result.IdLength)
            || !readUInt32Safely(frame, 11, &result.PayloadLength))
        return result;

    if (result.TotalSize != uint32_t(frame.size()) || result.IdLength < 1)
        return result;

    const quint64 expectedSize = quint64(15) + result.IdLength + result.PayloadLength;
    if (expectedSize != quint64(frame.size()))
        return result;

    const int idEnd = 15 + int(result.IdLength) - 1;
    if (frame.at(idEnd) != '\0')
        return result;

    const QByteArray command = frame.mid(4, 3);
    if (command == "set")
        result.CommandType = Command::Set;
    else if (command == "get")
        result.CommandType = Command::Get;

    result.Id = QString::fromLatin1(frame.mid(15, int(result.IdLength) - 1));
    result.Payload = frame.mid(15 + int(result.IdLength), int(result.PayloadLength));
    result.Status = DecodeStatus::Valid;
    return result;
}

bool RemoteControlProtocol::HasNumericSetPayload(const DecodedFrame& frame)
{
    return frame.Status == DecodeStatus::Valid
            && frame.CommandType == Command::Set
            && frame.Payload.size() >= int(sizeof(double));
}

QByteArray RemoteControlProtocol::RemoveOptionalTrailingNul(const QByteArray& payload)
{
    if (!payload.isEmpty() && payload.endsWith('\0'))
        return payload.left(payload.size() - 1);
    return payload;
}

RemoteControlProtocol::DecodedFrame RemoteControlProtocol::DecodeCompleteFrame(const QByteArray& frame)
{
    DecodedFrame result;
    result.TotalSize = readUInt32(frame, 0);
    const QByteArray command = frame.mid(4, 3);
    if (command == "set")
        result.CommandType = Command::Set;
    else if (command == "get")
        result.CommandType = Command::Get;
    else
        result.CommandType = Command::Unknown;

    result.IdLength = readUInt32(frame, 7);
    result.PayloadLength = readUInt32(frame, 11);
    result.Id = QString::fromLatin1(frame.mid(15, result.IdLength - 1));
    result.Payload = frame.mid(15 + result.IdLength, result.PayloadLength);
    result.Status = DecodeStatus::Valid;
    return result;
}

QByteArray RemoteControlProtocol::EncodeEmptyReply()
{
    return QByteArray(1, '\0') + encodeUInt32(0);
}

bool RemoteControlProtocol::CanEncodePaddedReplyElements(quint64 elements)
{
    return elements <= uint32_t(-1)
            && elements <= (MaxEncodedReplySize - kReplyHeaderSize) / kDoubleSize;
}

bool RemoteControlProtocol::CanEncodeVectorReplyElements(quint64 timeElements, quint64 dataElements)
{
    if (timeElements > std::numeric_limits<quint64>::max() - dataElements)
        return false;
    return CanEncodePaddedReplyElements(timeElements + dataElements);
}

bool RemoteControlProtocol::TryEncodeStringReply(const QString& value, QByteArray* reply)
{
    if (!reply)
        return false;

    const std::string text = value.toStdString();
    const quint64 textBytes = std::strlen(text.c_str());
    if (textBytes == std::numeric_limits<quint64>::max()
            || !CanEncodePaddedReplyElements(textBytes + 1))
        return false;

    const quint64 elements = textBytes + 1;
    const quint64 payloadBytes = elements * kDoubleSize;
    QByteArray encoded(1, '\1');
    encoded.append(encodeUInt32(uint32_t(elements)));
    encoded.append(text.data(), int(textBytes));
    encoded.append(int(payloadBytes - textBytes), '\0');
    *reply = encoded;
    return true;
}

bool RemoteControlProtocol::TryEncodeVectorReply(const std::vector<double>& time,
                                                  const std::vector<double>& data,
                                                  QByteArray* reply)
{
    if (!reply || !CanEncodeVectorReplyElements(time.size(), data.size()))
        return false;

    const quint64 elements = time.size() + data.size();
    QByteArray encoded(1, '\0');
    encoded.append(encodeUInt32(uint32_t(elements)));
    if (!time.empty())
        encoded.append(reinterpret_cast<const char*>(time.data()), int(kDoubleSize * time.size()));
    if (!data.empty())
        encoded.append(reinterpret_cast<const char*>(data.data()), int(kDoubleSize * data.size()));
    *reply = encoded;
    return true;
}

QByteArray RemoteControlProtocol::EncodeNumericReply(double value)
{
    return QByteArray(1, '\0') + encodeUInt32(1) + encodeDouble(value);
}

QByteArray RemoteControlProtocol::EncodeStringReply(const QString& value)
{
    QByteArray reply;
    TryEncodeStringReply(value, &reply);
    return reply;
}

QByteArray RemoteControlProtocol::EncodeVectorReply(const std::vector<double>& time, const std::vector<double>& data)
{
    QByteArray reply;
    TryEncodeVectorReply(time, data, &reply);
    return reply;
}
