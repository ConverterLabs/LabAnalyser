#include "DeviceRegistry.h"

#include "../plugins/platforminterface.h"

Platform_Interface* DeviceRegistry::Find(QString name) const
{
    auto it = Devices.find(name);
    if (it != Devices.end())
        return it->second;
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
    if (!Devices[name]) {
        Devices[name] = device;
        DevicePaths[name] = path;
    }
}

void DeviceRegistry::Close(QString name)
{
    auto it = Devices.find(name);
    if (it != Devices.end()) {
        delete it->second;
        Devices.erase(it);
        DevicePaths.erase(name);
    }
}

void DeviceRegistry::RemoveDevices()
{
    for (auto entry : Devices)
        if (entry.second)
            delete entry.second;
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
