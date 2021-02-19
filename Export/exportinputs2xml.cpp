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


#include "exportinputs2xml.h"
#include <QFile>
#include <QDir>


ExportInputs2Xml::ExportInputs2Xml(DataManagementSetClass* Manager_)
{
    Manager = Manager_;

}
bool ExportInputs2Xml::Export2XML(QString Filename_,  QStringList Ids_)
{


    ExportCounter = 0;
    Filename = Filename_;
    Ids = Ids_;
    //create a backup

    QFile file(Filename);
    QDir T(QFileInfo(file).absoluteDir());
    if(!file.open(QIODevice::WriteOnly)){
        Manager->GetMessenger()->SendError("Can't open file: " + file.errorString());
        return true;
    }
    xmlWriter.setDevice(&file);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    WriteParameterSet();

    xmlWriter.writeEndDocument();



}

void ExportInputs2Xml::WriteParameterSet()
{

    xmlWriter.writeStartElement("ParameterSet");
    for(ExportCounter = 0; ExportCounter < Ids.size(); ExportCounter++)
    {
     if(Manager->GetContainer(Ids[ExportCounter]))
         WriteEntry();
    }
    xmlWriter.writeEndElement();

}

void ExportInputs2Xml::WriteEntry()
{
    xmlWriter.writeStartElement("entry");
        WriteParameter();
        WriteValue();
    xmlWriter.writeEndElement();
}

void ExportInputs2Xml::WriteParameter()
{
    xmlWriter.writeStartElement("Parameter");
         xmlWriter.writeCharacters(Ids[ExportCounter]);
    xmlWriter.writeEndElement();
}

void ExportInputs2Xml::WriteValue()
{
    xmlWriter.writeStartElement("Value");
         xmlWriter.writeCharacters(Manager->GetContainer(Ids[ExportCounter])->GetString());
    xmlWriter.writeEndElement();
}
