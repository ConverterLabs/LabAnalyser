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

#include "QProgressBar.h"
#include "CreateID.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDataAccess.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetDropBinding.h"
#include "DropWidgetUpdate.h"
#include "../mainwindow.h"



QProgressBarD::QProgressBarD(QWidget *parent):QProgressBar(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

           return;
}

void QProgressBarD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}


void QProgressBarD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { true, false, false, false, true });
}

void QProgressBarD::RemoveConnection()
{

    DropWidgetDropBinding::ClearConnectionPresentation(this);
    DropWidgetDropBinding::RemoveManagerBinding(this);
}

void QProgressBarD::dragEnterEvent(QDragEnterEvent *event)
{
    ToFormMapper* container = DropWidgetDragSource::ContainerForFirstSelectedLeaf(event->source());
    if (container && (container->IsFloatingPointNumber() || container->IsUnsigedNumber() || container->IsSigedNumber()))
        event->acceptProposedAction();
}

void QProgressBarD::dropEvent(QDropEvent *event)
{
    const DropWidgetDropBinding::Context context = DropWidgetDropBinding::Prepare(this, event);
    DropWidgetDropBinding::Register(this, context);
     emit RequestUpdate();
}

void QProgressBarD::SetVariantData(ToFormMapper Data)
{
    ApplyDropWidgetUpdate(this, [&]{

    double value = 0.0;
    if (DropWidgetDataAccess::TryReadNumeric(Data, &value))
        setValue(static_cast<int>(value));
    });

}


void QProgressBarD::GetVariantData(ToFormMapper *Data)
{

}

bool QProgressBarD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QProgressBarD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QProgressBarD::ConnectToID(DataManagementSetClass* DM, QString ID)
{

}
