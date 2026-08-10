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

    struct DecodedFrame {
        uint32_t TotalSize;
        uint32_t IdLength;
        uint32_t PayloadLength;
        Command CommandType;
        QString Id;
        QByteArray Payload;
    };

    static DecodedFrame DecodeCompleteFrame(const QByteArray& frame);
    static QByteArray EncodeEmptyReply();
    static QByteArray EncodeNumericReply(double value);
    static QByteArray EncodeStringReply(const QString& value);
    static QByteArray EncodeVectorReply(const std::vector<double>& time, const std::vector<double>& data);
};

#endif // REMOTECONTROLPROTOCOL_H
