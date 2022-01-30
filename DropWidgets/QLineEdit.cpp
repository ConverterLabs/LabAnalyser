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
#include "../mainwindow.h"



//======================================================================================
//======================================================================================
//===============                   QLineEditD                  ========================
//======================================================================================
//======================================================================================


QLineEditD::QLineEditD(QWidget *parent):QLineEdit(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    return;
}

void QLineEditD::contextMenu(QPoint pos)
{
    QMenu* menu = createStandardContextMenu();
    QString Connection = GetMainWindow()->GetLogic()->GetContainerID(this);
    if(Connection.size())
    {
         MainWindow *MW = GetMainWindow();
         menu->addSeparator();
        QAction *Highlight = new QAction;
        connect(Highlight, &QAction::triggered, [=]{
            MW->HighLightConnection(Connection);});
        Highlight->setText("Highlight Connection");
        menu->addAction(Highlight);
        menu->addSeparator();

        menu->addAction("Remove Connection", this , SLOT(RemoveConnection()));
    }
    menu->popup(this->mapToGlobal(pos));

}

void QLineEditD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
    this->setText("");
}


void QLineEditD::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
    if (selectedItems[0]->childCount() == 0)
    {
        QString ID = CreateID(event->source());
        QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();
        if( GetMainWindow()->GetLogic()->GetContainer(ID)->IsFloatingPointNumber() ||
            GetMainWindow()->GetLogic()->GetContainer(ID)->IsUnsigedNumber() ||
            GetMainWindow()->GetLogic()->GetContainer(ID)->IsSigedNumber() ||
            GetMainWindow()->GetLogic()->GetContainer(ID)->IsString())
                event->acceptProposedAction();
    }
}

void QLineEditD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QLineEditD::dropEvent(QDropEvent *event)
{
    this->disconnect();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );


    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    for(auto si : selectedItems)
    {
        if(si->childCount() == 0)
        {
            for( int col = 0; col < 1; ++col )
            {
                QString ToolTip;
                auto sit = si;
                while(sit)
                {
                    ToolTip.insert(0,sit->text( col ));
                    sit = sit->parent();
                    if(sit)
                        ToolTip.insert(0,"::");
                }
               this->setToolTip(ToolTip);
               this->setToolTipDuration(2000);



               MainWindow *MW = GetMainWindow();
               MW->GetLogic()->DeleteEntryOfObject(this);
               MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ToolTip,this->metaObject()->className(),this);
               MW->ChangeForSaveDetected = true;
               if(!(this->isReadOnly()))
               connect(this, SIGNAL(editingFinished()), MW->GetLogic(),SLOT(SendNewValue()) );
               connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

               emit RequestUpdate();

            }
        }
    }
}

void QLineEditD::SetVariantData(ToFormMapper Data)
{
    setText(Data.GetString());

}

void QLineEditD::GetVariantData(ToFormMapper *Data)
{
    blockSignals(true);
    Data->SetData(text());
    blockSignals(false);
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
    connect(this, SIGNAL(editingFinished()), DM, SLOT(SendNewValue()) );
    RequestUpdate();
}
