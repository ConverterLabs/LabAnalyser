#pragma once
#include "../plugins/InterfaceDataType.h"
#include "../DataManagement/DataManagementClass.h"

class Export2HDF5
{
public:
    Export2HDF5(DataManagementClass* Manager_);
    bool Export(QString Filename_,  QStringList Ids_);

private:
    std::map<QString, InterfaceData> Data;
    QString Filename;
    QStringList Ids;

    DataManagementClass* Manager = nullptr;
    uint32_t IDcounter = 0;

};

