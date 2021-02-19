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
    //Check if we already know this Data
    if(Command.compare("publish")==0)
    {
         emit AddContainerElement(ID,Data.GetDataType(),Data.GetType(),Data.GetStateDependency());
         emit SetData(ID, Data);
         //Add elemet to ListWidget
         emit AddElementToWidget(ID,Data);
         MessageReceiver("set", ID, Data);
    }
    else if(Command.compare("StatusMessage")== 0)
    {
        if(MessageStatusbar)
            MessageStatusbar->showMessage(ID + " -> " + Data.GetString(),1000);
        return;
    }
    else if(Command.compare("remove")==0)
    {
//            this->this->RemoveID(ID);
//            this->RemoveElementFromWidget(ID);
    }
    else if(Command.compare("CloseProject")==0)
    {
        InterfaceData Data;
        Data.SetData("Closing forced by: " + ID);
        NotificationWriter(this->parent()->parent()->objectName(), Data.GetString());
        emit CloseProject();
    }
    else if(Command.compare("error")==0)
    {
        emit ErrorWriter(ID, Data.GetString());
    }
    else if(Command.compare("info")==0)
    {
        emit InfoWriter(ID, Data.GetString());
    }
    if(Command.compare("notification")==0)
    {
        emit NotificationWriter(ID, Data.GetString());
    }
    if(Command.compare("set")==0)
    {
       emit SetData(ID,Data);
       emit NewDataReceived(ID);
    }
    if(Command.compare("publish_finished")==0)
    {
        emit PublishFinished();
    }
    if(Command.compare("publish_start")==0)
    {
        emit PublishStart();
    }
}

void MessengerClass::MessageTransmitter(const QString &Command, const QString &ID, InterfaceData Data)
{
    MessageReceiver( Command,   ID,  Data);
    emit MessageSender(  Command,   ID,  Data);
}


void MessengerClass::SendInfo(QString text)
{
    InfoWriter(this->parent()->objectName(), text);
}

void MessengerClass::SendError(QString text)
{
    ErrorWriter(this->parent()->objectName(), text);
}

void MessengerClass::NewDeviceRegistration(QObject * Object)
{
    connect(this, SIGNAL(MessageSender(QString,QString,InterfaceData)), Object, SLOT(MessageReceiver(QString,QString,InterfaceData)));
    connect(Object, SIGNAL(MessageSender(QString,QString,InterfaceData)), this, SLOT(MessageReceiver(QString,QString,InterfaceData)));
}

void MessengerClass::WriteStatusMessage(QString Message)
{
    if(MessageStatusbar)
        MessageStatusbar->showMessage(Message,3000);
}

