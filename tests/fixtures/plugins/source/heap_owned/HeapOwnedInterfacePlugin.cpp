#include <QObject>
#include <QPointer>
#include <QVariant>

#include "plugins/platforminterface.h"

// Test fixture only: GetInterface allocates a standalone interface.  The
// plugin root does not delete it; a test may perform one controlled cleanup
// only after the manager has demonstrably performed no implicit cleanup.
class HeapOwnedDevice : public QObject, public Platform_Interface
{
    Q_OBJECT
public:
    explicit HeapOwnedDevice(QObject* telemetry) : Telemetry(telemetry)
    {
        setObjectName("HeapOwnedInterfaceDevice");
        setProperty("test_messageReceives", 0);
        setProperty("test_pluginRoot", QVariant::fromValue(telemetry));
    }
    ~HeapOwnedDevice() override
    {
        if (Telemetry)
            Telemetry->setProperty("test_interfaceDestructions", Telemetry->property("test_interfaceDestructions").toInt() + 1);
    }

    InterfaceData* GetSymbol(const QString&) override { return nullptr; }
    QObject* GetObject() override { return this; }
public slots:
    void MessageReceiver(const QString&, const QString&, InterfaceData) override
    {
        setProperty("test_messageReceives", property("test_messageReceives").toInt() + 1);
    }

signals:
    void MessageSender(const QString&, const QString&, InterfaceData);

private:
    QObject* Telemetry;
};

class HeapOwnedInterfacePlugin : public QObject, public Platform_Fabric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.EchoInterface")
    Q_INTERFACES(Platform_Fabric)
public:
    HeapOwnedInterfacePlugin()
    {
        setObjectName("HeapOwnedInterfacePluginRoot");
        setProperty("test_getInterfaceCalls", 0);
        setProperty("test_interfaceDestructions", 0);
    }

    Platform_Interface* GetInterface(QObject* messenger) override
    {
        setProperty("test_getInterfaceCalls", property("test_getInterfaceCalls").toInt() + 1);
        setProperty("test_messenger", QVariant::fromValue(messenger));
        if (!Device)
            Device = new HeapOwnedDevice(this);
        return Device;
    }

private:
    // Test-fixture-only observation: PLUGIN_010 may perform its documented
    // controlled heap cleanup, after which a later independent load must not
    // return the stale member value.
    QPointer<HeapOwnedDevice> Device;
};

#include "HeapOwnedInterfacePlugin.moc"
