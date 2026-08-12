#include <QtTest>
#include <QTcpSocket>
#include <QTcpServer>
#include <QPointer>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QTimer>
#include <cstring>
#include <cstdint>
#include <limits>

// Test-only seam: exposes the private QTcpServer member solely to observe its
// QObject-owned accepted sockets. Production headers and targets are unchanged.
#define private public
#include "RemoteControl/RemoteControlServer.h"
#undef private
#include "RemoteControl/RemoteControlFrameSplitter.h"
#include "RemoteControl/RemoteControlProtocol.h"
#include "DataManagement/DataManagementSetClass.h"

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

// Test-only fixture boundary: each observed socket remains QObject-owned by
// QTcpServer. The helper never deletes it; it only drives client disconnects
// and the event loop until production's disconnected()->deleteLater() path has
// completed before the stack-allocated server leaves scope.
bool finalizeRemoteServer(RemoteControlServer& server, std::initializer_list<QTcpSocket*> clients) {
    QList<QPointer<QTcpSocket>> observedSockets;
    for (QTcpSocket* socket : acceptedSockets(server)) observedSockets.append(socket);
    for (QTcpSocket* client : clients)
        if (client && client->state() != QAbstractSocket::UnconnectedState)
            client->disconnectFromHost();

    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 1000) {
        bool clientsClosed = true;
        for (QTcpSocket* client : clients)
            clientsClosed = clientsClosed && (!client || client->state() == QAbstractSocket::UnconnectedState);
        bool observedDestroyed = true;
        for (const QPointer<QTcpSocket>& socket : observedSockets)
            observedDestroyed = observedDestroyed && socket.isNull();
        if (clientsClosed && observedDestroyed && acceptedSockets(server).isEmpty())
            return true;

        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    bool observedDestroyed = true;
    for (const QPointer<QTcpSocket>& socket : observedSockets)
        observedDestroyed = observedDestroyed && socket.isNull();
    return observedDestroyed && acceptedSockets(server).isEmpty();
}

void compareLatin1QString(const QString& actual, const QByteArray& expectedBytes) {
    const QString expected = QString::fromLatin1(expectedBytes);
    QCOMPARE(actual.size(), expected.size());
    for (qsizetype index = 0; index < expected.size(); ++index)
        QCOMPARE(actual.at(index).unicode(), expected.at(index).unicode());
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
    void TCP_022_structurallyInvalidFrameIsDiscardedAndConnectionRecovers();
    void TCP_023_invalidPrefixAbortsCurrentConnection();
    void TCP_024_shortNumericSetIsDiscardedAndConnectionRecovers();
    void TCP_025_validUnknownCommandKeepsConnectionUsable();
    void TCP_026_coalescedInvalidAndValidFramesRecoverInOrder();
    void TCP_027_qStringAndStringListLegacyPayloads();
    void TCP_028_guiSelectionLegacyPayloads();
    void TCP_029_optionalTrailingNulHelper();
    void TCP_030_replyEncoderSizeLimits();
    void TCP_031_oversizedGetReplyAbortsAndFreshConnectionRecovers();
    void TCP_DM_001_tcpSetMutatesContainerThroughMessenger();
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
    getter->disconnectFromHost(); QTRY_COMPARE(getter->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client, getter})); clear(data);
}

void RemoteControlContractTests::TCP_003_stringListAndSelectionContracts() {
    std::map<QString, ToFormMapper*> data; data["text"] = text("old"); data["list"] = list({"first", "second"}); data["choice"] = selection("a", {"a", "b"});
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender); QTcpSocket* client = connectClient(server, this);
    client->write(frame("set", "text", "new")); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetString(), QString("new")); QCOMPARE(data["text"]->GetString(), QString("old"));
    client->write(frame("set", "list", "only")); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 2); QCOMPARE(spy.at(1).at(2).value<InterfaceData>().GetStringList(), QStringList({"only"}));
    client->write(frame("set", "choice", "b")); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 3); QCOMPARE(spy.at(2).at(2).value<InterfaceData>().GetGuiSelection().first, QString("b"));
    client->write(frame("set", "choice", "missing")); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 4); QCOMPARE(spy.at(3).at(2).value<InterfaceData>().GetGuiSelection().first, QString("a"));
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* getter = connectClient(server, this); getter->write(frame("get", "text")); QVERIFY(getter->waitForBytesWritten(1000)); const QByteArray reply = readReply(getter, 5 + 4 * 8); QCOMPARE(uchar(reply.at(0)), uchar(1)); QCOMPARE(readU32(reply, 1), uint32_t(4)); QCOMPARE(reply.mid(5, 4), QByteArray("old\0", 4)); getter->disconnectFromHost(); QTRY_COMPARE(getter->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client, getter})); clear(data);
}

void RemoteControlContractTests::TCP_004_vectorAndUnknownIdResponses() {
    std::map<QString, ToFormMapper*> data; data["wave"] = vectors({1.0, 2.0}, {4.0, 5.0}); data["device::state"] = numeric(3.0);
    RemoteControlServer server(&data); QTcpSocket* client = connectClient(server, this);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* vectorClient = connectClient(server, this); vectorClient->write(frame("get", "wave")); QVERIFY(vectorClient->waitForBytesWritten(1000)); const QByteArray vectorReply = readReply(vectorClient, 37); QCOMPARE(uchar(vectorReply.at(0)), uchar(0)); QCOMPARE(readU32(vectorReply, 1), uint32_t(4)); QCOMPARE(readF64(vectorReply, 5), 1.0); QCOMPARE(readF64(vectorReply, 13), 2.0); QCOMPARE(readF64(vectorReply, 21), 4.0); QCOMPARE(readF64(vectorReply, 29), 5.0); vectorClient->disconnectFromHost(); QTRY_COMPARE(vectorClient->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* matchClient = connectClient(server, this); matchClient->write(frame("get", "device")); QVERIFY(matchClient->waitForBytesWritten(1000)); const QByteArray matchingReply = readReply(matchClient, 5 + 14 * 8); QCOMPARE(uchar(matchingReply.at(0)), uchar(1)); QCOMPARE(readU32(matchingReply, 1), uint32_t(14)); QCOMPARE(matchingReply.mid(5, 14), QByteArray("device::state\0", 14)); matchClient->disconnectFromHost(); QTRY_COMPARE(matchClient->state(), QAbstractSocket::UnconnectedState);
    QTcpSocket* absentClient = connectClient(server, this); absentClient->write(frame("get", "absent")); QVERIFY(absentClient->waitForBytesWritten(1000)); const QByteArray absentReply = readReply(absentClient, 5); QCOMPARE(absentReply, QByteArray("\0\0\0\0\0", 5)); absentClient->disconnectFromHost(); QTRY_COMPARE(absentClient->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client, vectorClient, matchClient, absentClient})); clear(data);
}

void RemoteControlContractTests::TCP_005_fragmentationAndCoalescedFrames() {
    std::map<QString, ToFormMapper*> data; data["one"] = numeric(1.0); data["two"] = numeric(2.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender); QTcpSocket* client = connectClient(server, this);
    const QByteArray first = frame("set", "one", f64(10.0)); client->write(first.left(4)); QVERIFY(client->waitForBytesWritten(1000)); QCoreApplication::processEvents(); QCOMPARE(spy.count(), 0);
    client->write(first.mid(4)); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 1);
    client->write(frame("set", "one", f64(11.0)) + frame("set", "two", f64(12.0))); QVERIFY(client->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 3);
    QCOMPARE(spy.at(0).at(1).toString(), QString("one")); QCOMPARE(spy.at(1).at(1).toString(), QString("one")); QCOMPARE(spy.at(2).at(1).toString(), QString("two")); QCOMPARE(data["one"]->GetAsDouble(), 1.0); QCOMPARE(data["two"]->GetAsDouble(), 2.0);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client})); clear(data);
}

void RemoteControlContractTests::TCP_006_unknownCommandNullDataAndNoTimeoutReply() {
    RemoteControlServer server(nullptr); QSignalSpy spy(&server, &RemoteControlServer::MessageSender); QTcpSocket* client = connectClient(server, this);
    client->write(frame("get", "none")); QVERIFY(client->waitForBytesWritten(1000)); QCOMPARE(readReply(client, 5), QByteArray("\0\0\0\0\0", 5));
    client->write(frame("bad", "none")); QVERIFY(client->waitForBytesWritten(1000)); QVERIFY(!client->waitForReadyRead(150)); QCOMPARE(spy.count(), 0);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client}));
}

void RemoteControlContractTests::TCP_007_disconnectRepeatedAndMultipleConnections() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(7.0); RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* first = connectClient(server, this); QTRY_COMPARE(acceptedSockets(server).size(), 1); QPointer<QTcpSocket> firstServerSocket(acceptedSockets(server).first());
    const QByteArray partial = frame("set", "value", f64(9.0)).left(8); first->write(partial); QVERIFY(first->waitForBytesWritten(1000)); first->disconnectFromHost(); QTRY_COMPARE(first->state(), QAbstractSocket::UnconnectedState);
    QTRY_VERIFY(firstServerSocket.isNull()); QTRY_COMPARE(acceptedSockets(server).size(), 0);

    QTcpSocket* second = connectClient(server, this); QTRY_COMPARE(acceptedSockets(server).size(), 1); QPointer<QTcpSocket> secondServerSocket(acceptedSockets(server).first());
    second->write(frame("set", "value", f64(10.0))); QVERIFY(second->waitForBytesWritten(1000)); QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetAsDouble(), 10.0); QCOMPARE(data["value"]->GetAsDouble(), 7.0);
    QTcpSocket* third = connectClient(server, this); QTRY_COMPARE(acceptedSockets(server).size(), 2); QPointer<QTcpSocket> thirdServerSocket(acceptedSockets(server).last());
    third->write(frame("get", "value")); QVERIFY(third->waitForBytesWritten(1000)); const QByteArray reply = readReply(third, 13); QCOMPARE(readF64(reply, 5), 7.0);
    second->disconnectFromHost(); third->disconnectFromHost(); QTRY_COMPARE(second->state(), QAbstractSocket::UnconnectedState); QTRY_COMPARE(third->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {first, second, third}));
    QTRY_VERIFY(secondServerSocket.isNull()); QTRY_VERIFY(thirdServerSocket.isNull()); QTRY_COMPARE(acceptedSockets(server).size(), 0);
    clear(data);
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

    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client}));
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
    QTRY_COMPARE(clientA->state(), QAbstractSocket::UnconnectedState); QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {clientA, clientB}));
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
    QTRY_COMPARE(clientA->state(), QAbstractSocket::UnconnectedState); QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {clientA, clientB}));
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
    QTRY_COMPARE(clientA->state(), QAbstractSocket::UnconnectedState); QTRY_COMPARE(clientB->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {clientA, clientB}));
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
    QVERIFY(finalizeRemoteServer(*server, {clientA, clientB, fresh}));

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
    QPointer<QTcpSocket> acceptedGuard(acceptedSockets(server).first());
    QVERIFY(!QtMessageCapture::contains("No such signal QTcpSocket::error(QAbstractSocket::SocketError)"));
    client->disconnectFromHost();
    QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    QVERIFY(finalizeRemoteServer(server, {client}));
    QTRY_VERIFY(acceptedGuard.isNull());
    QTRY_COMPARE(acceptedSockets(server).size(), 0);
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
    QVERIFY(finalizeRemoteServer(server, {client}));
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
    QVERIFY(finalizeRemoteServer(server, {client, fresh}));
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
        QVERIFY(finalizeRemoteServer(*server, {client}));
        QTRY_VERIFY(accepted.isNull());
        QTRY_COMPARE(acceptedSockets(*server).size(), 0);
        client->deleteLater();
    }

    QTcpSocket* pendingClient = connectClient(*server, this);
    QTRY_COMPARE(acceptedSockets(*server).size(), 1);
    QPointer<QTcpSocket> pendingSocket(acceptedSockets(*server).last());
    pendingClient->disconnectFromHost();
    QTRY_COMPARE(pendingClient->state(), QAbstractSocket::UnconnectedState);
    QVERIFY(finalizeRemoteServer(*server, {pendingClient}));
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
    QVERIFY(finalizeRemoteServer(server, {clientA, clientB}));
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

void RemoteControlContractTests::TCP_022_structurallyInvalidFrameIsDiscardedAndConnectionRecovers() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(4.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* client = connectClient(server, this);
    QByteArray invalid = frame("set", "value", f64(8.0)); invalid[20] = 'x';
    client->write(invalid); QVERIFY(client->waitForBytesWritten(1000));
    QVERIFY(!client->waitForReadyRead(150)); QCOMPARE(spy.count(), 0);
    QCOMPARE(client->state(), QAbstractSocket::ConnectedState);

    client->write(frame("get", "value")); QVERIFY(client->waitForBytesWritten(1000));
    QCOMPARE(readReply(client, 13), numericReply(4.0)); QCOMPARE(spy.count(), 0);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client}));
    clear(data);
}

void RemoteControlContractTests::TCP_023_invalidPrefixAbortsCurrentConnection() {
    const QList<uint32_t> invalidSizes = {0, 15, RemoteControlFrameSplitter::MaxFrameSize + 1};
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(5.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);

    for (uint32_t size : invalidSizes) {
        QTcpSocket* client = connectClient(server, this);
        client->write(u32(size)); QVERIFY(client->waitForBytesWritten(1000));
        QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
        QCOMPARE(spy.count(), 0); QVERIFY(server.ConnectionState.GetFrameSplitter().Buffer.isEmpty());
        QVERIFY(!client->waitForReadyRead(50));
    }

    QTcpSocket* fresh = connectClient(server, this);
    fresh->write(frame("get", "value")); QVERIFY(fresh->waitForBytesWritten(1000));
    QCOMPARE(readReply(fresh, 13), numericReply(5.0)); QCOMPARE(spy.count(), 0);
    fresh->disconnectFromHost(); QTRY_COMPARE(fresh->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {fresh}));
    clear(data);
}

void RemoteControlContractTests::TCP_024_shortNumericSetIsDiscardedAndConnectionRecovers() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(2.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* client = connectClient(server, this);

    for (int size = 0; size < int(sizeof(double)); ++size) {
        client->write(frame("set", "value", QByteArray(size, '\0'))); QVERIFY(client->waitForBytesWritten(1000));
        QVERIFY(!client->waitForReadyRead(50)); QCOMPARE(spy.count(), 0); QCOMPARE(data["value"]->GetAsDouble(), 2.0);
        QCOMPARE(client->state(), QAbstractSocket::ConnectedState);
    }

    client->write(frame("set", "value", f64(9.0))); QVERIFY(client->waitForBytesWritten(1000));
    QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetAsDouble(), 9.0);
    QCOMPARE(data["value"]->GetAsDouble(), 2.0);
    client->write(frame("get", "value")); QVERIFY(client->waitForBytesWritten(1000));
    QCOMPARE(readReply(client, 13), numericReply(2.0));
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client}));
    clear(data);
}

void RemoteControlContractTests::TCP_025_validUnknownCommandKeepsConnectionUsable() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(6.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* client = connectClient(server, this);
    client->write(frame("bad", "value")); QVERIFY(client->waitForBytesWritten(1000));
    QVERIFY(!client->waitForReadyRead(150)); QCOMPARE(spy.count(), 0);
    QCOMPARE(client->state(), QAbstractSocket::ConnectedState);
    client->write(frame("get", "value")); QVERIFY(client->waitForBytesWritten(1000));
    QCOMPARE(readReply(client, 13), numericReply(6.0)); QCOMPARE(spy.count(), 0);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client}));
    clear(data);
}

void RemoteControlContractTests::TCP_026_coalescedInvalidAndValidFramesRecoverInOrder() {
    std::map<QString, ToFormMapper*> data; data["value"] = numeric(7.0);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* client = connectClient(server, this);
    QByteArray invalid = frame("set", "value", f64(1.0)); invalid[20] = 'x';
    client->write(invalid + frame("set", "value", f64(11.0))); QVERIFY(client->waitForBytesWritten(1000));
    QTRY_COMPARE(spy.count(), 1); QCOMPARE(spy.at(0).at(0).toString(), QString("set"));
    QCOMPARE(spy.at(0).at(1).toString(), QString("value")); QCOMPARE(spy.at(0).at(2).value<InterfaceData>().GetAsDouble(), 11.0);
    QCOMPARE(data["value"]->GetAsDouble(), 7.0); QVERIFY(!client->waitForReadyRead(100));
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client}));
    clear(data);
}

void RemoteControlContractTests::TCP_027_qStringAndStringListLegacyPayloads() {
    struct PayloadCase {
        QByteArray name;
        QByteArray payload;
        QByteArray expected;
    };
    const std::vector<PayloadCase> cases = {
        {"empty", QByteArray(), QByteArray()},
        {"one-byte", QByteArray("A", 1), QByteArray("A", 1)},
        {"multiple-bytes", QByteArray("AB", 2), QByteArray("AB", 2)},
        {"one-trailing-nul", QByteArray("AB\0", 3), QByteArray("AB", 2)},
        {"embedded-and-trailing-nul", QByteArray("A\0B\0", 4), QByteArray("A\0B", 3)},
        {"two-trailing-nuls", QByteArray("A\0\0", 3), QByteArray("A\0", 2)},
        {"latin-1", QByteArray("\xE4\0", 2), QByteArray("\xE4", 1)}
    };

    std::map<QString, ToFormMapper*> data;
    data["text"] = text("unchanged-text");
    data["list"] = list({"unchanged-first", "unchanged-second"});
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* client = connectClient(server, this);

    int expectedSignals = 0;
    for (const PayloadCase& payloadCase : cases) {
        client->write(frame("set", "text", payloadCase.payload));
        QVERIFY(client->waitForBytesWritten(1000));
        ++expectedSignals;
        QTRY_COMPARE(spy.count(), expectedSignals);
        QCOMPARE(spy.at(expectedSignals - 1).at(0).toString(), QString("set"));
        QCOMPARE(spy.at(expectedSignals - 1).at(1).toString(), QString("text"));
        InterfaceData textData = spy.at(expectedSignals - 1).at(2).value<InterfaceData>();
        QVERIFY(textData.IsString());
        compareLatin1QString(textData.GetString(), payloadCase.expected);

        client->write(frame("set", "list", payloadCase.payload));
        QVERIFY(client->waitForBytesWritten(1000));
        ++expectedSignals;
        QTRY_COMPARE(spy.count(), expectedSignals);
        QCOMPARE(spy.at(expectedSignals - 1).at(0).toString(), QString("set"));
        QCOMPARE(spy.at(expectedSignals - 1).at(1).toString(), QString("list"));
        InterfaceData listData = spy.at(expectedSignals - 1).at(2).value<InterfaceData>();
        QVERIFY(listData.IsStringList());
        const QStringList values = listData.GetStringList();
        QCOMPARE(values.size(), 1);
        compareLatin1QString(values.first(), payloadCase.expected);
    }

    QCOMPARE(data["text"]->GetString(), QString("unchanged-text"));
    QCOMPARE(data["list"]->GetStringList(), QStringList({"unchanged-first", "unchanged-second"}));
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client}));
    clear(data);
}

void RemoteControlContractTests::TCP_028_guiSelectionLegacyPayloads() {
    struct SelectionCase {
        QByteArray payload;
        QString expected;
    };
    const QString embeddedChoice = QString::fromLatin1(QByteArray("b\0x", 3));
    const QString latin1Choice(QChar(0x00e4));
    const QStringList choices = {"a", "b", embeddedChoice, latin1Choice};
    const std::vector<SelectionCase> cases = {
        {QByteArray("b", 1), QString("b")},
        {QByteArray("b\0", 2), QString("b")},
        {QByteArray("x\0", 2), QString("a")},
        {QByteArray(), QString("a")},
        {QByteArray("b\0\0", 3), QString("a")},
        {QByteArray("b\0x\0", 4), embeddedChoice},
        {QByteArray("b\0z\0", 4), QString("a")},
        {QByteArray("\xE4\0", 2), latin1Choice}
    };

    std::map<QString, ToFormMapper*> data;
    data["choice"] = selection("a", choices);
    RemoteControlServer server(&data); QSignalSpy spy(&server, &RemoteControlServer::MessageSender);
    QTcpSocket* client = connectClient(server, this);

    for (int index = 0; index < int(cases.size()); ++index) {
        client->write(frame("set", "choice", cases[index].payload));
        QVERIFY(client->waitForBytesWritten(1000));
        QTRY_COMPARE(spy.count(), index + 1);
        QCOMPARE(spy.at(index).at(0).toString(), QString("set"));
        QCOMPARE(spy.at(index).at(1).toString(), QString("choice"));
        InterfaceData selectionData = spy.at(index).at(2).value<InterfaceData>();
        QVERIFY(selectionData.IsGuiSelection());
        const GuiSelection selectionValue = selectionData.GetGuiSelection();
        QCOMPARE(selectionValue.first, cases[index].expected);
        QCOMPARE(selectionValue.second, choices);
        QVERIFY(!client->waitForReadyRead(50));
    }

    QCOMPARE(data["choice"]->GetGuiSelection().first, QString("a"));
    QCOMPARE(data["choice"]->GetGuiSelection().second, choices);
    client->disconnectFromHost(); QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState); QVERIFY(finalizeRemoteServer(server, {client}));
    clear(data);
}

void RemoteControlContractTests::TCP_029_optionalTrailingNulHelper() {
    const std::vector<QByteArray> payloads = {
        QByteArray(), QByteArray("A", 1), QByteArray("A\0", 2),
        QByteArray("A\0\0", 3), QByteArray("A\0B", 3)
    };
    const std::vector<QByteArray> expected = {
        QByteArray(), QByteArray("A", 1), QByteArray("A", 1),
        QByteArray("A\0", 2), QByteArray("A\0B", 3)
    };
    QCOMPARE(payloads.size(), expected.size());
    for (size_t index = 0; index < payloads.size(); ++index) {
        const QByteArray original = payloads[index];
        QCOMPARE(RemoteControlProtocol::RemoveOptionalTrailingNul(payloads[index]), expected[index]);
        QCOMPARE(payloads[index], original);
    }
}

void RemoteControlContractTests::TCP_030_replyEncoderSizeLimits() {
    const quint64 maxElements = (RemoteControlProtocol::MaxEncodedReplySize - 5) / sizeof(double);
    QVERIFY(RemoteControlProtocol::CanEncodePaddedReplyElements(maxElements));
    QVERIFY(RemoteControlProtocol::CanEncodeVectorReplyElements(maxElements - 1, 1));
    QVERIFY(!RemoteControlProtocol::CanEncodePaddedReplyElements(maxElements + 1));
    QVERIFY(!RemoteControlProtocol::CanEncodeVectorReplyElements(std::numeric_limits<quint64>::max(), 1));

    QByteArray encoded;
    const QString maxString(int(maxElements - 1), QLatin1Char('A'));
    QVERIFY(RemoteControlProtocol::TryEncodeStringReply(maxString, &encoded));
    QCOMPARE(encoded.size(), int(5 + maxElements * sizeof(double)));
    QCOMPARE(uchar(encoded.at(0)), uchar(1));
    QCOMPARE(readU32(encoded, 1), uint32_t(maxElements));
    QCOMPARE(encoded.mid(5, 3), QByteArray("AAA"));
    QVERIFY(!RemoteControlProtocol::TryEncodeStringReply(QString(int(maxElements), QLatin1Char('A')), &encoded));
}

void RemoteControlContractTests::TCP_031_oversizedGetReplyAbortsAndFreshConnectionRecovers() {
    std::map<QString, ToFormMapper*> data;
    const size_t oversizedElements = size_t((RemoteControlProtocol::MaxEncodedReplySize - 5) / sizeof(double)) + 1;
    data["large"] = vectors(std::vector<double>(oversizedElements, 1.0), {});
    data["small"] = numeric(9.0);
    RemoteControlServer server(&data);
    QSignalSpy messages(&server, &RemoteControlServer::MessageSender);
    QVERIFY(messages.isValid());

    QTcpSocket* client = connectClient(server, this);
    client->write(frame("get", "large"));
    QVERIFY(client->waitForBytesWritten(1000));
    QTRY_COMPARE(client->state(), QAbstractSocket::UnconnectedState);
    QCOMPARE(messages.count(), 0);

    QTcpSocket* fresh = connectClient(server, this);
    fresh->write(frame("get", "small"));
    QVERIFY(fresh->waitForBytesWritten(1000));
    QCOMPARE(readReply(fresh, 13), numericReply(9.0));
    QCOMPARE(messages.count(), 0);
    QVERIFY(finalizeRemoteServer(server, {client, fresh}));
    clear(data);
}

void RemoteControlContractTests::TCP_DM_001_tcpSetMutatesContainerThroughMessenger() {
    QObject root;
    root.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&root);
    const QString id("tcp::manager::double");
    manager.AddContainerElement(id, "double", "Parameter", "");
    manager.GetContainer(id)->SetData(2.0);

    ToFormMapper* const mapper = manager.GetContainer(id);
    QVERIFY(mapper);
    QCOMPARE(mapper->GetDouble(), 2.0);
    QCOMPARE(manager.GetContainerCount(), 1);

    RemoteControlServer server(manager.GetContainerPointer());
    MessengerClass* const messenger = manager.GetMessenger();
    QVERIFY(messenger);

    QStringList order;
    QObject::connect(&server, &RemoteControlServer::MessageSender, &root,
                     [&order](const QString&, const QString&, InterfaceData) { order << "server"; });
    QObject::connect(&server, &RemoteControlServer::MessageSender,
                     messenger, &MessengerClass::MessageTransmitter);
    QObject::connect(messenger, &MessengerClass::SetData, &root,
                     [&order](const QString&, InterfaceData) { order << "messenger-set"; });
    QObject::connect(messenger, &MessengerClass::NewDataReceived, &root,
                     [&order](const QString&) { order << "messenger-new-data"; });
    QObject::connect(messenger, &MessengerClass::MessageSender, &root,
                     [&order](const QString&, const QString&, InterfaceData) { order << "messenger-send"; });

    QSignalSpy serverMessages(&server, &RemoteControlServer::MessageSender);
    QSignalSpy messengerSet(messenger, &MessengerClass::SetData);
    QSignalSpy messengerNewData(messenger, &MessengerClass::NewDataReceived);
    QSignalSpy messengerMessages(messenger, &MessengerClass::MessageSender);
    QVERIFY(serverMessages.isValid());
    QVERIFY(messengerSet.isValid());
    QVERIFY(messengerNewData.isValid());
    QVERIFY(messengerMessages.isValid());

    QTcpSocket* client = connectClient(server, this);
    client->write(frame("get", id.toLatin1()));
    QVERIFY(client->waitForBytesWritten(1000));
    QCOMPARE(readReply(client, 13), numericReply(2.0));
    QCOMPARE(serverMessages.count(), 0);
    QCOMPARE(messengerSet.count(), 0);
    QCOMPARE(messengerNewData.count(), 0);
    QCOMPARE(messengerMessages.count(), 0);

    client->write(frame("set", id.toLatin1(), f64(12.5)));
    QVERIFY(client->waitForBytesWritten(1000));
    QTRY_COMPARE(serverMessages.count(), 1);
    QTRY_COMPARE(messengerSet.count(), 1);
    QTRY_COMPARE(messengerNewData.count(), 1);
    QTRY_COMPARE(messengerMessages.count(), 1);

    QCOMPARE(order, QStringList({"server", "messenger-set", "messenger-new-data", "messenger-send"}));
    QCOMPARE(serverMessages.at(0).at(0).toString(), QString("set"));
    QCOMPARE(serverMessages.at(0).at(1).toString(), id);
    QCOMPARE(serverMessages.at(0).at(2).value<InterfaceData>().GetDouble(), 12.5);
    QCOMPARE(messengerSet.at(0).at(0).toString(), id);
    QCOMPARE(messengerSet.at(0).at(1).value<InterfaceData>().GetDouble(), 12.5);
    QCOMPARE(messengerNewData.at(0).at(0).toString(), id);
    QCOMPARE(messengerMessages.at(0).at(0).toString(), QString("set"));
    QCOMPARE(messengerMessages.at(0).at(1).toString(), id);
    QCOMPARE(messengerMessages.at(0).at(2).value<InterfaceData>().GetDouble(), 12.5);

    QCOMPARE(manager.GetContainer(id), mapper);
    QCOMPARE(mapper->GetDouble(), 12.5);
    QCOMPARE(manager.GetContainerCount(), 1);
    QCOMPARE(manager.GetFormFileCount(), 0);

    client->write(frame("get", id.toLatin1()));
    QVERIFY(client->waitForBytesWritten(1000));
    QCOMPARE(readReply(client, 13), numericReply(12.5));
    QCOMPARE(serverMessages.count(), 1);
    QCOMPARE(messengerSet.count(), 1);
    QCOMPARE(messengerNewData.count(), 1);
    QCOMPARE(messengerMessages.count(), 1);

    QVERIFY(finalizeRemoteServer(server, {client}));
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
