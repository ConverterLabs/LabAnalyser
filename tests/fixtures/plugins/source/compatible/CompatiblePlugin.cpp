#include <QObject>
#include "plugins/platforminterface.h"

class CompatibleDevice : public QObject, public Platform_Interface {
public:
    InterfaceData* GetSymbol(const QString&) override { return nullptr; }
    QObject* GetObject() override { return this; }
    void MessageReceiver(const QString&, const QString&, InterfaceData) override {}
    void MessageSender(const QString&, const QString&, InterfaceData) override {}
};

class CompatiblePlugin : public QObject, public Platform_Fabric {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.EchoInterface")
    Q_INTERFACES(Platform_Fabric)
public:
    Platform_Interface* GetInterface(QObject* messenger) override {
        lastMessenger = messenger;
        ++getInterfaceCalls;
        device.setProperty("test_getInterfaceCalls", getInterfaceCalls);
        device.setProperty("test_messenger", QVariant::fromValue(static_cast<QObject*>(messenger)));
        return &device;
    }
    static QObject* lastMessenger;
    static int getInterfaceCalls;
private:
    CompatibleDevice device;
};
QObject* CompatiblePlugin::lastMessenger = nullptr;
int CompatiblePlugin::getInterfaceCalls = 0;
#include "CompatiblePlugin.moc"
