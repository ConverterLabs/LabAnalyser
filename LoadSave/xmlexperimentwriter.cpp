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

#include "xmlexperimentwriter.h"
#include "../UIFunctions/SubPlotMainWindow.h"
#include "../DropWidgets/Plots/PlotWidget.h"
#include "../mainwindow.h"
#undef GetObject //Bug
#include <QFile>

xmlexperimentwriter::xmlexperimentwriter(QObject *parent, MessengerClass& Messenger_, DataManagementClass& Manager_): QObject(parent), m_Messenger(Messenger_), m_Manager(Manager_)
{


}

void xmlexperimentwriter::writeTabs()
{
    m_xmlWriter.writeStartElement("Tabs");
    for(int i = 0; i < m_Manager.GetFormFileCount();i++)
     {
        m_xmlWriter.writeStartElement("Form");
        m_xmlWriter.writeAttribute("Name",m_Manager.GetFormFileEntry(i).first);
        m_xmlWriter.writeTextElement("AbsPath", m_Manager.GetFormFileEntry(i).second);
        m_xmlWriter.writeTextElement("RelPath", m_T.relativeFilePath(m_Manager.GetFormFileEntry(i).second) );
        m_xmlWriter.writeEndElement();
     }
    m_xmlWriter.writeEndElement();
}

void xmlexperimentwriter::writeExperiment()
{
    m_xmlWriter.writeStartElement("Experiment");
        writeTabs();
        writeDevices();
        writeFigureWindows();
        writeWidgets();
        writeConnections();
        writeState();
    m_xmlWriter.writeEndElement();
}

void xmlexperimentwriter::writeDevices()
{
    //Save all loaded plugins
    m_xmlWriter.writeStartElement("Devices");
    QList<QString> Liste = m_Manager.GetDevicePaths();
    for(int i = 0; i < Liste.size();i++)
     {
        m_xmlWriter.writeStartElement("Device");
        m_xmlWriter.writeTextElement("AbsPath", Liste[i] );
        m_xmlWriter.writeTextElement("RelPath", m_T.relativeFilePath(Liste[i]) );
        m_xmlWriter.writeEndElement();
     }
    m_xmlWriter.writeEndElement();
}

void xmlexperimentwriter::writeFigureWindows()
{
    Q_ASSERT(this->parent()->parent()->objectName() ==  QString("LabAnalyser"));

    m_xmlWriter.writeStartElement("FigureWindows");
    QList<SubPlotMainWindow*> cti = (this->parent()->parent()->findChildren<SubPlotMainWindow*>());

    for(int i = 0; i < cti.size();i++)
     {
        QString PlotType("PlotWidget");

        QWidget* Window = cti[i];
         m_xmlWriter.writeStartElement("Window");
         m_xmlWriter.writeAttribute("Rows", QString::number(m_Manager.GetPlotWindowRowsCols(cti[i]->objectName()).first));
         m_xmlWriter.writeAttribute("Cols", QString::number(m_Manager.GetPlotWindowRowsCols(cti[i]->objectName()).second));
         m_xmlWriter.writeAttribute("PosX", QString::number(Window->pos().x()));
         m_xmlWriter.writeAttribute("PosY", QString::number(Window->pos().y()));
         m_xmlWriter.writeAttribute("Width", QString::number(Window->size().width()));
         m_xmlWriter.writeAttribute("Height", QString::number(Window->size().height()));
         QList<PlotWidget*> PlotWidgetFound = (Window->findChildren<PlotWidget*>());

        for(int g = 0; g < PlotWidgetFound.size(); g++)
        {
            m_xmlWriter.writeStartElement("PlotWidgetName");
            m_xmlWriter.writeCharacters(PlotWidgetFound[g]->objectName());
            m_xmlWriter.writeEndElement();
        }
        m_xmlWriter.writeEndElement(); //END Window
     }
    m_xmlWriter.writeEndElement(); //END  FigureWindows

}

void xmlexperimentwriter::writeWidgets()
{
    Q_ASSERT(this->parent()->parent()->objectName() ==  QString("LabAnalyser"));

    m_xmlWriter.writeStartElement("Widgets");
    //Find all Widgets
    auto widgets = this->parent()->parent()->findChildren<QWidget*>();
    for(auto itt : widgets)
    {
       // qDebug() << itt->objectName();
        //Check if Widget has Save Function
        VariantDropWidget* DCObj = dynamic_cast<VariantDropWidget*>(itt);
        if(DCObj)
        {
            //qDebug() << itt->objectName();
            QString tmp;
            std::vector<std::pair<QString, QString>> Attributes;
            QString Text;
            if(DCObj->SaveToXML(Attributes, Text))
            {
                //qDebug() << "DCObj->SaveToXML(Attributes, Text)";
                m_xmlWriter.writeStartElement("Widget");
                m_xmlWriter.writeAttribute("Name",itt->objectName());
                for(auto aitt : Attributes)
                    m_xmlWriter.writeAttribute(aitt.first,aitt.second);
                if(Text.size())
                     m_xmlWriter.writeCharacters(Text);
                m_xmlWriter.writeEndElement();
            }
        }
    }
    m_xmlWriter.writeEndElement();
}

void xmlexperimentwriter::writeConnections()
{
    m_xmlWriter.writeStartElement("Connections");
    for(int i = 0; i <  m_Manager.GetContainerCount(); i++)
    {
      std::pair<QString, std::vector<QString>> ContainerElement = m_Manager.GetContainerElementForms(i);
      if(ContainerElement.second.size())
      {
        m_xmlWriter.writeStartElement("connect");
        m_xmlWriter.writeStartElement("ID");
        m_xmlWriter.writeAttribute("Min",QString::number(m_Manager.MinMaxValue(ContainerElement.first).first));
        m_xmlWriter.writeAttribute("Max",QString::number(m_Manager.MinMaxValue(ContainerElement.first).second));
        m_xmlWriter.writeAttribute("Alias",(m_Manager.GetAlias(ContainerElement.first)));

        ToFormMapper* cont = m_Manager.GetContainer(ContainerElement.first);
        if(cont)
        {
            m_xmlWriter.writeAttribute("Type", cont->GetType());
            m_xmlWriter.writeAttribute("DataType", cont->GetDataType());
        }

        m_xmlWriter.writeCharacters(ContainerElement.first);
        m_xmlWriter.writeEndElement();

        for(int j = 0;  j < ContainerElement.second.size(); j++)
        {
            m_xmlWriter.writeTextElement("ObjectName",ContainerElement.second[j]);
        }
        m_xmlWriter.writeEndElement();
      }
    }
    m_xmlWriter.writeEndElement(); //END Connections

}

void xmlexperimentwriter::writeState()
{
    Q_ASSERT(this->parent()->parent()->objectName() ==  QString("LabAnalyser"));

    m_xmlWriter.writeStartElement("State");
    QByteArray BA =  qobject_cast<MainWindow*>(this->parent()->parent())->saveState();
    m_xmlWriter.writeCharacters(BA.toBase64());
    m_xmlWriter.writeEndElement();

}

bool xmlexperimentwriter::write(QString LoadPath_)
{
    //QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QFile file(LoadPath_);

    if(!(file.open(QIODevice::WriteOnly| QIODevice::Text))){
        qDebug() << "Cannot open file" << file.errorString();
        return true;
    }
    m_T = (QFileInfo(file).absoluteDir());

    m_xmlWriter.setDevice(&file);
    m_xmlWriter.setAutoFormatting(true);
    m_xmlWriter.writeStartDocument();
     writeExperiment();
    m_xmlWriter.writeEndDocument();

    file.close();

    return false;

}
