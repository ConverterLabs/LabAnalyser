#pragma once

#include <QString>
#include <QStringList>

class DataManagementSetClass;
class UIDataManagementSetClass;

// Private value-like operation helper for the UI project-I/O facade.  It owns
// no QObject or adapter: each adapter is constructed for one operation with
// the same manager reference/pointer used by the legacy facade. Experiment
// reading also keeps the exact UI facade hierarchy required by
// XmlExperimentReader.
class ProjectIoCoordinator
{
public:
    explicit ProjectIoCoordinator(UIDataManagementSetClass& manager);

    bool ImportParameters(const QString& path) const;
    bool ExportParameters(const QString& path, const QStringList& exportIds) const;
    bool ExportMat(const QString& path, const QStringList& exportIds) const;
    bool ExportHdf5(const QString& path, const QStringList& exportIds) const;
    bool ReadExperiment(const QString& path) const;

private:
    DataManagementSetClass& manager;
    UIDataManagementSetClass& uiManager;
};
