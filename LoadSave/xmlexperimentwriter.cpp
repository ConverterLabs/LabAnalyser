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

xmlexperimentwriter::xmlexperimentwriter(QObject *parent, MessengerClass* Messenger_): QObject(parent)
{
    Messenger = Messenger_;
    Manager = dynamic_cast<DataManagementClass*>(parent);
    if(!Manager)
        throw;

}

void xmlexperimentwriter::writeTabs()
{
    xmlWriter.writeStartElement("Tabs");
    for(int i = 0; i < Manager->GetFormFileCount();i++)
     {
        xmlWriter.writeStartElement("Form");
        xmlWriter.writeAttribute("Name",Manager->GetFormFileEntry(i).first);
        xmlWriter.writeTextElement("AbsPath", Manager->GetFormFileEntry(i).second);
        xmlWriter.writeTextElement("RelPath", T.relativeFilePath(Manager->GetFormFileEntry(i).second) );
        xmlWriter.writeEndElement();
     }
    xmlWriter.writeEndElement();
}

void xmlexperimentwriter::writeExperiment()
{
    xmlWriter.writeStartElement("Experiment");
        writeTabs();
        writeDevices();
        writeFigureWindows();
        writeWidgets();
        writeConnections();
        writeState();
    xmlWriter.writeEndElement();
}

void xmlexperimentwriter::writeDevices()
{
    //Save all loaded plugins
    xmlWriter.writeStartElement("Devices");
    QList<QString> Liste = Manager->GetDevicePaths();
    for(int i = 0; i < Liste.size();i++)
     {
        xmlWriter.writeStartElement("Device");
        xmlWriter.writeTextElement("AbsPath", Liste[i] );
        xmlWriter.writeTextElement("RelPath", T.relativeFilePath(Liste[i]) );
        xmlWriter.writeEndElement();
     }
    xmlWriter.writeEndElement();
}

void xmlexperimentwriter::writeFigureWindows()
{
    Q_ASSERT(this->parent()->parent()->objectName() == "LabAnalyser");

    xmlWriter.writeStartElement("FigureWindows");
    QList<SubPlotMainWindow*> cti = (this->parent()->parent()->findChildren<SubPlotMainWindow*>());

    for(int i = 0; i < cti.size();i++)
     {
        QString PlotType("PlotWidget");

        QWidget* Window = cti[i];
         xmlWriter.writeStartElement("Window");
         xmlWriter.writeAttribute("Rows", QString::number(Manager->GetPlotWindowRowsCols(cti[i]->objectName()).first));
         xmlWriter.writeAttribute("Cols", QString::number(Manager->GetPlotWindowRowsCols(cti[i]->objectName()).second));
         xmlWriter.writeAttribute("PosX", QString::number(Window->pos().x()));
         xmlWriter.writeAttribute("PosY", QString::number(Window->pos().y()));
         xmlWriter.writeAttribute("Width", QString::number(Window->size().width()));
         xmlWriter.writeAttribute("Height", QString::number(Window->size().height()));
         QList<PlotWidget*> PlotWidgetFound = (Window->findChildren<PlotWidget*>());

        for(int g = 0; g < PlotWidgetFound.size(); g++)
        {
            xmlWriter.writeStartElement("PlotWidgetName");
            xmlWriter.writeCharacters(PlotWidgetFound[g]->objectName());
            xmlWriter.writeEndElement();
        }
        xmlWriter.writeEndElement(); //END Window
     }
    xmlWriter.writeEndElement(); //END  FigureWindows

}

void xmlexperimentwriter::writeWidgets()
{
    Q_ASSERT(this->parent()->parent()->objectName() == "LabAnalyser");

    xmlWriter.writeStartElement("Widgets");
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
                xmlWriter.writeStartElement("Widget");
                xmlWriter.writeAttribute("Name",itt->objectName());
                for(auto aitt : Attributes)
                    xmlWriter.writeAttribute(aitt.first,aitt.second);
                if(Text.size())
                     xmlWriter.writeCharacters(Text);
                xmlWriter.writeEndElement();
            }
        }
    }
    xmlWriter.writeEndElement();
}

void xmlexperimentwriter::writeConnections()
{
    xmlWriter.writeStartElement("Connections");
    for(int i = 0; i <  Manager->GetContainerCount(); i++)
    {
      std::pair<QString, std::vector<QString>> ContainerElement = Manager->GetContainerElementForms(i);
      if(ContainerElement.second.size())
      {
        xmlWriter.writeStartElement("connect");
        xmlWriter.writeStartElement("ID");
        xmlWriter.writeAttribute("Min",QString::number(Manager->MinMaxValue(ContainerElement.first).first));
        xmlWriter.writeAttribute("Max",QString::number(Manager->MinMaxValue(ContainerElement.first).second));
        xmlWriter.writeAttribute("Alias",(Manager->GetAlias(ContainerElement.first)));
        xmlWriter.writeCharacters(ContainerElement.first);
        xmlWriter.writeEndElement();

        for(int j = 0;  j < ContainerElement.second.size(); j++)
        {
            xmlWriter.writeTextElement("ObjectName",ContainerElement.second[j]);
        }
        xmlWriter.writeEndElement();
      }
    }
    xmlWriter.writeEndElement(); //END Connections

}

void xmlexperimentwriter::writeState()
{
    Q_ASSERT(this->parent()->parent()->objectName() == "LabAnalyser");

    xmlWriter.writeStartElement("State");
    QByteArray BA =  qobject_cast<MainWindow*>(this->parent()->parent())->saveState();
    xmlWriter.writeCharacters(BA.toBase64());
    xmlWriter.writeEndElement();

}

bool xmlexperimentwriter::write(QIODevice *device, QDir T_)
{
    T = T_;

    xmlWriter.setDevice(device);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    writeExperiment();

    xmlWriter.writeEndDocument();

}
