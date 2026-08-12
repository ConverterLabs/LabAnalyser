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

#pragma once
#include <QObject>
#include <QFile>
#include <QXmlStreamReader>
#include "plugins/platforminterface.h"
#include "../DataManagement/DataMessengerClass.h"

class LoadPlugin: public QObject
{
   Q_OBJECT
public:
    LoadPlugin(QObject *parent, MessengerClass* Messenger_);
    bool read(QIODevice *device, QString devFileName);
    QString errorString() { return QString();};
    Platform_Interface* GetNewDevice(){return NewDeviceReg;};

private:
    QXmlStreamReader reader;
    void readDevice();
    Platform_Interface *NewDeviceReg = nullptr;
    QString m_devFileName;
    MessengerClass* Messenger;


};

