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

#include "QSlider.h"
#include "CreateID.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetDropBinding.h"
#include "DropWidgetDataAccess.h"
#include "DropWidgetUpdate.h"
#include "../mainwindow.h"




QSliderD::QSliderD(QWidget *parent):QSlider(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));
    this->setMaximum(0);
    this->setMaximum(100);
      return;
}


void QSliderD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { true, false, false, false, true });
}

void QSliderD::RemoveConnection()
{

    DropWidgetDropBinding::ClearConnectionPresentation(this);
    DropWidgetDropBinding::RemoveManagerBinding(this);
    this->setValue(0);
}


void QSliderD::dragEnterEvent(QDragEnterEvent *event)
{
    ToFormMapper* container = DropWidgetDragSource::ContainerForFirstSelectedLeaf(event->source());
    if (container && (container->IsFloatingPointNumber() || container->IsUnsigedNumber() || container->IsSigedNumber()))
        event->acceptProposedAction();
}

void QSliderD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QSliderD::dropEvent(QDropEvent *event)
{
               const DropWidgetDropBinding::Context context = DropWidgetDropBinding::Prepare(this, event);
               if (!context.IsValid())
                   return;
               context.manager->DeleteEntryOfObject(this);


               // dot instead of comma
               QLocale::setDefault(QLocale::c());

               MinMax = context.manager->MinMaxValue(context.id);
               double MinValue = MinMax.first;
               double MaxValue = MinMax.second;
               if(MinMax.first == MinMax.second)
               {
                    MinValue = QInputDialog::getDouble(this, tr("Minimal Value of"),
                                                        context.id, 0,-2147483647, 2147483647,5);
                    MaxValue = QInputDialog::getDouble(this, tr("Maximum Value of"),
                                                        context.id, 0,-2147483647, 2147483647,5);
                   context.manager->SetMinMaxValue(context.id,MinValue,MaxValue);
               }

               MinMax = context.manager->MinMaxValue(context.id);
               DropWidgetDropBinding::Register(this, context);
               DropWidgetBinding::ConnectValueChanged(this, SIGNAL(valueChanged(int)), context.manager);
               ConnectedID = context.id;
               emit RequestUpdate();



}

void QSliderD::SetVariantData(ToFormMapper Data)
{
    //TODO Minmax übernehmen
    //HIer weiter
    ApplyDropWidgetUpdate(this, [&]{
    if(Data.IsEditable()) {
        auto MW = GetMainWindow();
        if(ConnectedID.size())
        {
            if (!MW)
                return;
            if(MW->GetLogic()->ElementExists(ConnectedID))
                MinMax = MW->GetLogic()->MinMaxValue(ConnectedID);
        }

        double value = 0.0;
        if(DropWidgetDataAccess::TryReadNumeric(Data, &value) && MinMax.first != MinMax.second)
        {
            int valueC = (int) round((value - MinMax.first)/(MinMax.second-MinMax.first)*100.0);
            setValue((int)valueC );
        }
    }});




}

void QSliderD::GetVariantData(ToFormMapper *Data)
{
     int valuei = value();
     std::pair<double, double> MinMax = std::pair<double, double>(Data->MinValue,Data->MaxValue);
     double ValueScaled = ((double) valuei)/100.0 * (MinMax.second - MinMax.first) + MinMax.first;

     Data->SetDataKeepType(ValueScaled);

}


bool QSliderD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QSliderD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QSliderD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    auto MW = GetMainWindow();
    if (!MW)
        return;
    QLocale::setDefault(QLocale::c());
    ConnectedID = ID;
    MinMax = MW->GetLogic()->MinMaxValue(ID);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(valueChanged(int)), DM);
    RequestUpdate();
}
