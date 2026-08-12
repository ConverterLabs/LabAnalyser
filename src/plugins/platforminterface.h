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

#ifndef PLATFORMINTERFACE
#define PLATFORMINTERFACE

#include <QString>
#include <QObject>
#include "InterfaceDataType.h"
#undef GetObject  // undefine this windows macro

class Platform_Interface;

class Platform_Fabric
{
public:
    virtual ~Platform_Fabric() {}
    virtual Platform_Interface* GetInterface(QObject* messenger) = 0;
};

class Platform_Interface
{
public:
    virtual ~Platform_Interface() {}
    virtual InterfaceData* GetSymbol(const QString &ID) = 0;
    virtual QObject* GetObject() = 0;

public slots:
    virtual void MessageReceiver(const QString &Command, const QString &ID, InterfaceData Data) = 0;

signals:
    virtual void MessageSender(const QString &Command, const QString &ID, InterfaceData Data) = 0;
};

#define EchoInterface_iid "org.qt-project.Qt.Examples.EchoInterface"
//#define EchoInterface_iid2 "org.qt-project.Qt.Examples.EchoInterface2"


Q_DECLARE_INTERFACE(Platform_Fabric, EchoInterface_iid)
//Q_DECLARE_INTERFACE(Platform_Fabric, EchoInterface_iid2)


#endif // PLATFORMINTERFACE

