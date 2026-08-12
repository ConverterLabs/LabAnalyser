#include "ProjectIoCoordinator.h"

#include "DataManagementSetClass.h"
#include "UIDataManagementSetClass.h"
#include "../Export/Export2Mat.h"
#include "../Export/export2highfive.h"
#include "../Export/exportinputs2xml.h"
#include "../Import/parameterloader.h"
#include "../LoadSave/xmlexperimentreader.h"
#include "../LoadSave/xmlexperimentwriter.h"
#include "../LoadSave/loadplugin.h"

#include <QFile>

ProjectIoCoordinator::ProjectIoCoordinator(UIDataManagementSetClass& manager_)
    : manager(manager_), uiManager(manager_)
{
}

bool ProjectIoCoordinator::ImportParameters(const QString& path) const
{
    ParameterLoader loader(&manager);
    return loader.Load(path);
}

bool ProjectIoCoordinator::ExportParameters(const QString& path, const QStringList& exportIds) const
{
    ExportInputs2Xml exporter(manager);
    return exporter.Export2XML(path, exportIds);
}

bool ProjectIoCoordinator::ExportMat(const QString& path, const QStringList& exportIds) const
{
    MatExporter exporter(&manager);
    return exporter.Export2Mat(path, exportIds);
}

bool ProjectIoCoordinator::ExportHdf5(const QString& path, const QStringList& exportIds) const
{
    Export2HDF5 exporter(&manager);
    return exporter.Export(path, exportIds);
}

bool ProjectIoCoordinator::ReadExperiment(const QString& path) const
{
    XmlExperimentReader reader(&uiManager, uiManager.GetMessenger(), &uiManager);
    return reader.read(path);
}

bool ProjectIoCoordinator::WriteExperiment(const QString& path) const
{
    xmlexperimentwriter writer(&uiManager, uiManager.GetMessengerRef(), uiManager);
    return writer.write(path);
}

PluginLoadOutcome ProjectIoCoordinator::LoadPlugin(const QString& path) const
{
    class LoadPlugin loader(&uiManager, uiManager.GetMessenger());
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return { PluginLoadOutcomeKind::FileOpenError, nullptr, QString() };

    if (loader.read(&file, path))
        return { PluginLoadOutcomeKind::ParseError, nullptr, loader.errorString() };

    Platform_Interface* device = loader.GetNewDevice();
    if (!device)
        return { PluginLoadOutcomeKind::NoDevice, nullptr, QString() };

    return { PluginLoadOutcomeKind::Loaded, device, QString() };
}
