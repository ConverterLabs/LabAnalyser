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

#include "QTSLed.h"
#include "CreateID.h"
#include "../mainwindow.h"


uint32_t QTSLed::bitcounter = 0;
QTSLed::QTSLed(QWidget *parent):QTSLedIndicator(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    return;
}

void QTSLed::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QTSLed::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    if (selectedItems[0]->childCount() == 0)
    {
        QString ID = CreateID(event->source());
        QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();
        if(GetMainWindow()->GetLogic()->GetContainer(ID)->IsBool() ||
           GetMainWindow()->GetLogic()->GetContainer(ID)->IsUnsigedNumber())
            event->acceptProposedAction();
    }
}


void QTSLed::contextMenu(QPoint pos)
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

void QTSLed::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
}


void QTSLed::dropEvent(QDropEvent *event)
{
    this->disconnect();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    QString ID =  CreateID(event->source());
    auto MW = GetMainWindow();

    QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();

    if(MW->GetLogic()->GetContainer(ID)->IsBool())
    {
        this->bit = 0;
        this->SetState(MW->GetLogic()->GetContainer(ID)->GetBool());
    }
    else if(MW->GetLogic()->GetContainer(ID)->IsUnsigedNumber())
    {
       bool ok;
       int i = QInputDialog::getInt(this, tr("Index of Bit to be set"),
                                      tr("Index of Bit to be set (zero based):"), bitcounter, 0, 63, 1, &ok);

       bitcounter = i+1;
       if(bitcounter > 31)
           bitcounter =0;
       if (ok)
            this->bit = i;

       this->SetState((bool) (MW->GetLogic()->GetContainer(ID)->GetUnsignedData() & (1<<this->bit)));
    }

    this->setToolTip(ID + ":" + QString::number(this->bit));
    this->setToolTipDuration(2000);


    MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
    MW->ChangeForSaveDetected = true;
}

void QTSLed::SetVariantData(ToFormMapper Data)
{
    if(Data.IsBool())
        SetState(Data.GetBool());
    else if(Data.IsUnsigedNumber())
       SetState((bool) (Data.GetUnsignedData() & (1<<GetBit())));
     repaint();
}

void QTSLed::GetVariantData(ToFormMapper *Data)
{

}

bool QTSLed::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    for(auto itt: Attributes)
    {
        if(itt.first == "Bit")
            SetBit(itt.second.toUInt());
    }
    return true;

}

bool QTSLed::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    std::pair<QString, QString> Attribut;
    Attribut.first = "Bit";
    Attribut.second = QString::number(GetBit());
    Attributes.push_back(Attribut);

    return true;

}

void QTSLed::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID + ":" + QString::number(GetBit() ));
    RequestUpdate();
}
