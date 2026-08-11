#include <QObject>
#include <QVariant>

#include "plugins/platforminterface.h"

// Test fixture only: GetInterface allocates a standalone interface.  The
// plugin root does not delete it; a test may perform one controlled cleanup
// only after the manager has demonstrably performed no implicit cleanup.
class HeapOwnedDevice : public QObject, public Platform_Interface
{
public:
    explicit HeapOwnedDevice(QObject* telemetry) : Telemetry(telemetry)
    {
        setObjectName("HeapOwnedInterfaceDevice");
        setProperty("test_pluginRoot", QVariant::fromValue(telemetry));
    }
    ~HeapOwnedDevice() override
    {
        if (Telemetry)
            Telemetry->setProperty("test_interfaceDestructions", Telemetry->property("test_interfaceDestructions").toInt() + 1);
    }

    InterfaceData* GetSymbol(const QString&) override { return nullptr; }
    QObject* GetObject() override { return this; }
    void MessageReceiver(const QString&, const QString&, InterfaceData) override {}
    void MessageSender(const QString&, const QString&, InterfaceData) override {}

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
    HeapOwnedDevice* Device = nullptr;
};

#include "HeapOwnedInterfacePlugin.moc"
