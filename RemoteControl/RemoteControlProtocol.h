#ifndef REMOTECONTROLPROTOCOL_H
#define REMOTECONTROLPROTOCOL_H

#include <QByteArray>
#include <QString>
#include <cstdint>
#include <vector>

// Private value codec for complete, already safe remote-control frames. It
// deliberately has no QObject, socket, Messenger, mapper or GUI dependency.
class RemoteControlProtocol
{
public:
    enum class Command { Set, Get, Unknown };
    enum class DecodeStatus { Valid, Invalid };

    struct DecodedFrame {
        uint32_t TotalSize = 0;
        uint32_t IdLength = 0;
        uint32_t PayloadLength = 0;
        Command CommandType = Command::Unknown;
        DecodeStatus Status = DecodeStatus::Invalid;
        QString Id;
        QByteArray Payload;
    };

    // Safe structural decoder for already complete frames. Invalid identifies
    // malformed framing separately from a structurally valid unknown command.
    static DecodedFrame DecodeValidatedFrame(const QByteArray& frame);
    static bool HasNumericSetPayload(const DecodedFrame& frame);

    // Legacy server decoder. Retained unchanged until the server delegation
    // slice adopts DecodeValidatedFrame().
    static DecodedFrame DecodeCompleteFrame(const QByteArray& frame);
    static QByteArray EncodeEmptyReply();
    static QByteArray EncodeNumericReply(double value);
    static QByteArray EncodeStringReply(const QString& value);
    static QByteArray EncodeVectorReply(const std::vector<double>& time, const std::vector<double>& data);
};

#endif // REMOTECONTROLPROTOCOL_H
