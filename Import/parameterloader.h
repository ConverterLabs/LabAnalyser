#pragma once

#include <QXmlStreamReader>
#include "../DataManagement/DataManagementSetClass.h"



class ParameterLoader
{
public:
    ParameterLoader( DataManagementSetClass* Manager_);
    bool Load(QString path);
private:
    QXmlStreamReader reader;
    DataManagementSetClass* Manager = nullptr;

    void readParameterSets();
    void readEntry();

};

