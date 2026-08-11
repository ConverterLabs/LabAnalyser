#include <QObject>
#include <QVariant>

#include "plugins/platforminterface.h"

// Test fixture only: the interface is a C++ member of the plugin root.  It is
// deliberately not a host-deletable object and is never registered for a
// DeviceRegistry cleanup test.
class MemberOwnedDevice : public QObject, public Platform_Interface
{
    Q_OBJECT
public:
    explicit MemberOwnedDevice(QObject* telemetry) : Telemetry(telemetry) {}
    ~MemberOwnedDevice() override
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

class MemberOwnedInterfacePlugin : public QObject, public Platform_Fabric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.EchoInterface")
    Q_INTERFACES(Platform_Fabric)
public:
    MemberOwnedInterfacePlugin() : Device(nullptr)
    {
        setObjectName("MemberOwnedInterfacePluginRoot");
        Device.setObjectName("MemberOwnedInterfaceDevice");
        Device.setProperty("test_messageReceives", 0);
        Device.setProperty("test_pluginRoot", QVariant::fromValue(static_cast<QObject*>(this)));
        setProperty("test_getInterfaceCalls", 0);
        setProperty("test_interfaceDestructions", 0);
    }

    Platform_Interface* GetInterface(QObject* messenger) override
    {
        setProperty("test_getInterfaceCalls", property("test_getInterfaceCalls").toInt() + 1);
        setProperty("test_messenger", QVariant::fromValue(messenger));
        return &Device;
    }

private:
    MemberOwnedDevice Device;
};

#include "MemberOwnedInterfacePlugin.moc"
