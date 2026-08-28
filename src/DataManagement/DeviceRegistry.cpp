#include "DeviceRegistry.h"

#include "../plugins/platforminterface.h"

Platform_Interface* DeviceRegistry::Find(QString name) const
{
    auto it = Devices.find(name);
    if (it != Devices.end())
        return it->second.interface;
    return nullptr;
}

QString DeviceRegistry::Path(QString name) const
{
    auto it = DevicePaths.find(name);
    if (it != DevicePaths.end())
        return it->second;
    return nullptr;
}

void DeviceRegistry::Add(QString name, QString path, Platform_Interface* device)
{
    AddWithCleanupStrategy(name, path, device, CleanupStrategy::HostDelete);
}

bool DeviceRegistry::AddLegacyPlugin(QString name, QString path,
                                     Platform_Interface* device,
                                     QObject* pluginObject, QObject* messenger)
{
    if (Devices.find(name) != Devices.end())
        return false;

    AddWithCleanupStrategy(name, path, device, CleanupStrategy::RetainLegacyPlugin,
                           pluginObject, messenger);
    return true;
}

void DeviceRegistry::AddWithCleanupStrategy(QString name, QString path,
                                             Platform_Interface* device,
                                             CleanupStrategy cleanup,
                                             QObject* pluginObject,
                                             QObject* messenger)
{
    if (!Devices[name].interface) {
        Devices[name] = { device, path, cleanup, pluginObject,
                          device ? device->GetObject() : nullptr, messenger };
        DevicePaths[name] = path;
    }
}

void DeviceRegistry::Cleanup(DeviceRecord& record)
{
    if (!record.interface)
        return;

    switch (record.cleanup) {
    case CleanupStrategy::HostDelete:
        delete record.interface;
        break;
    case CleanupStrategy::RetainLegacyPlugin:
        // A Legacy-V1 plugin does not declare who owns the returned interface.
        // Stop the returned device before retiring the active record. The
        // factory root and interface remain application-resident because
        // Legacy-V1 does not declare their ownership.
        if (record.messenger) {
            QObject::disconnect(record.messenger, nullptr, record.pluginObject, nullptr);
            QObject::disconnect(record.pluginObject, nullptr, record.messenger, nullptr);
            if (record.deviceObject) {
                QObject::disconnect(record.messenger, nullptr, record.deviceObject, nullptr);
                QObject::disconnect(record.deviceObject, nullptr, record.messenger, nullptr);
            }
        }
        if (record.deviceObject) {
            // Do not broadcast through Messenger::MessageSender here: a
            // device may feed CloseProject back into the manager while it is
            // being torn down. The direct lifecycle call is required for
            // legacy factories whose root is only a Platform_Fabric.
            QMetaObject::invokeMethod(record.deviceObject, "MessageReceiver",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QStringLiteral("CloseProject")),
                                      Q_ARG(QString, record.deviceObject->objectName()),
                                      Q_ARG(InterfaceData, InterfaceData()));
        }
        break;
    case CleanupStrategy::PluginReleaseV2:
        // Prepared only: no current registration path can create this strategy.
        // Do not delete an interface with unproven ownership.
        break;
    }
}

void DeviceRegistry::Close(QString name)
{
    auto it = Devices.find(name);
    if (it != Devices.end()) {
        Cleanup(it->second);
        Devices.erase(it);
        DevicePaths.erase(name);
    }
}

void DeviceRegistry::RemoveDevices()
{
    for (auto entry : Devices)
        Cleanup(entry.second);
    Devices.clear();
}

void DeviceRegistry::ClearProjectDevices()
{
    RemoveDevices();
    DevicePaths.clear();
}

QList<QString> DeviceRegistry::Names() const
{
    QList<QString> names;
    for (auto entry : DevicePaths)
        names.push_back(entry.first);
    return names;
}

QList<QString> DeviceRegistry::Paths() const
{
    QList<QString> paths;
    for (auto entry : DevicePaths)
        paths.push_back(entry.second);
    return paths;
}
