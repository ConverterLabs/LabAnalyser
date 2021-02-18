#pragma once
#include "../plugins/InterfaceDataType.h"
#include "../DataManagement/DataManagementSetClass.h"
#include <QXmlStreamReader>

class ExportInputs2Xml
{
public:

    ExportInputs2Xml(DataManagementSetClass* Manager_);
    bool Export2XML(QString Filename_,  QStringList Ids_);
    void WriteParameterSet();
    void WriteEntry();
    void WriteParameter();
    void WriteValue();


    QStringList Ids;

private:
    QString Filename;
    std::map<QString, InterfaceData> Data;
    DataManagementSetClass* Manager = nullptr;
    int ExportCounter = 0;

    QXmlStreamWriter xmlWriter;

};

