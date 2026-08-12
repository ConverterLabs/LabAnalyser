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

#include "QDoubleSpinBox.h"
#include "CreateID.h"
#include "DropWidgetDataAccess.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetDropBinding.h"
#include "DropWidgetUpdate.h"
#include "../mainwindow.h"


QDoubleSpinBoxD::QDoubleSpinBoxD(QWidget *parent):QDoubleSpinBox(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

        return;
}


void QDoubleSpinBoxD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { true, false, false, false, true });
}

void QDoubleSpinBoxD::RemoveConnection()
{

    DropWidgetDropBinding::ClearConnectionPresentation(this);
    DropWidgetDropBinding::RemoveManagerBinding(this);
    this->setValue(0);
}

void QDoubleSpinBoxD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QDoubleSpinBoxD::dragEnterEvent(QDragEnterEvent *event)
{
    ToFormMapper* container = DropWidgetDragSource::ContainerForFirstSelectedLeaf(event->source());
    if (container && (container->IsFloatingPointNumber() || container->IsUnsigedNumber() || container->IsSigedNumber()))
        event->acceptProposedAction();
}

void QDoubleSpinBoxD::dropEvent(QDropEvent *event)
{
    const DropWidgetDropBinding::Context context = DropWidgetDropBinding::Prepare(this, event);
    if (!DropWidgetDropBinding::SupportsNumeric(context))
        return;
    DropWidgetDropBinding::Activate(this, context);
    std::pair<double,double> MinMax = context.manager->MinMaxValue(context.id);
    this->setMinimum(MinMax.first);
    this->setMaximum(MinMax.second);

    DropWidgetDropBinding::Register(this, context);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(valueChanged(double)), context.manager);
    emit RequestUpdate();
}

void QDoubleSpinBoxD::SetVariantData(ToFormMapper Data)
{
    ApplyDropWidgetUpdate(this, [&]{
      double value = 0.0;
      if (DropWidgetDataAccess::TryReadNumeric(Data, &value))
          setValue(value);
      });

}

void QDoubleSpinBoxD::GetVariantData(ToFormMapper *Data)
{
     Data->SetDataKeepType(value());
}

bool QDoubleSpinBoxD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QDoubleSpinBoxD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QDoubleSpinBoxD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(valueChanged(double)), DM);
    RequestUpdate();
}
