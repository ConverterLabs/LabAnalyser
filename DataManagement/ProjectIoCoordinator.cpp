#include "ProjectIoCoordinator.h"

#include "DataManagementSetClass.h"
#include "../Export/Export2Mat.h"
#include "../Export/export2highfive.h"
#include "../Export/exportinputs2xml.h"
#include "../Import/parameterloader.h"

ProjectIoCoordinator::ProjectIoCoordinator(DataManagementSetClass& manager_)
    : manager(manager_)
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
