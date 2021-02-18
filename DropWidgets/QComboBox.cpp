/***************************************************************************
**                                                                        **
**  This file is part of LabAnlyser.                                      **
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
#include "../mainwindow.h"

QComboBoxD::QComboBoxD(QWidget *parent):QComboBox(parent)
{
     this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

        return;
}

void QComboBoxD::contextMenu(QPoint pos)
{
    QMenu* menu = new QMenu(this);
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
        menu->addAction("Remove Connection", this , SLOT(RemoveConnection()));
    }
    menu->popup(this->mapToGlobal(pos));

}

void QComboBoxD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
    this->clear();
}

void QComboBoxD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QComboBoxD::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    if (selectedItems[0]->childCount() == 0)
    {
        QString ID = CreateID(event->source());
        QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();
        if(GetMainWindow()->GetLogic()->GetContainer(ID)->IsGuiSelection())
            event->acceptProposedAction();
    }
}

void QComboBoxD::dropEvent(QDropEvent *event)
{
    this->disconnect();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    QString ID =  CreateID(event->source());
    this->setToolTip(ID);
    this->setToolTipDuration(2000);
    auto MW = GetMainWindow();

    QString Type = MW->GetLogic()->GetContainer(ID)->GetDataType();

    while(this->count())
        this->removeItem(0);

    try
    {
        auto Sel = GetMainWindow()->GetLogic()->GetContainer(ID)->GetGuiSelection();
        for(int i = 0; i < Sel.second.size();i++)
        {
            this->addItem(Sel.second[i]);
        }
        this->setCurrentText(Sel.first);

        MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
        MW->ChangeForSaveDetected = true;
        connect(this, SIGNAL(currentIndexChanged(int)),GetMainWindow()->GetLogic(),SLOT(SendNewValue()));
    }
    catch(...)
    {
        GetMainWindow()->Error("Incompatible Datatype deteced, please check plugin for error");
    }


}

void QComboBoxD::SetVariantData(ToFormMapper Data)
{
    if(Data.IsEditable() && Data.IsGuiSelection())
    {
        while(count())
            removeItem(0);
         auto Sel = Data.GetGuiSelection();
         for(int i = 0; i < Sel.second.size();i++)
         {
             addItem(Sel.second[i]);
        }
    }
}

void QComboBoxD::GetVariantData(ToFormMapper *Data)
{
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
    connect(this, SIGNAL(currentIndexChanged(int)), DM, SLOT(SendNewValue()));
    RequestUpdate();
}
