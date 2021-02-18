/***************************************************************************
**                                                                        **
**  This file is part of LabAnlyser.                                      **
**                                                                        **
**  LabAnlyser is free software: you can redistribute it and/or modify ´  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
****************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <QMainWindow>
#include "../plugins/InterfaceDataType.h"
#include "../DataManagement/DataManagementClass.h"
//#include "matOut.h"


class MatExporter
{
public:
    MatExporter(DataManagementClass* Manager_);
    bool Export2Mat(QString Filename_,  QStringList Ids_);
    void ExportChannel(QString ID);

private:
    QString Filename;
    std::map<QString, InterfaceData> Data;
    DataManagementClass* Manager = nullptr;
    FILE* matfile = nullptr;

    QStringList Ids;

    void WriteTimeStamp();
    void ExportChannels();

    std::vector<std::string> fieldnames_;
    std::vector<const char*> fieldNames;
    void* mxStruct = nullptr;
    uint32_t IDcounter = 0;


};

void SaveData2MatNV(QString Filename , std::map<QString, DataPair> Data);
