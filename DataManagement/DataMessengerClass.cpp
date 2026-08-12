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

#include "DataMessengerClass.h"
#include "MessageDispatchPolicy.h"
#include <QDebug>

MessengerClass::MessengerClass(QObject *parent, QStatusBar* SB) : QObject(parent)
{
    connect(parent,SIGNAL(Error(QString)),this,SLOT(SendError(QString)));
    connect(parent,SIGNAL(Info(QString)),this,SLOT(SendInfo(QString)));
    connect(parent,SIGNAL(MessageSender(QString,QString,InterfaceData)),this,SLOT(MessageTransmitter(QString,QString,InterfaceData)));

    connect(this, SIGNAL(AddContainerElement(QString,QString,QString,QString)), parent, SLOT(AddContainerElement(QString,QString,QString,QString)));
    connect(this, SIGNAL(SetData(QString,InterfaceData)), parent, SLOT(SetData(QString,InterfaceData)));
    connect(this, SIGNAL(NewDataReceived(QString)), parent, SLOT(SetData(QString)));


    this->MessageStatusbar = SB;
}

void MessengerClass::MessageReceiver(const QString &Command, const QString &ID, InterfaceData Data)
{
    const QVector<MessageDispatchIntent> intents = MessageDispatchPolicy::ReceiverIntents(Command);
    for (const MessageDispatchIntent intent : intents) {
        switch (intent) {
        case MessageDispatchIntent::AddContainerElement:
            emit AddContainerElement(ID,Data.GetDataType(),Data.GetType(),Data.GetStateDependency());
            break;
        case MessageDispatchIntent::SetData:
            emit SetData(ID, Data);
            break;
        case MessageDispatchIntent::AddElementToWidget:
            emit AddElementToWidget(ID,Data);
            break;
        case MessageDispatchIntent::NewDataReceived:
            emit NewDataReceived(ID);
            break;
        case MessageDispatchIntent::WriteStatusMessage:
            if(MessageStatusbar)
                MessageStatusbar->showMessage(ID + " -> " + Data.GetString(),1000);
            break;
        case MessageDispatchIntent::CloseProject:
            emit CloseProject();
            break;
        case MessageDispatchIntent::WriteError:
            emit ErrorWriter(ID, Data.GetString());
            break;
        case MessageDispatchIntent::WriteInfo:
            emit InfoWriter(ID, Data.GetString());
            break;
        case MessageDispatchIntent::WriteNotification:
            emit NotificationWriter(ID, Data.GetString());
            break;
        case MessageDispatchIntent::WriteCloseProjectNotification:
            {
                QObject* manager = this->parent();
                QObject* application = manager ? manager->parent() : nullptr;
                if (!application)
                    break;
                InterfaceData closeData;
                closeData.SetData("Closing forced by: " + ID);
                NotificationWriter(application->objectName(), closeData.GetString());
            }
            break;
        case MessageDispatchIntent::PublishFinished:
            emit PublishFinished();
            break;
        case MessageDispatchIntent::PublishStart:
            emit PublishStart();
            break;
        }
    }
}

void MessengerClass::MessageTransmitter(const QString &Command, const QString &ID, InterfaceData Data)
{
    MessageReceiver( Command,   ID,  Data);
    emit MessageSender(  Command,   ID,  Data);
}


void MessengerClass::SendInfo(QString text)
{
    if (this->parent())
        InfoWriter(this->parent()->objectName(), text);
}

void MessengerClass::SendError(QString text)
{
    if (this->parent())
        ErrorWriter(this->parent()->objectName(), text);
}

void MessengerClass::NewDeviceRegistration(QObject * Object)
{
    if (!Object)
        return;
    connect(this, SIGNAL(MessageSender(QString,QString,InterfaceData)), Object, SLOT(MessageReceiver(QString,QString,InterfaceData)));
    connect(Object, SIGNAL(MessageSender(QString,QString,InterfaceData)), this, SLOT(MessageReceiver(QString,QString,InterfaceData)));
}

void MessengerClass::WriteStatusMessage(QString Message)
{
    if(MessageStatusbar)
        MessageStatusbar->showMessage(Message,3000);
}

