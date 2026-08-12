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

#include "QLCDNumber.h"
#include "CreateID.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDataAccess.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetUpdate.h"
#include "../mainwindow.h"


QLCDNumberD::QLCDNumberD(QWidget *parent):QLCDNumber(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

           return;
}

void QLCDNumberD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}


void QLCDNumberD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { true, false, false, false, true });
}

void QLCDNumberD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
}

void QLCDNumberD::dragEnterEvent(QDragEnterEvent *event)
{
    if (!DropWidgetDragSource::HasFirstSelectedLeaf(event->source()))
        return;

    ToFormMapper* container = GetMainWindow()->GetLogic()->GetContainer(CreateID(event->source()));
    if (container && (container->IsFloatingPointNumber() || container->IsUnsigedNumber() || container->IsSigedNumber()))
        event->acceptProposedAction();
}

void QLCDNumberD::dropEvent(QDropEvent *event)
{
    this->disconnect();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

    QString ID =  CreateID(event->source());
    this->setToolTip(ID);
    this->setToolTipDuration(2000);
    auto MW = GetMainWindow();

    QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();

    /*if(Type.compare("double")==0)
    {
        this->display((GetMainWindow()->GetLogic()->GetContainer(ID)->GetDouble()));
    }
    else if(Type.compare("int")==0)
    {
        this->display((double) GetMainWindow()->GetLogic()->GetContainer(ID)->GetInt());
    }
    else if(Type.compare("float")==0)
    {
        this->display((double) (GetMainWindow()->GetLogic()->GetContainer(ID)->GetFloat()));
    }*/

    MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
     MW->ChangeForSaveDetected = true;
     emit RequestUpdate();

}

void QLCDNumberD::SetVariantData(ToFormMapper Data)
{
    ApplyDropWidgetUpdate(this, [&]{
    double value = 0.0;
    if (DropWidgetDataAccess::TryReadNumeric(Data, &value))
        display(value);
    });
}

void QLCDNumberD::GetVariantData(ToFormMapper *Data)
{

}

bool QLCDNumberD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QLCDNumberD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QLCDNumberD::ConnectToID(DataManagementSetClass* DM, QString ID)
{

}
