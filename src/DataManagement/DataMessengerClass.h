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
#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>
#include "../plugins/platforminterface.h"
#include "DataManagementClass.h"

class MessengerClass : public QObject
{
    Q_OBJECT
public:
    explicit MessengerClass(QObject *parent = nullptr, QStatusBar* SB= nullptr);

public slots:

    void MessageReceiver(const QString &Command, const QString &ID, InterfaceData Data);    //Extra Klasse
    void MessageTransmitter(const QString &Command, const QString &ID, InterfaceData Data); //Extra Klasse
    void SendInfo(QString text);
    void SendError(QString text);
    void NewDeviceRegistration(QObject*);
    void WriteStatusMessage(QString);

signals:
    void MessageSender(const QString &Command, const QString &ID, InterfaceData Data);          //Extra Klasse
    void AddElementToWidget(QString ID, InterfaceData Data);
    void ErrorWriter(const QString &ID, const QString Data);
    void InfoWriter(const QString &ID, QString Data);
    void NotificationWriter(const QString &ID, QString Data);
    void NewDataReceived(const QString &ID);
    void AddContainerElement(QString ID,QString DataType, QString Type,QString StateDependency );
    void SetData(const QString &ID, InterfaceData Data);
    void CloseProject();
    void PublishFinished();
    void PublishStart();


private:
    QStatusBar* MessageStatusbar;
};

#endif // MESSAGEHANDLER_H
