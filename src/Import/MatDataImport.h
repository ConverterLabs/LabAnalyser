#pragma once

#include <QString>

class DataManagementSetClass;

// Reads the MAT v5 ExportedChannels schema emitted by Export2Mat.
namespace MatDataImport {
bool Import(DataManagementSetClass& manager, const QString& path, QString* datasetRoot = nullptr,
            QString* error = nullptr);
}
