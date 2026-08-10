#include "ProjectIoCoordinator.h"

#include "DataManagementSetClass.h"
#include "UIDataManagementSetClass.h"
#include "../Export/Export2Mat.h"
#include "../Export/export2highfive.h"
#include "../Export/exportinputs2xml.h"
#include "../Import/parameterloader.h"
#include "../LoadSave/xmlexperimentreader.h"

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
