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

#include "QCheckBox.h"
#include "CreateID.h"
#include "../mainwindow.h"

uint32_t QCheckBoxD::bitcounter = 0;
QCheckBoxD::QCheckBoxD(QWidget *parent):QCheckBox(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );
    return;
}


void QCheckBoxD::contextMenu(QPoint pos)
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

void QCheckBoxD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
    this->setChecked(0);
    setText("CheckBox");
}

void QCheckBoxD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QCheckBoxD::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    if (selectedItems[0]->childCount() == 0)
    {
        QString ID = CreateID(event->source());
        if(GetMainWindow()->GetLogic()->GetContainer(ID)->IsBool() ||
           GetMainWindow()->GetLogic()->GetContainer(ID)->IsUnsigedNumber())
            event->acceptProposedAction();
    }
}

void QCheckBoxD::dropEvent(QDropEvent *event)
{
    //this->disconnect();
    disconnect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    disconnect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    QString ID =  CreateID(event->source());
    this->setToolTip(ID);
    this->setToolTipDuration(2000);
    auto MW = GetMainWindow();

    if(MW->GetLogic()->GetContainer(ID)->IsBool())
    {
        this->bit = 0;
        this->setChecked(MW->GetLogic()->GetContainer(ID)->GetBool());
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

       this->setChecked((bool) (MW->GetLogic()->GetContainer(ID)->GetUnsignedData() & (1<<this->bit)));
    }
    QStringList sp = ID.split("::");
    QString label = sp.back();
    label += ":";
    label += QString::number(this->bit);
    this->setText(label);

    MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);

    MW->ChangeForSaveDetected = true;
}

void QCheckBoxD::SetVariantData(ToFormMapper Data)
{
    //DO NOT BLOCK SIGNALS !!!
    auto MW = GetMainWindow();
    disconnect(this, SIGNAL(clicked(bool)), MW->GetLogic(),SLOT(SendNewValue()));
    if(Data.IsBool())
    {
        setChecked(Data.GetBool());
        clicked(Data.GetUnsignedData() & (1<<GetBit()));

    }
    else if(Data.IsUnsigedNumber())
    {

        setChecked((bool) (Data.GetUnsignedData() & (1<<GetBit())));
        clicked(Data.GetUnsignedData() & (1<<GetBit()));

    }
     repaint();
     connect(this, SIGNAL(clicked(bool)), MW->GetLogic(),SLOT(SendNewValue()) );

}

void QCheckBoxD::GetVariantData(ToFormMapper *Data)
{

    if(Data->IsBool())
    {
        Data->SetDataKeepType(isChecked());
    }
    else if(Data->IsUnsigedNumber())
    {
        uint64_t numbr =Data->GetUnsignedData();
        if(isChecked())
           Data->SetDataKeepType( (numbr | 1<<GetBit()));
        else
         Data->SetDataKeepType( (numbr & ~(1<<GetBit())));
    }
}

bool QCheckBoxD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    for(auto itt: Attributes)
    {
        if(itt.first == "Bit")
            SetBit(itt.second.toUInt());
    }
    return true;

}

bool QCheckBoxD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    std::pair<QString, QString> Attribut;
    Attribut.first = "Bit";
    Attribut.second = QString::number(GetBit());
    Attributes.push_back(Attribut);

    return true;

}

void QCheckBoxD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    QStringList sp = ID.split("::");
    QString label = sp.back();
    label += ":";
    label += QString::number(GetBit());
    setText(label);
    connect(this, SIGNAL(clicked(bool)), DM, SLOT(SendNewValue()) );
    RequestUpdate();
}
