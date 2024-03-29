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

#include "../Export/export2highfive.h"



#undef GetObject //Bug

UIDataManagementSetClass::UIDataManagementSetClass(QObject *parent) : DataManagementSetClass(parent)
{
    MainWindow* MW = qobject_cast<MainWindow*>(this->parent());
    Q_ASSERT(MW->objectName() ==  QString("LabAnalyser"));

    connect(parent, SIGNAL(SaveExperiment(QString)),this, SLOT(SaveExperiment(QString)));

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
    xmlexperimentwriter Writer(this, GetMessengerRef(), *this);

    if ((Writer.write(Path)))
    {
        Info("Error saving File ");
        Error = true;
    }

    //Signal Plugins to save there properties as well
    QList<QString> Devices = this->GetDevices();
    for(int i = 0; i < Devices.size(); i++)
    {
        QString Name = ((this->GetDevice(Devices[i])))->GetObject()->objectName() ;
        InterfaceData tmp;
        tmp.SetData(Path);
        MessageSender("save" ,Name,tmp);
    }
    if(!Error)
        GetMessenger()->WriteStatusMessage(QString("Experiment saved to '%1'.").arg(Path));

    return Error;
}


bool UIDataManagementSetClass::LoadExperiment(QString Path)
{
     bool Error = false;
     LoadPath = Path;

     XmlExperimentReader Reader(this, this->GetMessenger(), this);
     if (Reader.read(LoadPath))
     {
         Info("Parse error in file " + LoadPath);
         Error = true;
         GetMessenger()->MessageReceiver("CloseProject", "LoadExperiment", InterfaceData());
     }
     return Error;
}


UIDataManagementSetClass::~UIDataManagementSetClass()
{

}

bool UIDataManagementSetClass::Export2Xml(QString Path, QStringList ExportIds)
{

        ExportInputs2Xml Exporter(*this);
        auto Error = Exporter.Export2XML(Path, ExportIds);
        if(!Error)
            GetMessenger()->WriteStatusMessage("Exported Parameters to " + Path );
        return Error;

}

bool UIDataManagementSetClass::Export2Mat(QString Path , QStringList ExportIds )
{
    try {
        auto Exporter = MatExporter(this);
        auto Error = Exporter.Export2Mat(Path, ExportIds);
        if(!Error)
               GetMessenger()->WriteStatusMessage(QString("Data exported to '%1'.").arg(Path));
        return Error;
    } catch (const std::exception& e) {
        Error("Export 2mat failed -> " + QString::fromStdString(e.what()));
    }
    return 1;
}

bool UIDataManagementSetClass::Export2Hdf5(QString Path , QStringList ExportIds )
{
    auto Exporter = Export2HDF5(this);
    try
    {
    auto Error = Exporter.Export(Path, ExportIds);
    if(!Error)
           GetMessenger()->WriteStatusMessage(QString("Data exported to '%1'.").arg(Path));
         return Error;
    }
    catch (...)
    {
        Error("Export failed - please mind the HDF5 library does not support special characters in the path. Only UTF8!");
    }

    return 0;
}

bool UIDataManagementSetClass::LoadPlugin(QString FileName)
{

    class LoadPlugin PluginLoader(this, this->GetMessenger());
    QFile file(FileName);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        Error("Cannot read file " + FileName);
        return true;
    }
    if (PluginLoader.read(&file, FileName))
    {
        Error("Parse error in file " + FileName + ". Reason: " + PluginLoader.errorString());
        return true;
    }
    else
    {
        if(PluginLoader.GetNewDevice())
        {
            #undef GetObject
            GetMessenger()->NewDeviceRegistration(PluginLoader.GetNewDevice()->GetObject());
            InterfaceData Data;
            Data.SetData(FileName);
            GetMessenger()->MessageTransmitter("load" ,PluginLoader.GetNewDevice()->GetObject()->objectName(),Data);
            Data.SetData(LoadPath);
            GetMessenger()->MessageTransmitter("LoadCustomData" ,PluginLoader.GetNewDevice()->GetObject()->objectName(),Data);

        }
        else
            return true;

    }
    return false;
}

bool UIDataManagementSetClass::ImportFromXml(QString Path  )
{
    ParameterLoader Loader(this);
    auto Error = Loader.Load(Path);
    if(!Error)
        GetMessenger()->WriteStatusMessage(Path + " loaded");
    return Error;
}
