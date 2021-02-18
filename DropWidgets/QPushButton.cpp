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

#include "QPushButton.h"
#include "CreateID.h"
#include "../mainwindow.h"


QPushButtonD::QPushButtonD(QWidget *parent) : QPushButton(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    return;
}

void QPushButtonD::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;

    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    if (selectedItems[0]->childCount() == 0)
    {
        QString ID = CreateID(event->source());
        QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetType();
        QString DataType = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();
        if(Type.compare("State")==0)
            event->acceptProposedAction();
        if(Type.compare("Parameter")==0 && (GetMainWindow()->GetLogic()->GetContainer(ID)->IsBool() || GetMainWindow()->GetLogic()->GetContainer(ID)->IsUnsigedNumber()))
            event->acceptProposedAction();
    }
}

void QPushButtonD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QPushButtonD::dropEvent(QDropEvent *event)
{
    this->disconnect();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    QString ID =  CreateID(event->source());
    this->setToolTip(ID);
    this->setToolTipDuration(2000);
    auto MW = GetMainWindow();

    //this->setChecked(MW->GetLogic()->GetContainer(ID)->GetBool());
    QStringList sp = ID.split("::");
    this->setText(sp.back());

    MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
    MW->ChangeForSaveDetected = true;
    connect(this, SIGNAL(pressed()), MW->GetLogic(),SLOT(SendNewValue()),Qt::DirectConnection );
    connect(this, SIGNAL(released()), MW->GetLogic(),SLOT(SendNewValue()),Qt::DirectConnection );
    connect(this, SIGNAL(pressed()), this, SLOT(StartTimeOut()),Qt::DirectConnection);

}

void QPushButtonD::StartTimeOut()
{
    QTimer::singleShot(100, this, SLOT(TimeOut()));
}

void QPushButtonD::TimeOut()
{
    if(this->isDown())
        QTimer::singleShot(100, this, SLOT(TimeOut()));
    else
    {
        if(GetMainWindow()->GetLogic()->GetContainer(this)->IsBool() == 0)
            if(GetMainWindow()->GetLogic()->GetContainer(this)->GetBool())
            {
                QTimer::singleShot(100, this, SLOT(TimeOut()));
                this->released();
            }
        if(GetMainWindow()->GetLogic()->GetContainer(this)->IsUnsigedNumber())
            if(GetMainWindow()->GetLogic()->GetContainer(this)->GetUnsignedData())
            {
                QTimer::singleShot(100, this, SLOT(TimeOut()));
                this->released();
            }
    }
}

void QPushButtonD::contextMenu(QPoint pos)
{
    QMenu* menu = new QMenu(this);
    QString Connection = GetMainWindow()->GetLogic()->GetContainerID(this);
    if(Connection.size())
    {
         MainWindow *MW = GetMainWindow();
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

void QPushButtonD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
    this->setText("");
}

void QPushButtonD::SetVariantData(ToFormMapper Data)
{

}

void QPushButtonD::GetVariantData(ToFormMapper *Data)
{
    Data->SetDataKeepType(isDown());
}

bool QPushButtonD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QPushButtonD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QPushButtonD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    setText(ID.split("::").back());
    connect(this, SIGNAL(pressed()), DM, SLOT(SendNewValue()),Qt::DirectConnection );
    connect(this, SIGNAL(released()), DM, SLOT(SendNewValue()),Qt::DirectConnection );
}
