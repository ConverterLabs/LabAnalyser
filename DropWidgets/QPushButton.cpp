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

#include "QPushButton.h"
#include "CreateID.h"
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetDropBinding.h"
#include "../mainwindow.h"


QPushButtonD::QPushButtonD(QWidget *parent) : QPushButton(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));

    return;
}

void QPushButtonD::dragEnterEvent(QDragEnterEvent *event)
{
    ToFormMapper* container = DropWidgetDragSource::ContainerForFirstSelectedLeaf(event->source());
    if (!container)
        return;
    if (container->GetType().compare("State") == 0
            || (container->GetType().compare("Parameter") == 0
                && (container->IsBool() || container->IsUnsigedNumber())))
        event->acceptProposedAction();
}

void QPushButtonD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QPushButtonD::dropEvent(QDropEvent *event)
{
    const DropWidgetDropBinding::Context context = DropWidgetDropBinding::Prepare(this, event);

    //this->setChecked(context.manager->GetContainer(context.id)->GetBool());
    QStringList sp = context.id.split("::");
    this->setText(sp.back());

    DropWidgetDropBinding::Register(this, context);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(pressed()), context.manager, Qt::DirectConnection);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(released()), context.manager, Qt::DirectConnection);
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
        ToFormMapper* container = GetMainWindow()->GetLogic()->GetContainer(this);
        if (!container)
            return;

        if(container->IsBool() == 0)
            if(container->GetBool())
            {
                QTimer::singleShot(100, this, SLOT(TimeOut()));
                this->released();
            }
        if(container->IsUnsigedNumber())
            if(container->GetUnsignedData())
            {
                QTimer::singleShot(100, this, SLOT(TimeOut()));
                this->released();
            }
    }
}

void QPushButtonD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { true, false, false, false, false });
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
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(pressed()), DM, Qt::DirectConnection);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(released()), DM, Qt::DirectConnection);
}
