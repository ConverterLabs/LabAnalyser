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

#include "DataManagementSetClass.h"
#include "DropWidgets/DropWidgets.h"
#include "DropWidgets/Plots/PlotWidget.h"
#include "mainwindow.h"
#undef GetObject //Bug same name as windows.h macro

DataManagementSetClass::DataManagementSetClass(QObject *parent) : DataManagementClass(parent)
{
    Q_ASSERT(parent->objectName()==  QString("LabAnalyser"));
    MainWindow* MW = qobject_cast<MainWindow*>(this->parent());
    if(MW)
    {
        Messenger = new MessengerClass(this, MW->GetStatusBar());
        connect(Messenger, SIGNAL(PublishFinished()),MW,SLOT(PublishFinished()));
        connect(Messenger, SIGNAL(PublishStart()),MW,SLOT(PublishStart()));
    }
    else
        Messenger = new MessengerClass(this, nullptr);

}


void DataManagementSetClass::SetData(const QString &ID)
{
    if(!this->GetContainer(ID))
        return;

    ToFormMapper Data = *(this->GetContainer(ID));

    //verhindern, dass das setzten von neuen Daten ein Signal auslöst
    //for(auto i = 0; i < this->GetContainer(ID)->Objects.size(); i++)
    //{
    //        this->GetContainer(ID)->Objects[i].FormP->blockSignals(true);
    //}
    //Get The Container that contains the information about the qobject pointer
    ToFormMapper* Ele =  this->GetContainer(ID);
    //Go through all elements of the container
    for(auto i = 0; i < Ele->Objects.size(); i++)
    {
        auto DCObj = dynamic_cast<VariantDropWidget*>(Ele->Objects[i].FormP);
		if(DCObj)
		{
			DCObj->SetVariantData(Data);
        }
    }
    //for(auto i = 0; i < this->GetContainer(ID)->Objects.size(); i++)
    //    this->GetContainer(ID)->Objects[i].FormP->blockSignals(false);
}


//Neue Daten wurden vov GUI Objekt empfangen
void DataManagementSetClass::SendNewValue()
{
    auto SenderOC = QObject::sender();
    QString ID = this->GetContainerID(SenderOC);
    if(!ID.size())
        return;

    if(this->GetContainer(ID)->GetType().compare("Data") == 0)
    {
        //Data sould not be changed, request update with original data
        emit MessageSender("get", ID, InterfaceData());
        return;
    }

    auto DCObj = dynamic_cast<VariantDropWidget*>(SenderOC);
    if(DCObj)
    {
        DCObj->GetVariantData(this->GetContainer(SenderOC));
        emit MessageSender("set", ID, this->GetInterfaceData(SenderOC));
    }
}


void DataManagementSetClass::UpdateRequest()
{
    auto SenderOC = QObject::sender();
    {
        emit MessageSender("get", this->GetContainerID(SenderOC->objectName()), InterfaceData());
    }
}

void DataManagementSetClass::UpdateRequest(QString ID)
{
        emit MessageSender("get", ID, InterfaceData());
}

