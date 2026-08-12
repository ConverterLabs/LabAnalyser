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
        menu->addSeparator();

        menu->addAction("Remove Connection", this , SLOT(RemoveConnection()));
    }
    menu->popup(this->mapToGlobal(pos));

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
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    if (selectedItems[0]->childCount() == 0)
    {
        QString ID = CreateID(event->source());
        QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();
        if(GetMainWindow()->GetLogic()->GetContainer(ID)->IsFloatingPointNumber()||
           GetMainWindow()->GetLogic()->GetContainer(ID)->IsUnsigedNumber()||
           GetMainWindow()->GetLogic()->GetContainer(ID)->IsSigedNumber())
            event->acceptProposedAction();
    }
}

void QSpinBoxD::dropEvent(QDropEvent *event)
{
    this->disconnect();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

    QString ID =  CreateID(event->source());
    this->setToolTip(ID);
    this->setToolTipDuration(2000);
    auto MW = GetMainWindow();

    QString Type = MW->GetLogic()->GetContainer(ID)->GetDataType();
    std::pair<double,double> MinMax = MW->GetLogic()->MinMaxValue(ID);
    this->setMinimum((int)MinMax.first);
    this->setMaximum((int)MinMax.second);

    MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
    MW->ChangeForSaveDetected = true;
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(valueChanged(int)), MW->GetLogic());
    ConnectedID = ID;
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
