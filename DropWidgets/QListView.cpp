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

#include "QListView.h"
#include "CreateID.h"
#include "../mainwindow.h"



QListViewD::QListViewD(QWidget *parent):QListView(parent)
{
     model = new QStringListModel(this);
     this->setModel(model);
     this->setContextMenuPolicy(Qt::CustomContextMenu);
     connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
     connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

      return;
}

void QListViewD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();

}

void QListViewD::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    if(GetMainWindow()->GetLogic()->IsObjectLinked(this))
    {
         event->acceptProposedAction();
    }

    if(selectedItems.size()==1)
    {
        if (selectedItems[0]->childCount() == 0)
        {
            QString ID = CreateID(event->source());
            QString DataType = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();
            QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetType();

            if(GetMainWindow()->GetLogic()->GetContainer(ID)->IsStringList() && Type.compare("Parameter") == 0 )
                event->acceptProposedAction();
            GetMainWindow()->GetStatusBar()->showMessage("This List is not linked to a List Parameter",2000);
        }
    }
}

void QListViewD::dropEvent(QDropEvent *event)
{


    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
    auto MW = GetMainWindow();
    for(int i = 0; i < selectedItems.size(); i++)
    {
        QString ID;
        if (selectedItems[i]->childCount() == 0)
        {
            auto sit = selectedItems[i];
            while(sit)
            {
                ID.insert(0,sit->text( 0 ));
                sit = sit->parent();
                if(sit)
                    ID.insert(0,"::");
            }
            QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();
            if(Type.compare("QStringList")==0)
            {
                this->disconnect();
                connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
                connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

                QString ID =  CreateID(event->source());
                this->setToolTip(ID);
                this->setToolTipDuration(2000);
                model->setStringList((GetMainWindow()->GetLogic()->GetContainer(ID)->GetStringList()));
                MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
                 MW->ChangeForSaveDetected = true;
                 connect(this, SIGNAL(NewEntry()), MW->GetLogic(),SLOT(SendNewValue()) );

            }
            else
            {
                auto liste = model->stringList();
                liste.append(ID);
                model->setStringList(liste);
                emit NewEntry();
            }
        }

    }



}

void QListViewD::contextMenu(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction("Delete Entry", this, SLOT(DeleteEntry()));
    menu->addAction("Delete all Entries", this, SLOT(DeleteAllEntries()));

    menu->popup(this->mapToGlobal(pos));
}

void QListViewD::DeleteEntry()
{
   model->removeRow(this->currentIndex().row());
   emit NewEntry();
}

void QListViewD::DeleteAllEntries()
{
   model->removeRows(0, model->rowCount());
   emit NewEntry();
}

void QListViewD::SetVariantData(ToFormMapper Data)
{
    blockSignals(true);

    if(Data.IsStringList())
        model->setStringList(Data.GetStringList());
    blockSignals(false);

}

void QListViewD::GetVariantData(ToFormMapper *Data)
{
    if(model->rowCount())
        Data->SetData(model->stringList());
    else
         Data->SetData(QStringList(QString("")));


}


bool QListViewD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QListViewD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QListViewD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    connect(this, SIGNAL(NewEntry()),DM , SLOT(SendNewValue()));
    RequestUpdate();
}
