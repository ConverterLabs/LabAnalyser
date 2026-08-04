#include <QtTest>
#include <QTcpSocket>
#include <QPointer>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <cstring>

#include "RemoteControl/RemoteControlServer.h"

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
ToFormMapper* text(const QString& value, const QString& type = "string") { auto* result = new ToFormMapper(type, "Parameter"); result->SetData(value); return result; }
ToFormMapper* list(const QStringList& value) { auto* result = new ToFormMapper("QStringList", "Parameter"); result->SetData(value); return result; }
ToFormMapper* selection(const QString& current, const QStringList& choices) { auto* result = new ToFormMapper("GuiSelection", "Parameter"); result->SetData(GuiSelection(current, choices)); return result; }
ToFormMapper* vectors(const std::vector<double>& time, const std::vector<double>& data) {
    auto* result = new ToFormMapper("DataPair", "Data");
    result->SetData(DataPair(boost::shared_ptr<std::vector<double>>(new std::vector<double>(time)), boost::shared_ptr<std::vector<double>>(new std::vector<double>(data))));
    return result;
}
void clear(std::map<QString, ToFormMapper*>& data) { for (auto pair : data) delete pair.second; data.clear(); }
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

QTEST_MAIN(RemoteControlContractTests)
#include "RemoteControlContractTests.moc"
