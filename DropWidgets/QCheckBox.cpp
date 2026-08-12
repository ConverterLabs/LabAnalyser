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
#include "DropWidgetBinding.h"
#include "DropWidgetConnectionMenu.h"
#include "DropWidgetDropBinding.h"
#include "DropWidgetDragSource.h"
#include "DropWidgetIndicatorBinding.h"
#include "../mainwindow.h"

uint32_t QCheckBoxD::bitcounter = 0;
QCheckBoxD::QCheckBoxD(QWidget *parent, bool show_label):QCheckBox(parent), m_show_label(show_label)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(this, SIGNAL(RequestUpdate()));
    return;
}


void QCheckBoxD::contextMenu(QPoint pos)
{
    DropWidgetConnectionMenu::Show(this, pos, { true, false, false, false, true });
}

void QCheckBoxD::RemoveConnection()
{

    DropWidgetDropBinding::ClearConnectionPresentation(this);
    DropWidgetDropBinding::RemoveManagerBinding(this);
    this->setChecked(0);
    setText("CheckBox");
}

void QCheckBoxD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QCheckBoxD::dragEnterEvent(QDragEnterEvent *event)
{
    ToFormMapper* container = DropWidgetDragSource::ContainerForFirstSelectedLeaf(event->source());
    if (container && (container->IsBool() || container->IsUnsigedNumber()))
        event->acceptProposedAction();
}

void QCheckBoxD::dropEvent(QDropEvent *event)
{
    MainWindow* mainWindow = GetMainWindow();
    if (!mainWindow)
        return;

    QString ID =  CreateID(event->source());
    auto MW = mainWindow;

    ToFormMapper* container = MW->GetLogic()->GetContainer(ID);
    if (!container)
        return;
    DropWidgetDropBinding::ResetContextConnections(this, MW->GetLogic());
    this->setToolTip(ID);
    this->setToolTipDuration(2000);
    DropWidgetIndicatorBinding::InitializeState(this, container, bit, bitcounter,
                                                 tr("Index of Bit to be set"),
                                                 tr("Index of Bit to be set (zero based):"),
                                                 [this](bool state) { setChecked(state); });
    QStringList sp = ID.split("::");
    QString label = sp.back();
    label += ":";
    label += QString::number(this->bit);
    if(m_show_label == false)
        label.clear();

    this->setText(label);

    MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(clicked(bool)), MW->GetLogic());
    MW->ChangeForSaveDetected = true;
}

void QCheckBoxD::SetVariantData(ToFormMapper Data)
{
    //DO NOT BLOCK SIGNALS !!!
    auto MW = GetMainWindow();
    if (!MW)
        return;
    disconnect(this, SIGNAL(clicked(bool)), MW->GetLogic(),SLOT(SendNewValue()));
    if(Data.IsBool())
    {
        setChecked(Data.GetBool());
        clicked(Data.GetBool());

    }
    else if(Data.IsUnsigedNumber())
    {

        setChecked((bool) (Data.GetUnsignedData() & (1ULL<<GetBit())));
        clicked(Data.GetUnsignedData() & (1ULL<<GetBit()));

    }
     repaint();
     DropWidgetBinding::ConnectValueChanged(this, SIGNAL(clicked(bool)), MW->GetLogic());

}

void QCheckBoxD::GetVariantData(ToFormMapper *Data)
{
    if (!Data)
        return;

    if(Data->IsBool())
    {
        Data->SetDataKeepType(isChecked());
    }
    else if(Data->IsUnsigedNumber())
    {
        uint64_t numbr =Data->GetUnsignedData();
        if(isChecked())
           Data->SetDataKeepType( (numbr | 1ULL<<GetBit()));
        else
         Data->SetDataKeepType( (numbr & ~(1ULL<<GetBit())));
    }
}

bool QCheckBoxD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    for(auto itt: Attributes)
    {
        if(itt.first ==  QString("Bit"))
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
    if(m_show_label == false)
        label.clear();
    setText(label);
    DropWidgetBinding::ConnectValueChanged(this, SIGNAL(clicked(bool)), DM);
    RequestUpdate();
}
