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

#include "QSpinBox.h"
#include "CreateID.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetDropBinding.h"
#include "DropWidgetDataAccess.h"
#include "DropWidgetUpdate.h"
#include "../mainwindow.h"


QSpinBoxD::QSpinBoxD(QWidget *parent):QSpinBox(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));
    return;
}


void QSpinBoxD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { true, false, false, false, true });
}

void QSpinBoxD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
    this->setValue(0);
}

void QSpinBoxD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QSpinBoxD::dragEnterEvent(QDragEnterEvent *event)
{
    if (!DropWidgetDragSource::HasFirstSelectedLeaf(event->source()))
        return;

    ToFormMapper* container = GetMainWindow()->GetLogic()->GetContainer(CreateID(event->source()));
    if (container && (container->IsFloatingPointNumber() || container->IsUnsigedNumber() || container->IsSigedNumber()))
        event->acceptProposedAction();
}

void QSpinBoxD::dropEvent(QDropEvent *event)
{
    const DropWidgetDropBinding::Context context = DropWidgetDropBinding::Prepare(this, event);
    ToFormMapper* container = context.manager->GetContainer(context.id);

    QString Type = container->GetDataType();
    std::pair<double,double> MinMax = context.manager->MinMaxValue(context.id);
    this->setMinimum((int)MinMax.first);
    this->setMaximum((int)MinMax.second);

    DropWidgetDropBinding::Register(this, context);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(valueChanged(int)), context.manager);
    ConnectedID = context.id;
    emit RequestUpdate();
}

void QSpinBoxD::SetVariantData(ToFormMapper Data)
{
    ApplyDropWidgetUpdate(this, [&]{
    double value = 0.0;
    if (DropWidgetDataAccess::TryReadNumeric(Data, &value))
        setValue(static_cast<int>(value));

    });


}

void QSpinBoxD::GetVariantData(ToFormMapper *Data)
{
    Data->SetDataKeepType(value());
}

bool QSpinBoxD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QSpinBoxD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QSpinBoxD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(valueChanged(int)), DM);
    RequestUpdate();
}
