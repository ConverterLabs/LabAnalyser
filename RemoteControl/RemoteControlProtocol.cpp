#include "RemoteControlProtocol.h"

#include <cstring>

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

QByteArray RemoteControlProtocol::EncodeNumericReply(double value)
{
    return QByteArray(1, '\0') + encodeUInt32(1) + encodeDouble(value);
}

QByteArray RemoteControlProtocol::EncodeStringReply(const QString& value)
{
    const std::string text = value.toStdString();
    const uint32_t elements = uint32_t(std::strlen(text.c_str()) + 1);
    QByteArray dataOut(1, '\1');
    dataOut.append(encodeUInt32(elements));
    std::vector<char> padded(elements * 8, 0);
    for (uint32_t i = 0; i < elements; i++)
        padded[i] = text[i];
    dataOut.append(padded.data(), int(padded.size()));
    return dataOut;
}

QByteArray RemoteControlProtocol::EncodeVectorReply(const std::vector<double>& time, const std::vector<double>& data)
{
    QByteArray dataOut(1, '\0');
    dataOut.append(encodeUInt32(uint32_t(time.size() + data.size())));
    if (!time.empty())
        dataOut.append(reinterpret_cast<const char*>(time.data()), int(sizeof(double) * time.size()));
    if (!data.empty())
        dataOut.append(reinterpret_cast<const char*>(data.data()), int(sizeof(double) * data.size()));
    return dataOut;
}
