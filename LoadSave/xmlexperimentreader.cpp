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


#include "xmlexperimentreader.h"
#include <QDebug>
#include <QByteArray>
#include "loadplugin.h"
#include "../mainwindow.h"
#include "../ui_mainwindow.h"
#include "../DropWidgets/Plots/PlotWidget.h"
#undef GetObject

XmlExperimentReader::XmlExperimentReader(QObject *parent, MessengerClass* Messenger_, UIDataManagementSetClass* Manage): QObject(parent)
{
 Manager = Manage;
 Messenger = Messenger_;
 connect(this, SIGNAL(LoadFormFromXML(QString,QString,bool)), this->parent()->parent(),SLOT(LoadFormFromXML(QString,QString,bool)));

}

bool XmlExperimentReader::read(QString LoadPath_)
{
    LoadPath = LoadPath_;
    QFile file(LoadPath);
    Messenger->SendInfo("Loading Experiment: " + LoadPath);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Cannot read file" << file.errorString();
        return true;
    }

    reader.setDevice(&file);

    if (reader.readNextStartElement()) {
        if (reader.name() ==  QString("Experiment"))
            readExperiment();
        else
            reader.raiseError(QObject::tr("Not a Experiment file"));
    }
    file.close();

    return reader.error();
}

void XmlExperimentReader::readExperiment()
{
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("Tabs"))
           readTab();
       else if(reader.name() ==  QString("Devices"))
           readDevices();
       else if(reader.name() ==  QString("Widgets"))
           readWidgets();
       else if(reader.name() ==  QString("State"))
       {
           try
           {
               ReadState();
           }
           catch (...)
           {

           }
       }
       else if(reader.name() ==  QString("FigureWindows"))
       {
           try
           {
               CreateFigureWindows();
           }
           catch (...)
           {

           }
       }

       else if(reader.name() ==  QString("Connections"))
       {
           try
           {
               CreateConnections();
           }
           catch (...)
           {

           }
       }
       else
           reader.skipCurrentElement();
   }
}



void XmlExperimentReader::CreateConnection()
{
    DataManagementSetClass* DCObj = dynamic_cast<DataManagementSetClass*>(this->parent());
    MainWindow* MW = qobject_cast<MainWindow*>(this->parent()->parent());
    Q_ASSERT(MW->objectName() ==  QString("LabAnalyser"));
    QString ID;
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("ID"))
       {
           double Min = reader.attributes().value("Min").toDouble();
           double Max = reader.attributes().value("Max").toDouble();
           QString Alias = reader.attributes().value("Alias").toString();
           InterfaceData _DataBackup;

           if(!(reader.attributes().value("Type").isEmpty()) && !(reader.attributes().value("DataType").isEmpty()))
           {
               _DataBackup.SetDataType(reader.attributes().value("DataType").toString());
               _DataBackup.SetType(reader.attributes().value("Type").toString());
           }
           ID = reader.readElementText().trimmed();
           //Because of threading the ID might not be there
           if(!DCObj->ElementExists(ID))
           {
                   QString DeviceName = ID.split("::").at(0);
                   Platform_Interface* CD = DCObj->GetDevice(DeviceName);
                   InterfaceData* IDS = nullptr;
                   if(CD)
                        IDS = CD->GetSymbol(ID);
                   if(IDS)
                   {
                       if(IDS->GetDataType().size())
                       {
                           DCObj->MessageSender("publish",ID,*IDS);
                       }
                       else
                       {
                           if(!(_DataBackup.GetDataType().isEmpty()))
                              DCObj->MessageSender("publish",ID,_DataBackup);
                       }
                   }else
                   {
                       if(!(_DataBackup.GetDataType().isEmpty()))
                            DCObj->MessageSender("publish",ID,_DataBackup);
                   }

           }
           if((DCObj->ElementExists(ID)))
           {
               DCObj->SetMinMaxValue(ID, Min, Max);
               if(Alias.size())
                   DCObj->SetAlias(ID,Alias);
           }
           else
               qDebug() << ID;
       }
       else if(reader.name() ==  QString("ObjectName"))
        {
            QString ObjectName(reader.readElementText());
            QObject* FoundObject = MW->findChild<QObject*>(ObjectName);
            if(FoundObject && DCObj->ElementExists(ID))
            {
                DCObj->AddElementToContainerEntry(FoundObject->objectName(),ID,FoundObject->metaObject()->className(),FoundObject);
                VariantDropWidget* VObj = dynamic_cast<VariantDropWidget*>(FoundObject);
                if(VObj)
                {
                    VObj->ConnectToID(DCObj, ID);
                }
                //Magic Heres
            }

        }
       else
           reader.skipCurrentElement();
   }


}

void XmlExperimentReader::CreateConnections()
{
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("connect"))
           CreateConnection();
       else
           reader.skipCurrentElement();
   }
}


void XmlExperimentReader::CreateFigureWindows()
{
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("Window"))
           CreateFigureWindow();
       else
           reader.skipCurrentElement();
   }
}

void XmlExperimentReader::CreateFigureWindow()
{
       int rows = 0;
       int cols = 0;
       QPoint Pos;
       QSize Size;
       rows = reader.attributes().value("Rows").toInt();
       cols = reader.attributes().value("Cols").toInt();
       Pos.setX(reader.attributes().value("PosX").toInt());
       Pos.setY(reader.attributes().value("PosY").toInt());
       Size.setWidth(reader.attributes().value("Width").toInt());
       Size.setHeight(reader.attributes().value("Height").toInt());

       //Create Plos
       MainWindow* MW = qobject_cast<MainWindow*>(this->parent()->parent());
       SubPlotMainWindow* Sub = MW->CreateSubPlotWindow(rows,cols);
       Sub->resize(Size);
       Sub->move(Pos);

       QList<PlotWidget*> PlotWidgetFound = (Sub->findChildren<PlotWidget*>());
       DataManagementSetClass* DCObj = dynamic_cast<DataManagementSetClass*>(this->parent());
       int itt_counter = 0;
       while(reader.readNextStartElement())
       {
           if(reader.name() ==  QString("PlotWidgetName"))
            {
                auto NewName = reader.readElementText();
                DCObj->RenamePlotPointer(PlotWidgetFound[itt_counter]->objectName(), NewName);
                PlotWidgetFound[itt_counter]->setObjectName(NewName);
                itt_counter++;
            }
            else
              reader.skipCurrentElement();
      }

}

void XmlExperimentReader::ReadState()
{
    QByteArray State;
    State.append(reader.readElementText().toUtf8());
    MainWindow* MW = qobject_cast<MainWindow*>(this->parent()->parent());
    Q_ASSERT(MW->objectName() ==  QString("LabAnalyser"));

    MW->restoreState((QByteArray::fromBase64(State)));
    //Compensate Bug, redock all docked windows
    int i = 0;
    QList<QDockWidget *> dockWidgets =MW->findChildren<QDockWidget *>();
    for(auto itt: dockWidgets)
    {
        if(MW->dockWidgetArea(itt) == Qt::LeftDockWidgetArea)
        {
            if(itt->objectName().compare("ParameterDock") &&
                    itt->objectName().compare("StateDock") &&
                    itt->objectName().compare("DataDock")&&
                    itt->objectName().compare("OutputDock"))
                if(!(itt->isFloating()))
                    i++;
        }
    }
    if(!i)
         MW->UI()->centralWidget->show();
    else
         MW->UI()->centralWidget->hide();

}

void XmlExperimentReader::readTab()
{
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("Form"))
           LoadForm(reader.attributes().value("Name").toString());
       else
           reader.skipCurrentElement();
   }
}


void XmlExperimentReader::LoadForm(QString Name)
{
    QString Filename;
 QString FilenameT;
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("AbsPath"))
       {
           FilenameT = reader.readElementText();
           QFile file(FilenameT);
           if(file.exists())
           {
               Filename = QFileInfo(file).absoluteFilePath();
           }
       }
       else if(reader.name() ==  QString("RelPath"))
       {
           FilenameT = QFileInfo(LoadPath).absolutePath() + "/" + reader.readElementText();
           QFile file(FilenameT);
           if(file.exists())
           {
               Filename = QFileInfo(file).absoluteFilePath();
           }
       }
       else
           reader.skipCurrentElement();
   }
    if(Filename.size())
        emit LoadFormFromXML(Filename, Name, true);
    else
        Messenger->ErrorWriter(this->parent()->objectName(), "Form File " + FilenameT + " not found!");


}


void XmlExperimentReader::readDevices()
{
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("Device"))
           LoadDevice();
       else
           reader.skipCurrentElement();
   }
}

void XmlExperimentReader::LoadDevice()
{
    QString Filename;
    QString FilenameT;
    while(reader.readNextStartElement()){
       if(reader.name() ==  QString("AbsPath"))
       {
           FilenameT = reader.readElementText();
           QFile file(FilenameT);
           if(file.exists())
           {
               Filename = QFileInfo(file).absoluteFilePath();
           }
       }
       else if(reader.name() ==  QString("RelPath"))
       {
           QString FilenameT = QFileInfo(LoadPath).absolutePath() + "/" + reader.readElementText();
           QFile file(FilenameT);
           if(file.exists())
           {
               Filename = QFileInfo(file).absoluteFilePath();
           }
       }
       else
           reader.skipCurrentElement();
   }

    if(Filename.size())
    {

        if(Manager->LoadPlugin(Filename))
           reader.raiseError("Plugin couldn't be loaded.");
    }

        /*LoadPlugin PluginLoader(this->parent(), Messenger);
        QFile file(Filename);
        if(!file.open(QFile::ReadOnly | QFile::Text)){
            Messenger->ErrorWriter(this->parent()->objectName(),"Cannot read file: " + Filename + " Reason: " + file.errorString());
            return;
        }
        if (!PluginLoader.read(&file, Filename))
                Messenger->ErrorWriter(this->parent()->objectName(),"Parse error reading file: " + Filename + " Reason: " + PluginLoader.errorString());
        else
        {
            if(PluginLoader.GetNewDevice())
            {
                Messenger->NewDeviceRegistration(PluginLoader.GetNewDevice()->GetObject());
                InterfaceData Data;
                Data.SetData(Filename);
                Messenger->MessageTransmitter("load" ,PluginLoader.GetNewDevice()->GetObject()->objectName(),Data);                
                Data.SetData(LoadPath);
                Messenger->MessageTransmitter("LoadCustomData" ,PluginLoader.GetNewDevice()->GetObject()->objectName(),Data);
            }
        }
    }
    else
    {
        Messenger->ErrorWriter(this->parent()->objectName(), "Device File " + FilenameT + " not found!");
    }*/

}


void XmlExperimentReader::readWidgets()
{
    while(reader.readNextStartElement()){
       if(reader.name() == QString("Widget"))
           readWidget();
       else
           reader.skipCurrentElement();
   }
}

void XmlExperimentReader::readWidget()
{
    MainWindow* MW = qobject_cast<MainWindow*>(this->parent()->parent());
    Q_ASSERT(MW->objectName() ==  QString("LabAnalyser"));

    std::vector<std::pair<QString, QString>> Attributes;
    QString Text;
    QString Name;
    for(int k = 0; k <  reader.attributes().size(); k++)
    {
        std::pair<QString, QString> Attribut;
        Attribut.first = reader.attributes().at(k).name().toString();
        Attribut.second = reader.attributes().at(k).value().toString();
        if(Attribut.first ==  QString("Name"))
            Name = Attribut.second;
        Attributes.push_back(Attribut);
    }
    Text = reader.readElementText().trimmed();
    QObject* FoundObject = MW->findChild<QObject*>(Name);
    VariantDropWidget* VObj = dynamic_cast<VariantDropWidget*>(FoundObject);
    if(VObj)
    {
        if(!VObj->LoadFromXML(Attributes, Text))
        {
            qDebug() << "LoadFromXML function of " << FoundObject->objectName() << " not implemented.";
        }
    }
}



