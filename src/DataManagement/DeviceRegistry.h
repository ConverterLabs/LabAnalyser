/***************************************************************************
** Internal device registry used exclusively by DataManagementClass.
**
** It preserves the legacy raw Platform_Interface ownership boundary: the
** first pointer and path registered for a name win; accepted pointers are
** deleted only by the existing explicit cleanup operations. Rejected
** duplicate pointers, QObjects returned by GetObject(), and QPluginLoaders
** remain outside this type's ownership.
***************************************************************************/

#ifndef DEVICEREGISTRY_H
#define DEVICEREGISTRY_H

#include <QList>
#include <QPointer>
#include <QString>

#include <map>

class Platform_Interface;

class DeviceRegistry
{
public:
    Platform_Interface* Find(QString name) const;
    QString Path(QString name) const;
    void Add(QString name, QString path, Platform_Interface* device);
    bool AddLegacyPlugin(QString name, QString path, Platform_Interface* device,
                         QObject* pluginObject, QObject* messenger);
    void Close(QString name);
    void RemoveDevices();
    void ClearProjectDevices();
    QList<QString> Names() const;
    QList<QString> Paths() const;

private:
    enum class CleanupStrategy {
        HostDelete,
        RetainLegacyPlugin,
        PluginReleaseV2
    };

    struct DeviceRecord {
        Platform_Interface* interface = nullptr;
        QString descriptorPath;
        CleanupStrategy cleanup = CleanupStrategy::HostDelete;
        QPointer<QObject> pluginObject;
        QPointer<QObject> deviceObject;
        QPointer<QObject> messenger;
    };

    void AddWithCleanupStrategy(QString name, QString path,
                                Platform_Interface* device,
                                CleanupStrategy cleanup,
                                QObject* pluginObject = nullptr,
                                QObject* messenger = nullptr);
    void Cleanup(DeviceRecord& record);

    std::map<QString, DeviceRecord> Devices;
    std::map<QString, QString> DevicePaths;
};

#endif
