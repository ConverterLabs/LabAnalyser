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


#include "loadplugin.h"
#include <QDebug>
#include"../DataManagement/DataManagementSetClass.h"
#include <QPluginLoader>
#undef GetObject

LoadPlugin::LoadPlugin(QObject *parent, MessengerClass* Messenger_): QObject(parent), Messenger(Messenger_)
{

}

bool LoadPlugin::read(QIODevice *device, QString devFileName)
{
    m_devFileName = devFileName;

    reader.setDevice(device);
    if (reader.readNextStartElement()) {
        if (reader.name() == "LEDevice")
            readDevice();
        else
            reader.raiseError(QObject::tr("Not a LEDevice file"));
    }
    return !reader.error();
}

void LoadPlugin::readDevice()
{
    if( reader.attributes().hasAttribute("DevicePlugin"))
    {
        if( reader.attributes().hasAttribute("DeviceName"))
        {
            QString Name =  reader.attributes().value("DeviceName").toString();
            DataManagementSetClass* DCObj = dynamic_cast<DataManagementSetClass*>(this->parent());
            if(DCObj)
            {
                if(!DCObj->GetDevice(Name))
                {
                    QString filename =  reader.attributes().value("DevicePlugin").toString();
                    QPluginLoader loader(filename);
                    QObject *Plugin = loader.instance();
                    if(Plugin)
                    {
                        Platform_Fabric *NewDevice = qobject_cast<Platform_Fabric *>(Plugin);
                        Platform_Interface* PI = NewDevice->GetInterface(Messenger);
                        DCObj->AddDevice(Name, m_devFileName, PI);
                        NewDeviceReg = DCObj->GetDevice(Name);
                        NewDeviceReg->GetObject()->setObjectName(Name);
                    }
                    else
                    {
                        Messenger->SendError("the xml-Device: " + filename + " couldn't be loaded! Reason: " + loader.errorString());
                    }
                }
            }
        }
        else
             Messenger->SendError("Error Reading plugin file, DeviceName tag missing.") ;
    }
    else
        Messenger->SendError("Error Reading plugin file, DevicePlugin tag missing.") ;

}
