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

#include <QDir>
#include <QFile>

ExportInputs2Xml::ExportInputs2Xml(DataManagementSetClass& Manager_): m_Manager(Manager_)
{

}
bool ExportInputs2Xml::Export2XML(QString Filename_,  QStringList Ids_)
{
    ExportCounter = 0;
    Filename = Filename_;
    Ids = Ids_;

    //create a backup
    QFile file(Filename_);

    if(!(file.open(QIODevice::WriteOnly| QIODevice::Text))){
        qDebug() << "Cannot open file" << file.errorString();
        return true;
    }
    m_xmlWriter.setDevice(&file);

    m_xmlWriter.setAutoFormatting(true);
    m_xmlWriter.writeStartDocument();
    WriteParameterSet();
    m_xmlWriter.writeEndDocument();

    file.close();

    return false;
}

void ExportInputs2Xml::WriteParameterSet()
{

    m_xmlWriter.writeStartElement("ParameterSet");
    for(ExportCounter = 0; ExportCounter < Ids.size(); ExportCounter++)
    {
     if(m_Manager.GetContainer(Ids[ExportCounter]))
         WriteEntry();
    }
    m_xmlWriter.writeEndElement();

}

void ExportInputs2Xml::WriteEntry()
{
    m_xmlWriter.writeStartElement("entry");
        WriteParameter();
        WriteValue();
    m_xmlWriter.writeEndElement();
}

void ExportInputs2Xml::WriteParameter()
{
    m_xmlWriter.writeStartElement("Parameter");
         m_xmlWriter.writeCharacters(Ids[ExportCounter]);
    m_xmlWriter.writeEndElement();
}

void ExportInputs2Xml::WriteValue()
{
    m_xmlWriter.writeStartElement("Value");
         m_xmlWriter.writeCharacters(m_Manager.GetContainer(Ids[ExportCounter])->GetString());
    m_xmlWriter.writeEndElement();
}
