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

#include "QDoubleSpinBox.h"
#include "CreateID.h"
#include "../mainwindow.h"


QDoubleSpinBoxD::QDoubleSpinBoxD(QWidget *parent):QDoubleSpinBox(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

        return;
}


void QDoubleSpinBoxD::contextMenu(QPoint pos)
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

void QDoubleSpinBoxD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
    this->setValue(0);
}

void QDoubleSpinBoxD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QDoubleSpinBoxD::dragEnterEvent(QDragEnterEvent *event)
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

void QDoubleSpinBoxD::dropEvent(QDropEvent *event)
{
    this->disconnect();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    QString ID =  CreateID(event->source());
    this->setToolTip(ID);
    this->setToolTipDuration(2000);
    auto MW = GetMainWindow();

    QString Type = MW->GetLogic()->GetContainer(ID)->GetDataType();
    std::pair<double,double> MinMax = MW->GetLogic()->MinMaxValue(ID);
    this->setMinimum(MinMax.first);
    this->setMaximum(MinMax.second);

    MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
    MW->ChangeForSaveDetected = true;
    connect(this, SIGNAL(valueChanged(double)), MW->GetLogic(),SLOT(SendNewValue()) );
    emit RequestUpdate();
}

void QDoubleSpinBoxD::SetVariantData(ToFormMapper Data)
{
      if(Data.IsFloatingPointNumber())
           setValue(Data.GetFloatingPointData());
      else if(Data.IsUnsigedNumber())
         setValue((double) Data.GetUnsignedData());
      else if (Data.IsSigedNumber() )
          setValue((double) Data.GetSignedData());
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
    connect(this, SIGNAL(valueChanged(double)), DM, SLOT(SendNewValue()) );
    RequestUpdate();
}
