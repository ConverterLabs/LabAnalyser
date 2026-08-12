#pragma once

#include <map>
#include <QString>
#include <QStringList>
#include <matio.h>

#include "../plugins/InterfaceDataType.h"
#include "../DataManagement/DataManagementClass.h"

class MatExporter
{
public:
    explicit MatExporter(DataManagementClass* Manager_);
    bool Export2Mat(QString Filename_, QStringList Ids_);

private:
    void WriteTimeStamp();
    void ExportChannels();

    QString Filename;
    std::map<QString, InterfaceData> Data;
    DataManagementClass* Manager = nullptr;
    mat_t* matfile = nullptr;
    QStringList Ids;
    matvar_t* exportedChannels = nullptr;
};

void SaveData2MatNV(QString Filename, std::map<QString, DataPair> Data);
