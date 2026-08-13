#pragma once

#include <QString>
#include <QStringList>

class DataManagementSetClass;
class UIDataManagementSetClass;
class Platform_Interface;

enum class PluginLoadOutcomeKind
{
    Loaded,
    FileOpenError,
    ParseError,
    NoDevice
};

struct PluginLoadOutcome
{
    PluginLoadOutcomeKind kind;
    Platform_Interface* device = nullptr;
    QString parserError;
};

// Private value-like operation helper for the UI project-I/O facade.  It owns
// no QObject or adapter: each adapter is constructed for one operation with
// the same manager reference/pointer used by the legacy facade. Experiment
// reading and writing also keep the exact UI facade hierarchy required by the
// legacy XML adapters.
class ProjectIoCoordinator
{
public:
    explicit ProjectIoCoordinator(UIDataManagementSetClass& manager);

    bool ImportParameters(const QString& path) const;
    bool ExportParameters(const QString& path, const QStringList& exportIds) const;
    bool ExportMat(const QString& path, const QStringList& exportIds) const;
    bool ExportHdf5(const QString& path, const QStringList& exportIds) const;
    bool ExportLabData(const QString& path, QString* error) const;
    bool ImportLabData(const QString& path, QString* datasetRoot, QString* error) const;
    bool ImportMatData(const QString& path, QString* datasetRoot, QString* error) const;
    bool ReadExperiment(const QString& path) const;
    bool WriteExperiment(const QString& path) const;
    PluginLoadOutcome LoadPlugin(const QString& path) const;

private:
    DataManagementSetClass& manager;
    UIDataManagementSetClass& uiManager;
};
