#include <QtTest>
#include <QTcpSocket>
#include <QTcpServer>
#include <QPointer>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <cstring>
#include <cstdint>

// Test-only seam: exposes the private QTcpServer member solely to observe its
// QObject-owned accepted sockets. Production headers and targets are unchanged.
#define private public
#include "RemoteControl/RemoteControlServer.h"
#undef private
#include "RemoteControl/RemoteControlFrameSplitter.h"
#include "RemoteControl/RemoteControlProtocol.h"

Q_DECLARE_METATYPE(InterfaceData)

namespace {
QByteArray u32(uint32_t value) { return QByteArray(reinterpret_cast<const char*>(&value), sizeof(value)); }
QByteArray f64(double value) { return QByteArray(reinterpret_cast<const char*>(&value), sizeof(value)); }
uint32_t readU32(const QByteArray& bytes, int offset) { uint32_t value = 0; std::memcpy(&value, bytes.constData() + offset, sizeof(value)); return value; }
double readF64(const QByteArray& bytes, int offset) { double value = 0; std::memcpy(&value, bytes.constData() + offset, sizeof(value)); return value; }

QByteArray frame(const QByteArray& command, const QByteArray& id, const QByteArray& payload = QByteArray()) {
    const QByteArray nulId = id + '\0';
    const uint32_t size = 15 + uint32_t(nulId.size()) + uint32_t(payload.size());
    return u32(size) + command + u32(uint32_t(nulId.size())) + u32(uint32_t(payload.size())) + nulId + payload;
}

QTcpSocket* connectClient(RemoteControlServer& server, QObject* parent) {
    auto* client = new QTcpSocket(parent);
    client->connectToHost(QHostAddress::LocalHost, quint16(server.GetPort()));
    if (!client->waitForConnected(1000)) qFatal("Loopback connection did not complete");
    return client;
}

QByteArray readReply(QTcpSocket* client, int expected) {
    QByteArray result;
    QElapsedTimer timer; timer.start();
    while (result.size() < expected && timer.elapsed() < 1000) {
        if (client->bytesAvailable() == 0) {
            QEventLoop loop;
            QTimer timeout; timeout.setSingleShot(true);
            QObject::connect(client, &QTcpSocket::readyRead, &loop, &QEventLoop::quit);
            QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
            timeout.start(1000 - int(timer.elapsed()));
            loop.exec();
            if (client->bytesAvailable() == 0) break;
        }
        result += client->readAll();
    }
    if (result.size() < expected) qFatal("Expected loopback reply did not arrive completely");
    return result;
}

ToFormMapper* numeric(double value) { auto* result = new ToFormMapper("double", "Parameter"); result->SetData(value); return result; }
template <typename T>
ToFormMapper* scalar(T value) { auto* result = new ToFormMapper("scalar", "Parameter"); result->SetData(value); return result; }
ToFormMapper* text(const QString& value, const QString& type = "string") { auto* result = new ToFormMapper(type, "Parameter"); result->SetData(value); return result; }
ToFormMapper* list(const QStringList& value) { auto* result = new ToFormMapper("QStringList", "Parameter"); result->SetData(value); return result; }
ToFormMapper* selection(const QString& current, const QStringList& choices) { auto* result = new ToFormMapper("GuiSelection", "Parameter"); result->SetData(GuiSelection(current, choices)); return result; }
ToFormMapper* vectors(const std::vector<double>& time, const std::vector<double>& data) {
    auto* result = new ToFormMapper("DataPair", "Data");
    result->SetData(DataPair(boost::shared_ptr<std::vector<double>>(new std::vector<double>(time)), boost::shared_ptr<std::vector<double>>(new std::vector<double>(data))));
    return result;
}
void clear(std::map<QString, ToFormMapper*>& data) { for (auto pair : data) delete pair.second; data.clear(); }

QByteArray numericReply(double value) { return QByteArray(1, '\0') + u32(1) + f64(value); }
QByteArray paddedStringReply(const QString& value) {
    const QByteArray bytes = value.toStdString().c_str();
    const uint32_t elements = uint32_t(bytes.size() + 1);
    QByteArray result(1, '\1');
    result += u32(elements);
    result += bytes;
    result += QByteArray(int(elements * 8 - bytes.size()), '\0');
    return result;
}
QByteArray vectorReply(const std::vector<double>& time, const std::vector<double>& data) {
    QByteArray result(1, '\0');
    result += u32(uint32_t(time.size() + data.size()));
    for (double value : time) result += f64(value);
    for (double value : data) result += f64(value);
    return result;
}

QStringList capturedTargetWarnings;
QtMessageHandler previousQtMessageHandler = nullptr;

class QtMessageCapture {
public:
    QtMessageCapture() { capturedTargetWarnings.clear(); previousQtMessageHandler = qInstallMessageHandler(handler); }
    ~QtMessageCapture() { qInstallMessageHandler(previousQtMessageHandler); }

    static bool contains(const QString& part) {
        for (const QString& message : capturedTargetWarnings) if (message.contains(part)) return true;
        return false;
    }

private:
    static void handler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
        if (message.contains("No such signal QTcpSocket::error(QAbstractSocket::SocketError)"))
            capturedTargetWarnings.append(message);
        if (previousQtMessageHandler)
            previousQtMessageHandler(type, context, message);
    }
};

QList<QTcpSocket*> acceptedSockets(RemoteControlServer& server) {
    return server.tcpServer.findChildren<QTcpSocket*>();
}
}

class RemoteControlContractTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void TCP_001_lifecyclePortAndLoopback();
    void TCP_002_numericSetSignalAndGetBytes();
    void TCP_003_stringListAndSelectionContracts();
    void TCP_004_vectorAndUnknownIdResponses();
    void TCP_005_fragmentationAndCoalescedFrames();
    void TCP_006_unknownCommandNullDataAndNoTimeoutReply();
    void TCP_007_disconnectRepeatedAndMultipleConnections();
    void TCP_008_protocolByteBoundariesAndReplies();
    void TCP_009_twoConnectedClientsUseLatestAcceptedSocket();
    void TCP_010_fragmentStateIsDiscardedOnSecondConnection();
    void TCP_011_firstClientRequestsAfterSecondAcceptance();
    void TCP_012_disconnectLifetimeAndFreshConnection();
    void TCP_013_qt6ErrorSignalMetaobjectAndNoLegacyWarning();
    void TCP_014_serverSocketAfterClientDisconnect();
    void TCP_015_repeatedConnectionsAndServerSocketLifetime();
    void TCP_016_oldSocketDisconnectKeepsCurrentConnection();
    void TCP_017_frameSplitterShortPrefixIsIncomplete();
    void TCP_018_frameSplitterRejectsTooSmallPrefixes();
    void TCP_019_frameSplitterBuffersPlausiblePartialFrame();
    void TCP_020_frameSplitterMaxSizeBoundary();
    void TCP_021_protocolStructuralValidation();
    void frameSplitterPreservesPartialRemainder();
};

void RemoteControlContractTests::initTestCase() { qRegisterMetaType<InterfaceData>("InterfaceData"); }

void RemoteControlContractTests::TCP_001_lifecyclePortAndLoopback() {
    std::map<QString, ToFormMapper*> data;
    auto* server = new RemoteControlServer(&data);
    QVERIFY(server->GetPort() >= 4080);
    QTcpSocket client;
    client.connectToHost(QHostAddress::LocalHost, quint16(server->GetPort()));
    QTRY_COMPARE(client.state(), QAbstractSocket::ConnectedState);
    QPointer<RemoteControlServer> guard(server);
    delete server;
    QTRY_VERIFY(guard.isNull());
    QTRY_VERIFY(client.state() == QAbstractSocket::UnconnectedState);
    clear(data);
}

void RemoteControlContractTests::TCP_002_numericSetSignalAndGetBytes() {
    std::map<QString, ToFormMapper*> data; data["numeric"] = numeric(2.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender); QVERIFY(spy.isValid());
    QTcpSocket* client = connectClient(server, this);
    QCOMPARE(client->write(frame("set", "numeric", f64(-12.5))), qint64(31)); QVERIFY(client->waitForBytesWritten(1000));
    QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(0).toString(), QString("set")); QCOMPARE(spy.at(0).at(1).toString(), QString("numeric"));
    QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetAsDouble(), -12.5);
    QCOMPARE(data["numeric"]->GetAsDouble(), 2.0);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* getter = connectClient(server, this); QCOMPARE(getter->write(frame("get", "numeric")), qint64(23)); QVERIFY(getter->waitForBytesWritten(1000));
    const QByteArray reply = readReply(getter, 13); QCOMPARE(reply.size(), 13); QCOMPARE(uchar(reply.at(0)), uchar(0)); QCOMPARE(readU32(reply, 1), uint32_t(1)); QCOMPARE(readF64(reply, 5), 2.0);
    getter->disconnectFromHost(); QTRY_COMPARE(getter->state(), QAbstractSocket::UnconnectedState); clear(data);
}

void RemoteControlContractTests::TCP_003_stringListAndSelectionContracts() {
    std::map<QString, ToFormMapper*> data; data["text"] = text("old"); data["list"] = list({"first", "second"}); data["choice"] = selection("a", {"a", "b"});
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender); QTcpSocket* client = connectClient(server, this);
    client->write(frame("set", "text", "new")); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetString(), QString("ne")); QCOMPARE(data["text"]->GetString(), QString("old"));
    client->write(frame("set", "list", "only")); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 2); QCOMPARE(spy.at(1).at(2).value<InterfaceData>().GetStringList(), QStringList({"onl"}));
    client->write(frame("set", "choice", "b")); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 3); QCOMPARE(spy.at(2).at(2).value<InterfaceData>().GetGuiSelection().first, QString("a"));
    client->write(frame("set", "choice", "missing")); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 4); QCOMPARE(spy.at(3).at(2).value<InterfaceData>().GetGuiSelection().first, QString("a"));
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* getter = connectClient(server, this); getter->write(frame("get", "text")); QVERIFY(getter->waitForBytesWritten(1000)); const QByteArray reply = readReply(getter, 5 + 4 * 8); QCOMPARE(uchar(reply.at(0)), uchar(1)); QCOMPARE(readU32(reply, 1), uint32_t(4)); QCOMPARE(reply.mid(5, 4), QByteArray("old\0", 4)); getter->disconnectFromHost(); QTRY_COMPARE(getter->state(), QAbstractSocket::UnconnectedState); clear(data);
}

void RemoteControlContractTests::TCP_004_vectorAndUnknownIdResponses() {
    std::map<QString, ToFormMapper*> data; data["wave"] = vectors({1.0, 2.0}, {4.0, 5.0}); data["device::state"] = numeric(3.0);
    RemoteControlServer server(&data); QTcpSocket* client = connectClient(server, this);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* vectorClient = connectClient(server, this); vectorClient->write(frame("get", "wave")); QVERIFY(vectorClient->waitForBytesWritten(1000)); const QByteArray vectorReply = readReply(vectorClient, 37); QCOMPARE(uchar(vectorReply.at(0)), uchar(0)); QCOMPARE(readU32(vectorReply, 1), uint32_t(4)); QCOMPARE(readF64(vectorReply, 5), 1.0); QCOMPARE(readF64(vectorReply, 13), 2.0); QCOMPARE(readF64(vectorReply, 21), 4.0); QCOMPARE(readF64(vectorReply, 29), 5.0); vectorClient->disconnectFromHost(); QTRY_COMPARE(vectorClient->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* matchClient = connectClient(server, this); matchClient->write(frame("get", "device")); QVERIFY(matchClient->waitForBytesWritten(1000)); const QByteArray matchingReply = readReply(matchClient, 5 + 14 * 8); QCOMPARE(uchar(matchingReply.at(0)), uchar(1)); QCOMPARE(readU32(matchingReply, 1), uint32_t(14)); QCOMPARE(matchingReply.mid(5, 14), QByteArray("device::state\0", 14)); matchClient->disconnectFromHost(); QTRY_COMPARE(matchClient->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* absentClient = connectClient(server, this); absentClient->write(frame("get", "absent")); QVERIFY(absentClient->waitForBytesWritten(1000)); const QByteArray absentReply = readReply(absentClient, 5); QCOMPARE(absentReply, QByteArray("\0\0\0\0\0", 5)); absentClient->disconnectFromHost(); QTRY_COMPARE(absentClient->state(), QAbstractSocket::UnconnectedState); clear(data);
}

void RemoteControlContractTests::TCP_005_fragmentationAndCoalescedFrames() {
    std::map<QString, ToFormMapper*> data; data["one"] = numeric(1.0); data["two"] = numeric(2.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender); QTcpSocket* client = connectClient(server, this);
    const QByteArray first = frame("set", "one", f64(10.0)); client->write(first.left(4)); QVERIFY(client->waitForBytesWritten(1000)); QCoreApplication::processEvents(); QCOMPARE(spy.count(), 0);
    client->write(first.mid(4)); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 1);
    client->write(frame("set", "one", f64(11.0)) + frame("set", "two", f64(12.0))); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 3);
    QCOMPARE(spy.at(0).at(1).toString(), QString("one")); QCOMPARE(spy.at(1).at(1).toString(), QString("one")); QCOMPARE(spy.at(2).at(1).toString(), QString("two")); QCOMPARE(data["one"]->GetAsDouble(), 1.0); QCOMPARE(data["two"]->GetAsDouble(), 2.0);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); clear(data);
}

void RemoteControlContractTests::TCP_006_unknownCommandNullDataAndNoTimeoutReply() {
    RemoteControlServer server(nullptr); QSignalSpy spy(&server, &RemoteControlServer::MessageSender); QTcpSocket* client = connectClient(server, this);
    client->write(frame("get", "none")); QVERIFY(client->waitForBytesWritten(1000)); QCOMPARE(readReply(client, 5), QByteArray("\0\0\0\0\0", 5));
    client->write(frame("bad", "none")); QVERIFY(client->waitForBytesWritten(1000)); QVERIFY(!client->waitForReadyRead(150)); QCOMPARE(spy.count(), 0);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
}

void RemoteControlContractTests::TCP_007_disconnectRepeatedAndMultipleConnections() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(7.0); RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* first = connectClient(server, this); const QByteArray partial = frame("set", "value", f64(9.0)).left(8); first->write(partial); QVERIFY(first->waitForBytesWritten(1000)); first->disconnectFromHost(); QTRY_COMPARE(first->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* second = connectClient(server, this); second->write(frame("set", "value", f64(10.0))); QVERIFY(second->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetAsDouble(), 10.0); QCOMPARE(data["value"]->GetAsDouble(), 7.0);
    QTcpSocket* third = connectClient(server, this); third->write(frame("get", "value")); QVERIFY(third->waitForBytesWritten(1000)); const QByteArray reply = readReply(third, 13); QCOMPARE(readF64(reply, 5), 7.0);
    second->disconnectFromHost(); third->disconnectFromHost(); QTRY_COMPARE(second->state(), QAbstractSocket::UnconnectedState); QTRY_COMPARE(third->state(), QAbstractSocket::UnconnectedState); clear(data);
}

void RemoteControlContractTests::TCP_008_protocolByteBoundariesAndReplies() {
    std::map<QString, ToFormMapper*> data;
    data[""] = numeric(7.0);
    data["i8"] = scalar(int8_t(-8)); data["i16"] = scalar(int16_t(-160));
    data["i32"] = scalar(int32_t(-32000)); data["i64"] = scalar(int64_t(-6400000));
    data["u8"] = scalar(uint8_t(8)); data["u16"] = scalar(uint16_t(160));
    data["u32"] = scalar(uint32_t(32000)); data["u64"] = scalar(uint64_t(6400000));
    data["float"] = scalar(1.25f); data["double"] = scalar(-2.5); data["bool"] = scalar(true);
    data["text"] = text("text"); data["list"] = list({"list-first", "list-second"});
    data["choice"] = selection("selected", {"selected", "other"});
    data["vector"] = vectors({1.0, 2.0}, {3.0, 4.0});

    const QByteArray longestSafeTestId(4096, 'x');
    const QString longestSafeTestKey = QString::fromLatin1(longestSafeTestId);
    data[longestSafeTestKey] = numeric(42.0);

    RemoteControlServer server(&data);
    QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* client = connectClient(server, this);

    const QByteArray minimalGet = frame("get", QByteArray());
    QCOMPARE(minimalGet.size(), 16); QCOMPARE(readU32(minimalGet, 0), uint32_t(16));
    QCOMPARE(minimalGet.mid(4, 3), QByteArray("get")); QCOMPARE(readU32(minimalGet, 7), uint32_t(1));
    QCOMPARE(readU32(minimalGet, 11), uint32_t(0)); QCOMPARE(minimalGet.mid(15), QByteArray("\0", 1));
    QCOMPARE(client->write(minimalGet), qint64(minimalGet.size())); QVERIFY(client->waitForBytesWritten(1000));
    QCOMPARE(readReply(client, 13), numericReply(7.0));

    const QByteArray minimalSet = frame("set", QByteArray(), f64(-3.5));
    QCOMPARE(minimalSet.size(), 24); QCOMPARE(readU32(minimalSet, 0), uint32_t(24));
    QCOMPARE(minimalSet.mid(4, 3), QByteArray("set")); QCOMPARE(readU32(minimalSet, 7), uint32_t(1));
    QCOMPARE(readU32(minimalSet, 11), uint32_t(8)); QCOMPARE(minimalSet.mid(15, 1), QByteArray("\0", 1));
    QCOMPARE(minimalSet.mid(16), f64(-3.5));
    QCOMPARE(client->write(minimalSet), qint64(minimalSet.size())); QVERIFY(client->waitForBytesWritten(1000));
    QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(1).toString(), QString());
    QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetAsDouble(), -3.5); QCOMPARE(data[""]->GetAsDouble(), 7.0);

    const QByteArray embeddedNulPayload("A\0B\0", 4);
    const QByteArray embeddedNulSet = frame("set", "text", embeddedNulPayload);
    QCOMPARE(readU32(embeddedNulSet, 0), uint32_t(15 + 5 + 4));
    QCOMPARE(readU32(embeddedNulSet, 7), uint32_t(5)); QCOMPARE(readU32(embeddedNulSet, 11), uint32_t(4));
    QCOMPARE(embeddedNulSet.mid(20), embeddedNulPayload);
    QCOMPARE(client->write(embeddedNulSet), qint64(embeddedNulSet.size())); QVERIFY(client->waitForBytesWritten(1000));
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(2).value<InterfaceData>().GetString(), QString::fromLatin1("A\0B", 3));
    QCOMPARE(data["text"]->GetString(), QString("text"));

    const QByteArray longGet = frame("get", longestSafeTestId);
    QCOMPARE(readU32(longGet, 0), uint32_t(15 + longestSafeTestId.size() + 1));
    QCOMPARE(readU32(longGet, 7), uint32_t(longestSafeTestId.size() + 1)); QCOMPARE(readU32(longGet, 11), uint32_t(0));

    struct ReplyCase { QByteArray id; QByteArray expected; };
    const std::vector<ReplyCase> cases = {
        {"i8", numericReply(-8.0)}, {"i16", numericReply(-160.0)}, {"i32", numericReply(-32000.0)}, {"i64", numericReply(-6400000.0)},
        {"u8", numericReply(8.0)}, {"u16", numericReply(160.0)}, {"u32", numericReply(32000.0)}, {"u64", numericReply(6400000.0)},
        {"float", numericReply(1.25)}, {"double", numericReply(-2.5)}, {"bool", numericReply(1.0)},
        {"text", paddedStringReply("text")}, {"list", paddedStringReply("list-first")}, {"choice", paddedStringReply("selected")},
        {"vector", vectorReply({1.0, 2.0}, {3.0, 4.0})}, {longestSafeTestId, numericReply(42.0)}
    };
    QByteArray coalescedGets;
    qsizetype expectedBytes = 0;
    for (const ReplyCase& replyCase : cases) { coalescedGets += frame("get", replyCase.id); expectedBytes += replyCase.expected.size(); }
    QCOMPARE(client->write(coalescedGets), qint64(coalescedGets.size())); QVERIFY(client->waitForBytesWritten(1000));
    const QByteArray replies = readReply(client, int(expectedBytes)); QCOMPARE(replies.size(), expectedBytes);
    qsizetype offset = 0;
    for (const ReplyCase& replyCase : cases) { QCOMPARE(replies.mid(offset, replyCase.expected.size()), replyCase.expected); offset += replyCase.expected.size(); }

    const QByteArray unknown = frame("xyz", "unknown", QByteArray("x\0", 2));
    QCOMPARE(readU32(unknown, 0), uint32_t(15 + 8 + 2)); QCOMPARE(unknown.mid(4, 3), QByteArray("xyz"));
    QCOMPARE(readU32(unknown, 7), uint32_t(8)); QCOMPARE(readU32(unknown, 11), uint32_t(2));
    QCOMPARE(client->write(unknown), qint64(unknown.size())); QVERIFY(client->waitForBytesWritten(1000));
    QVERIFY(!client->waitForReadyRead(150)); QCOMPARE(spy.count(), 2);

    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    clear(data);
}

void RemoteControlContractTests::TCP_009_twoConnectedClientsUseLatestAcceptedSocket() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(3.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* clientA = connectClient(server, this);
    QCoreApplication::processEvents();
    QTcpSocket* clientB = connectClient(server, this);
    QCoreApplication::processEvents();

    clientA->write(frame("set", "value", f64(10.0))); QVERIFY(clientA->waitForBytesWritten(1000));
    QCoreApplication::processEvents(); QCOMPARE(spy.count(), 0);
    clientB->write(frame("set", "value", f64(20.0))); QVERIFY(clientB->waitForBytesWritten(1000));
    QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(1).toString(), QString("value"));
    QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetAsDouble(), 20.0);

    clientA->write(frame("get", "value")); QVERIFY(clientA->waitForBytesWritten(1000));
    QVERIFY(!clientA->waitForReadyRead(150)); QVERIFY(!clientB->waitForReadyRead(150));
    clientB->write(frame("get", "value")); QVERIFY(clientB->waitForBytesWritten(1000));
    QCOMPARE(readReply(clientB, 13), numericReply(3.0)); QVERIFY(!clientA->waitForReadyRead(150));

    clientA->disconnectFromHost(); clientB->disconnectFromHost();
    QTRY_COMPARE(clientA->state(), QAbstractSocket::UnconnectedState); QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState);
    clear(data);
}

void RemoteControlContractTests::TCP_010_fragmentStateIsDiscardedOnSecondConnection() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(4.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* clientA = connectClient(server, this);
    const QByteArray partial = frame("set", "value", f64(10.0));
    clientA->write(partial.left(8)); QVERIFY(clientA->waitForBytesWritten(1000));
    QCoreApplication::processEvents(); QCOMPARE(spy.count(), 0);

    QTcpSocket* clientB = connectClient(server, this);
    clientB->write(frame("set", "value", f64(20.0))); QVERIFY(clientB->waitForBytesWritten(1000));
    QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetAsDouble(), 20.0);

    clientA->write(partial.mid(8)); QVERIFY(clientA->waitForBytesWritten(1000));
    QCoreApplication::processEvents(); QCOMPARE(spy.count(), 1);
    clientB->write(frame("get", "value")); QVERIFY(clientB->waitForBytesWritten(1000));
    QCOMPARE(readReply(clientB, 13), numericReply(4.0)); QVERIFY(!clientA->waitForReadyRead(150));

    clientA->disconnectFromHost(); clientB->disconnectFromHost();
    QTRY_COMPARE(clientA->state(), QAbstractSocket::UnconnectedState); QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState);
    clear(data);
}

void RemoteControlContractTests::TCP_011_firstClientRequestsAfterSecondAcceptance() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(5.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* clientA = connectClient(server, this);
    QCoreApplication::processEvents();
    QTcpSocket* clientB = connectClient(server, this);
    QCoreApplication::processEvents();

    clientA->write(frame("set", "value", f64(30.0)) + frame("get", "value"));
    QVERIFY(clientA->waitForBytesWritten(1000)); QCoreApplication::processEvents();
    QCOMPARE(spy.count(), 0); QVERIFY(!clientA->waitForReadyRead(150)); QVERIFY(!clientB->waitForReadyRead(150));

    clientB->write(frame("get", "value")); QVERIFY(clientB->waitForBytesWritten(1000));
    QCOMPARE(readReply(clientB, 13), numericReply(5.0)); QCOMPARE(spy.count(), 0);

    clientA->disconnectFromHost(); clientB->disconnectFromHost();
    QTRY_COMPARE(clientA->state(), QAbstractSocket::UnconnectedState); QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState);
    clear(data);
}

void RemoteControlContractTests::TCP_012_disconnectLifetimeAndFreshConnection() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(6.0);
    auto* server = new RemoteControlServer(&data); QPointer<RemoteControlServer> serverGuard(server);
    QTcpSocket* clientA = connectClient(*server, this); QPointer<QTcpSocket> clientAGuard(clientA);
    QTcpSocket* clientB = connectClient(*server, this); QPointer<QTcpSocket> clientBGuard(clientB);
    clientA->disconnectFromHost(); QTRY_COMPARE(clientA->state(), QAbstractSocket::UnconnectedState); QVERIFY(!clientAGuard.isNull());
    clientB->disconnectFromHost(); QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState); QVERIFY(!clientBGuard.isNull());

    QTcpSocket* fresh = connectClient(*server, this); QPointer<QTcpSocket> freshGuard(fresh);
    fresh->write(frame("get", "value")); QVERIFY(fresh->waitForBytesWritten(1000)); QCOMPARE(readReply(fresh, 13), numericReply(6.0));
    fresh->disconnectFromHost(); QTRY_COMPARE(fresh->state(), QAbstractSocket::UnconnectedState);

    clientA->deleteLater(); clientB->deleteLater(); fresh->deleteLater();
    QTRY_VERIFY(clientAGuard.isNull()); QTRY_VERIFY(clientBGuard.isNull()); QTRY_VERIFY(freshGuard.isNull());
    delete server; QTRY_VERIFY(serverGuard.isNull());
    clear(data);
}

void RemoteControlContractTests::TCP_013_qt6ErrorSignalMetaobjectAndNoLegacyWarning() {
    QTcpSocket socket;
    QCOMPARE(socket.metaObject()->indexOfSignal("error(QAbstractSocket::SocketError)"), -1);
    QVERIFY(socket.metaObject()->indexOfSignal("errorOccurred(QAbstractSocket::SocketError)") >= 0);

    QtMessageCapture capture;
    RemoteControlServer server(nullptr);
    QTcpSocket* client = connectClient(server, this);
    QTRY_COMPARE(acceptedSockets(server).size(), 1);
    QVERIFY(!QtMessageCapture::contains("No such signal QTcpSocket::error(QAbstractSocket::SocketError)"));
    client->disconnectFromHost();
    QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
}

void RemoteControlContractTests::TCP_014_serverSocketAfterClientDisconnect() {
    RemoteControlServer server(nullptr);
    QTcpSocket* client = connectClient(server, this);
    QTRY_COMPARE(acceptedSockets(server).size(), 1);
    QTcpSocket* accepted = acceptedSockets(server).first();
    QPointer<QTcpSocket> acceptedGuard(accepted);
    QCOMPARE(accepted->parent(), static_cast<QObject*>(&server.tcpServer));

    client->write(frame("set", "partial", f64(1.0)).left(8));
    QVERIFY(client->waitForBytesWritten(1000));
    QTRY_VERIFY(!server.ConnectionState.GetFrameSplitter().Buffer.isEmpty());
    client->disconnectFromHost();
    QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    QTRY_VERIFY(acceptedGuard.isNull());
    QCOMPARE(server.ConnectionState.GetCurrentSocket(), static_cast<QTcpSocket*>(nullptr));
    QVERIFY(server.ConnectionState.GetFrameSplitter().Buffer.isEmpty());
    QTRY_COMPARE(acceptedSockets(server).size(), 0);

    QTcpSocket* fresh = connectClient(server, this);
    QTRY_COMPARE(acceptedSockets(server).size(), 1);
    QCOMPARE(acceptedSockets(server).last()->parent(), static_cast<QObject*>(&server.tcpServer));
    fresh->write(frame("get", "missing"));
    QVERIFY(fresh->waitForBytesWritten(1000));
    QCOMPARE(readReply(fresh, 5), QByteArray("\0\0\0\0\0", 5));
    fresh->disconnectFromHost();
    QTRY_COMPARE(fresh->state(), QAbstractSocket::UnconnectedState);
    QTRY_COMPARE(acceptedSockets(server).size(), 0);
}

void RemoteControlContractTests::TCP_015_repeatedConnectionsAndServerSocketLifetime() {
    auto* server = new RemoteControlServer(nullptr);
    QList<QPointer<QTcpSocket>> serverSockets;

    for (int cycle = 0; cycle < 3; ++cycle) {
        QTcpSocket* client = connectClient(*server, this);
        QTRY_COMPARE(acceptedSockets(*server).size(), 1);
        QPointer<QTcpSocket> accepted(acceptedSockets(*server).last());
        QCOMPARE(accepted->parent(), static_cast<QObject*>(&server->tcpServer));
        serverSockets.append(accepted);
        client->disconnectFromHost();
        QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
        QTRY_VERIFY(accepted.isNull());
        QTRY_COMPARE(acceptedSockets(*server).size(), 0);
        client->deleteLater();
    }

    QTcpSocket* pendingClient = connectClient(*server, this);
    QTRY_COMPARE(acceptedSockets(*server).size(), 1);
    QPointer<QTcpSocket> pendingSocket(acceptedSockets(*server).last());
    pendingClient->disconnectFromHost();
    QTRY_COMPARE(pendingClient->state(), QAbstractSocket::UnconnectedState);
    delete server;
    for (const QPointer<QTcpSocket>& accepted : serverSockets) QTRY_VERIFY(accepted.isNull());
    QTRY_VERIFY(pendingSocket.isNull());
}

void RemoteControlContractTests::TCP_016_oldSocketDisconnectKeepsCurrentConnection() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(8.0);
    RemoteControlServer server(&data);
    QTcpSocket* clientA = connectClient(server, this);
    QTRY_COMPARE(acceptedSockets(server).size(), 1);
    QPointer<QTcpSocket> serverSocketA(acceptedSockets(server).first());
    QTcpSocket* clientB = connectClient(server, this);
    QTRY_COMPARE(acceptedSockets(server).size(), 2);
    QTcpSocket* serverSocketB = acceptedSockets(server).last();
    QPointer<QTcpSocket> serverSocketBGuard(serverSocketB);
    QCOMPARE(server.ConnectionState.GetCurrentSocket(), serverSocketB);

    clientA->disconnectFromHost();
    QTRY_COMPARE(clientA->state(), QAbstractSocket::UnconnectedState);
    QTRY_VERIFY(serverSocketA.isNull());
    QCOMPARE(server.ConnectionState.GetCurrentSocket(), serverSocketB);
    QVERIFY(!serverSocketBGuard.isNull());

    clientB->write(frame("get", "value"));
    QVERIFY(clientB->waitForBytesWritten(1000));
    QCOMPARE(readReply(clientB, 13), numericReply(8.0));
    clientB->disconnectFromHost();
    QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState);
    QTRY_VERIFY(serverSocketBGuard.isNull());
    clear(data);
}

void RemoteControlContractTests::TCP_017_frameSplitterShortPrefixIsIncomplete() {
    for (int size = 0; size < 4; ++size) {
        RemoteControlFrameSplitter splitter;
        const QByteArray partial(size, '\x5a');
        QByteArray extracted("unchanged");
        splitter.Append(partial);
        QCOMPARE(splitter.TakeFrame(&extracted), RemoteControlFrameSplitter::FrameResult::Incomplete);
        QCOMPARE(splitter.Buffer, partial);
        QCOMPARE(extracted, QByteArray("unchanged"));
    }
}

void RemoteControlContractTests::TCP_018_frameSplitterRejectsTooSmallPrefixes() {
    for (uint32_t size = 0; size < RemoteControlFrameSplitter::MinimumFrameSize; ++size) {
        RemoteControlFrameSplitter splitter;
        QByteArray extracted;
        splitter.Append(u32(size));
        QCOMPARE(splitter.TakeFrame(&extracted), RemoteControlFrameSplitter::FrameResult::InvalidPrefix);
        QVERIFY(splitter.Buffer.isEmpty());
        QCOMPARE(splitter.TakeFrame(&extracted), RemoteControlFrameSplitter::FrameResult::Incomplete);
    }
}

void RemoteControlContractTests::TCP_019_frameSplitterBuffersPlausiblePartialFrame() {
    RemoteControlFrameSplitter splitter;
    QByteArray extracted;
    const QByteArray partial = u32(32) + QByteArray("get", 3) + QByteArray(4, '\0');
    splitter.Append(partial);
    QCOMPARE(splitter.TakeFrame(&extracted), RemoteControlFrameSplitter::FrameResult::Incomplete);
    QCOMPARE(splitter.Buffer, partial);
}

void RemoteControlContractTests::TCP_020_frameSplitterMaxSizeBoundary() {
    RemoteControlFrameSplitter splitter;
    QByteArray extracted;
    splitter.Append(u32(RemoteControlFrameSplitter::MaxFrameSize));
    QCOMPARE(splitter.TakeFrame(&extracted), RemoteControlFrameSplitter::FrameResult::Incomplete);
    QCOMPARE(splitter.Buffer.size(), int(sizeof(uint32_t)));

    RemoteControlFrameSplitter tooLarge;
    tooLarge.Append(u32(RemoteControlFrameSplitter::MaxFrameSize + 1));
    QCOMPARE(tooLarge.TakeFrame(&extracted), RemoteControlFrameSplitter::FrameResult::InvalidPrefix);
    QVERIFY(tooLarge.Buffer.isEmpty());
}

void RemoteControlContractTests::TCP_021_protocolStructuralValidation() {
    const QByteArray validSet = frame("set", "id", f64(3.0));
    const RemoteControlProtocol::DecodedFrame known = RemoteControlProtocol::DecodeValidatedFrame(validSet);
    QCOMPARE(known.Status, RemoteControlProtocol::DecodeStatus::Valid);
    QCOMPARE(known.CommandType, RemoteControlProtocol::Command::Set);
    QCOMPARE(known.Id, QString("id"));
    QCOMPARE(known.Payload, f64(3.0));
    QVERIFY(RemoteControlProtocol::HasNumericSetPayload(known));

    const RemoteControlProtocol::DecodedFrame unknown = RemoteControlProtocol::DecodeValidatedFrame(frame("bad", "id"));
    QCOMPARE(unknown.Status, RemoteControlProtocol::DecodeStatus::Valid);
    QCOMPARE(unknown.CommandType, RemoteControlProtocol::Command::Unknown);

    QByteArray idLengthZero = validSet; idLengthZero.replace(7, 4, u32(0));
    QCOMPARE(RemoteControlProtocol::DecodeValidatedFrame(idLengthZero).Status, RemoteControlProtocol::DecodeStatus::Invalid);

    QByteArray missingNul = validSet; missingNul[17] = 'x';
    QCOMPARE(RemoteControlProtocol::DecodeValidatedFrame(missingNul).Status, RemoteControlProtocol::DecodeStatus::Invalid);

    QByteArray totalMismatch = validSet; totalMismatch.replace(0, 4, u32(uint32_t(validSet.size() + 1)));
    QCOMPARE(RemoteControlProtocol::DecodeValidatedFrame(totalMismatch).Status, RemoteControlProtocol::DecodeStatus::Invalid);

    QByteArray sumMismatch = validSet; sumMismatch.replace(11, 4, u32(uint32_t(f64(3.0).size() + 1)));
    QCOMPARE(RemoteControlProtocol::DecodeValidatedFrame(sumMismatch).Status, RemoteControlProtocol::DecodeStatus::Invalid);

    QByteArray oversizedFields = u32(16) + QByteArray("get", 3) + u32(0xfffffff0U) + u32(32) + QByteArray("\0", 1);
    QCOMPARE(RemoteControlProtocol::DecodeValidatedFrame(oversizedFields).Status, RemoteControlProtocol::DecodeStatus::Invalid);

    const RemoteControlProtocol::DecodedFrame shortNumeric = RemoteControlProtocol::DecodeValidatedFrame(frame("set", "id", QByteArray(7, '\0')));
    QCOMPARE(shortNumeric.Status, RemoteControlProtocol::DecodeStatus::Valid);
    QVERIFY(!RemoteControlProtocol::HasNumericSetPayload(shortNumeric));
}

void RemoteControlContractTests::frameSplitterPreservesPartialRemainder() {
    const QByteArray first = frame("set", "one", f64(1.0));
    const QByteArray second = frame("get", "two");
    RemoteControlFrameSplitter splitter;
    QByteArray extracted;

    splitter.Append(first + second.left(7));
    QVERIFY(splitter.TakeCompleteFrame(&extracted));
    QCOMPARE(extracted, first);
    QVERIFY(!splitter.TakeCompleteFrame(&extracted));

    splitter.Append(second.mid(7));
    QVERIFY(splitter.TakeCompleteFrame(&extracted));
    QCOMPARE(extracted, second);
    QVERIFY(!splitter.TakeCompleteFrame(&extracted));
}

QTEST_MAIN(RemoteControlContractTests)
#include "RemoteControlContractTests.moc"
