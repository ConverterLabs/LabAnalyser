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

#include "QLineEdit.h"
#include "CreateID.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetDropBinding.h"
#include "DropWidgetTreePath.h"
#include "DropWidgetUpdate.h"
#include "app/mainwindow.h"



//======================================================================================
//======================================================================================
//===============                   QLineEditD                  ========================
//======================================================================================
//======================================================================================


QLineEditD::QLineEditD(QWidget *parent):QLineEdit(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

    return;
}

void QLineEditD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { true, false, false, true, true });
}

void QLineEditD::RemoveConnection()
{

    DropWidgetDropBinding::ClearConnectionPresentation(this);
    DropWidgetDropBinding::RemoveManagerBinding(this);
    this->setText("");
}


void QLineEditD::dragEnterEvent(QDragEnterEvent *event)
{
    ToFormMapper* container = DropWidgetDragSource::ContainerForFirstSelectedLeaf(event->source());
    if (container && (container->IsFloatingPointNumber() || container->IsUnsigedNumber()
                      || container->IsSigedNumber() || container->IsString()))
        event->acceptProposedAction();
}

void QLineEditD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QLineEditD::dropEvent(QDropEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
    MainWindow *MW = GetMainWindow();
    if (!MW)
        return;

    bool rebound = false;

    for(auto si : selectedItems)
    {
        if(si->childCount() == 0)
        {
            for( int col = 0; col < 1; ++col )
            {
                QString ToolTip = DropWidgetTreePath::IdForItem(si, col);
               ToFormMapper* container = MW->GetLogic()->GetContainer(ToolTip);
               if (!container || !(container->IsFloatingPointNumber() || container->IsUnsigedNumber()
                                   || container->IsSigedNumber() || container->IsString()))
                   continue;

               if (!rebound)
               {
                   DropWidgetDropBinding::ResetConnections(this);
                   rebound = true;
               }
               this->setToolTip(ToolTip);
               this->setToolTipDuration(2000);

               MW->GetLogic()->DeleteEntryOfObject(this);
               MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ToolTip,this->metaObject()->className(),this);
               MW->ChangeForSaveDetected = true;
               if(!(this->isReadOnly()))
               DropWidgetBinding::ConnectValueChanged(this, SIGNAL(editingFinished()), MW->GetLogic());
               DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

               emit RequestUpdate();

            }
        }
    }
}

void QLineEditD::SetVariantData(ToFormMapper Data)
{
    ApplyDropWidgetUpdate(this, [&]{
    setText(Data.GetString());
    });


}

void QLineEditD::GetVariantData(ToFormMapper *Data)
{
    if (!Data)
        return;
    Data->SetData(text());
}


bool QLineEditD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QLineEditD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QLineEditD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(editingFinished()), DM);
    RequestUpdate();
}
