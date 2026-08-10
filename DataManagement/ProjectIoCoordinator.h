#pragma once

#include <QString>
#include <QStringList>

class DataManagementSetClass;

// Private value-like operation helper for the UI project-I/O facade.  It owns
// no QObject or adapter: each adapter is constructed for one operation with
// the same manager reference/pointer used by the legacy facade.
class ProjectIoCoordinator
{
public:
    explicit ProjectIoCoordinator(DataManagementSetClass& manager);

    bool ImportParameters(const QString& path) const;
    bool ExportParameters(const QString& path, const QStringList& exportIds) const;
    bool ExportMat(const QString& path, const QStringList& exportIds) const;
    bool ExportHdf5(const QString& path, const QStringList& exportIds) const;

private:
    DataManagementSetClass& manager;
};
