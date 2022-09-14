/***************************************************************************
**                                                                        **
**  LabAnlyser, a plugin based data modification and visualization tool   **
**  Copyright (C) 2015-2021 Andreas Hoffmann                              **
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


#include "parameterloader.h"
#include <QFile>
#include <QDir>
#include <QThread>

ParameterLoader::ParameterLoader( DataManagementSetClass* Manager_)
{
    Manager= Manager_;

}

bool ParameterLoader::Load(QString Path)
{
    bool Error = false;
    //create a backup
    QFile file(Path);
    QDir T(QFileInfo(file).absoluteDir());
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        Manager->GetMessenger()->SendError("Can't open file: " + file.errorString());
        return true;
    }
    reader.setDevice(&file);

    if (reader.readNextStartElement()) {
        if (reader.name() == QString("ParameterSet"))
            readParameterSets();
        else
        {
            reader.raiseError(QObject::tr("Not a Parameter file!"));
            Error = true;
        }
    }
    file.close();


    if(reader.error())
        Manager->GetMessenger()->SendError(reader.errorString());

    return Error||reader.error();

}

void ParameterLoader::readParameterSets()
{
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("entry"))
            readEntry();
       else
           reader.skipCurrentElement();
   }
}

void ParameterLoader::readEntry()
{
    QString Id;
    while(reader.readNextStartElement())
    {
       if(reader.name() ==  QString("Parameter"))
       {
           Id =  reader.readElementText().trimmed();
       }
       else if(reader.name() ==  QString("Value"))
       {
           if(!Id.isEmpty())
           {
                auto con = Manager->GetContainer(Id);
                if(con)
                {
                    InterfaceData tmp = *(Manager->GetContainer(Id));
                    tmp.SetDataKeepType(reader.readElementText().trimmed());
                    Manager->GetMessenger()->MessageTransmitter("set", Id, tmp);
                    QThread::msleep(10);
                }
                else
                {
                    Manager->GetMessenger()->SendInfo("Parameter " + Id + " unkown!");
                    reader.skipCurrentElement();
                }
           }
       }
       else
           reader.skipCurrentElement();
   }
}

