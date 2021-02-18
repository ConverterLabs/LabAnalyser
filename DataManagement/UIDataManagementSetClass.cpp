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

#include "UIDataManagementSetClass.h"
#include <QFile>
#include <QDir>
#include <QXmlStreamWriter>
#include <QPluginLoader>
#include "../DropWidgets/DropWidgets.h"
#include "../UIFunctions/SubPlotMainWindow.h"
#include "../DropWidgets/Plots/PlotWidget.h"
#include "../mainwindow.h"
#include "../ui_mainwindow.h"
#include "../plugins/platforminterface.h"
#include "../LoadSave/xmlexperimentreader.h"
#include "../LoadSave/xmlexperimentwriter.h"

#include "../Import/parameterloader.h"
#include "../Export/Export2Mat.h"
#include "../LoadSave/loadplugin.h"
#include "../Export/exportinputs2xml.h"
#include "../LoadSave/loadplugin.h"



#undef GetObject //Bug

UIDataManagementSetClass::UIDataManagementSetClass(QObject *parent) : DataManagementSetClass(parent)
{
    MainWindow* MW = qobject_cast<MainWindow*>(this->parent());
    Q_ASSERT(MW->objectName() == "LabAnalyser");

    connect(this->GetMessenger(), SIGNAL(CloseProject()),MW, SLOT(CloseProject()));
    connect(this->GetMessenger(), SIGNAL(PublishFinished()), MW, SLOT(PublishFinished()));
    connect(this->GetMessenger(), SIGNAL(PublishStart()),MW, SLOT(PublishStart()));
    connect(this->GetMessenger(), SIGNAL(ErrorWriter(QString,QString)),MW,SLOT(ErrorWriter(QString,QString)));
    connect(this->GetMessenger(),SIGNAL(InfoWriter(QString,QString)),MW,SLOT(InfoWriter(QString,QString)));
    connect(this->GetMessenger(),SIGNAL(NotificationWriter(QString,QString)),MW,SLOT(NotificationWriter(QString,QString)));
    connect(this->GetMessenger(),SIGNAL(AddElementToWidget(QString, InterfaceData)),MW,SLOT(AddElementToWidget(QString, InterfaceData)));


    connect(this, SIGNAL(LoadFormFromXML(QString,QString,bool)), MW,SLOT(LoadFormFromXML(QString,QString,bool)));

}

bool UIDataManagementSetClass::SaveExperiment(QString Path)
{

    bool Error = false;
    //create a backup
    QFile file(Path);
    QDir T(QFileInfo(file).absoluteDir());
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "Cannot open file" << file.errorString();
        return true;
    }
    xmlexperimentwriter Writer(this, GetMessenger());
    if (!Writer.write(&file, T))
    {
        Info("Error saving File ");
        Error = true;
    }

    file.close();
    //Signal Plugins to save there properties as well
    QList<QString> Devices = this->GetDevices();
    for(int i = 0; i < Devices.size(); i++)
    {
        QString Name = (((this->GetDevice(Devices[i])))->GetObject()->objectName() + "::" + Path);
        this->GetDevice(Devices[i])->MessageReceiver("save" ,Name,InterfaceData());
    }
    if(!Error)
        GetMessenger()->WriteStatusMessage(QString("Experiment saved to '%1'.").arg(Path));

    return Error;
}


bool UIDataManagementSetClass::LoadExperiment(QString Path)
{
     bool Error = false;
     LoadPath = Path;
     QFile file(LoadPath);
     Info("Loading Experiment: " + Path);
     if(!file.open(QFile::ReadOnly | QFile::Text)){
         qDebug() << "Cannot read file" << file.errorString();
         return true;
     }
     XmlExperimentReader Reader(this, this->GetMessenger());
     if (!Reader.read(&file))
     {
         Info("Parse error in file " + Reader.errorString());
         Error = true;
     }
     return Error;
}


UIDataManagementSetClass::~UIDataManagementSetClass()
{

}

bool UIDataManagementSetClass::Export2Xml(QString Path, QStringList ExportIds)
{
        auto Exporter = ExportInputs2Xml(this);
        auto Error = Exporter.Export2XML(Path, ExportIds);
        if(!Error)
            GetMessenger()->WriteStatusMessage("Exported Parameters to " + Path );

        return Error;

}

bool UIDataManagementSetClass::Export2Mat(QString Path , QStringList ExportIds )
{
    auto Exporter = MatExporter(this);
    auto Error = Exporter.Export2Mat(Path, ExportIds);
    if(!Error)
           GetMessenger()->WriteStatusMessage(QString("Data exported to '%1'.").arg(Path));
    return Error;
}

bool UIDataManagementSetClass::LoadPlugin(QString FileName)
{

    class LoadPlugin PluginLoader(this, this->GetMessenger());
    QFile file(FileName);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        GetMessenger()->SendInfo("Cannot read file " + FileName);
        return true;
    }
    if (!PluginLoader.read(&file, FileName))
        Info("Parse error in file " + FileName + ". Reason: " + PluginLoader.errorString());
    else
    {
        if(PluginLoader.GetNewDevice())
        {
            #undef GetObject
            GetMessenger()->NewDeviceRegistration(PluginLoader.GetNewDevice()->GetObject());
            GetMessenger()->MessageTransmitter("load" ,PluginLoader.GetNewDevice()->GetObject()->objectName() + "::" + FileName,InterfaceData());
            GetMessenger()->MessageTransmitter("LoadCustomData" ,PluginLoader.GetNewDevice()->GetObject()->objectName() +  "::" + FileName,InterfaceData());
        }
    }
}

bool UIDataManagementSetClass::ImportFromXml(QString Path  )
{
    ParameterLoader Loader(this);
    auto Error = Loader.Load(Path);
    if(!Error)
        GetMessenger()->WriteStatusMessage(Path + " loaded");
    return Error;
}
