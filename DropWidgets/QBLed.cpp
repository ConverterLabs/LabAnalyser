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

#include "QBLed.h"
#include "CreateID.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDataAccess.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetIndicatorBinding.h"
#include "DropWidgetUpdate.h"
#include "../mainwindow.h"

uint32_t QBLed::bitcounter = 0;
QBLed::QBLed(QWidget *parent):QBLedIndicator(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));
    return;
}

void QBLed::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}


void QBLed::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { false, false, false, false, true });
}

void QBLed::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    this->SetState(0);

    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
}


void QBLed::dragEnterEvent(QDragEnterEvent *event)
{
    ToFormMapper* container = DropWidgetDragSource::ContainerForFirstSelectedLeaf(event->source());
    if (container && (container->IsBool() || container->IsUnsigedNumber()))
        event->acceptProposedAction();
}

void QBLed::dropEvent(QDropEvent *event)
{
    DropWidgetIndicatorBinding::BindFromDrop(this, event, bit, bitcounter);
}

void QBLed::SetVariantData(ToFormMapper Data)
{
    ApplyDropWidgetUpdate(this, [&]{
    bool state = false;
    if (DropWidgetDataAccess::TryReadIndicatorState(Data, GetBit(), &state))
        SetState(state);
     repaint();
    });
}


void QBLed::GetVariantData(ToFormMapper *Data)
{

}

bool QBLed::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    uint32_t bit = 0;
    if (DropWidgetDataAccess::LoadBitAttribute(Attributes, &bit))
        SetBit(bit);
    return true;

}

bool QBLed::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    DropWidgetDataAccess::SaveBitAttribute(Attributes, GetBit());

    return true;

}

void QBLed::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID + ":" + QString::number(GetBit() ));
    RequestUpdate();
}

