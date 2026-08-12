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

#include "QComboBox.h"
#include "CreateID.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetDropBinding.h"
#include "DropWidgetUpdate.h"
#include "../mainwindow.h"

QComboBoxD::QComboBoxD(QWidget *parent):QComboBox(parent)
{
     this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

        return;
}

void QComboBoxD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { false, false, false, false, true });
}

void QComboBoxD::RemoveConnection()
{

    DropWidgetDropBinding::ClearConnectionPresentation(this);
    DropWidgetDropBinding::RemoveManagerBinding(this);
    this->clear();
}

void QComboBoxD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QComboBoxD::dragEnterEvent(QDragEnterEvent *event)
{
    ToFormMapper* container = DropWidgetDragSource::ContainerForFirstSelectedLeaf(event->source());
    if (container && container->IsGuiSelection())
        event->acceptProposedAction();
}

void QComboBoxD::dropEvent(QDropEvent *event)
{
    const DropWidgetDropBinding::Context context = DropWidgetDropBinding::Prepare(this, event);
    if (!DropWidgetDropBinding::SupportsGuiSelection(context))
        return;
    ToFormMapper* container = context.container;

    DropWidgetDropBinding::Activate(this, context);

    while(this->count())
        this->removeItem(0);

    try
    {
        auto Sel = container->GetGuiSelection();
        for(int i = 0; i < Sel.second.size();i++)
        {
            this->addItem(Sel.second[i]);
        }
        this->setCurrentText(Sel.first);

        DropWidgetDropBinding::Register(this, context);
        DropWidgetBinding::ConnectValueChanged(this, SIGNAL(currentIndexChanged(int)), context.manager);
    }
    catch(...)
    {
        if (MainWindow* mainWindow = GetMainWindow())
            mainWindow->Error("Incompatible Datatype deteced, please check plugin for error");
    }


}

void QComboBoxD::SetVariantData(ToFormMapper Data)
{
    ApplyDropWidgetUpdate(this, [&]{
    if(Data.IsEditable() && Data.IsGuiSelection())
    {
        while(count())
            removeItem(0);
         auto Sel = Data.GetGuiSelection();
         for(int i = 0; i < Sel.second.size();i++)
         {
             addItem(Sel.second[i]);
        }
         this->setCurrentText(Sel.first);
    }
    });

}

void QComboBoxD::GetVariantData(ToFormMapper *Data)
{
    if (!Data || !Data->IsGuiSelection())
        return;
    GuiSelection Sel = Data->GetGuiSelection();
    Sel.first = currentText();
    Data->SetData(Sel);
}

bool QComboBoxD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QComboBoxD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QComboBoxD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(currentIndexChanged(int)), DM);
    RequestUpdate();
}
