#pragma once

#include <QString>

class DataManagementSetClass;

// Versioned LabAnalyser data archive: JSON metadata followed by binary payloads.
namespace LabDataArchive {
bool ExportAll(DataManagementSetClass& manager, const QString& path, QString* error = nullptr);
bool Import(DataManagementSetClass& manager, const QString& path, QString* datasetRoot = nullptr,
            QString* error = nullptr);
}
