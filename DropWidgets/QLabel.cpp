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

#include "QLabel.h"
#include "CreateID.h"
#include "../mainwindow.h"


QLabelD::QLabelD(QWidget *parent):QLabel(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));

    return;
}

void QLabelD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QLabelD::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    //event->acceptProposedAction();
    if (selectedItems[0]->childCount() == 0)
        event->acceptProposedAction();
}

void QLabelD::contextMenu(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction("Edit Text", this, SLOT(EditText()));
    if(UserDefinedText)
    {
        menu->addSeparator();
        menu->addAction("Original UI-Text", this, SLOT(RemoveUserText()));
    }

    menu->popup(this->mapToGlobal(pos));
}

void QLabelD::EditText()
{
    QString input = QInputDialog::getText(this, tr("Edit"),tr("Label Text"),QLineEdit::Normal, this->text());
    if(input.size())
    this->setText(input);
      auto MW = GetMainWindow();
    MW->ChangeForSaveDetected = true;
}


void QLabelD::dropEvent(QDropEvent *event)
{
    this->disconnect();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    QString ID =  CreateID(event->source());
    this->setText(ID);
      auto MW = GetMainWindow();
    MW->ChangeForSaveDetected = true;

}

void QLabelD::SetVariantData(ToFormMapper Data)
{

}

void QLabelD::GetVariantData(ToFormMapper *Data)
{

}

bool QLabelD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    for(auto itt : Attributes)
    {
        if(itt.first ==  QString("UserDefinedText") && itt.second.toInt())
        {
            setText(Text);
        }
    }
    return true;
}

bool QLabelD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    std::pair<QString, QString> Attribut;
    Attribut.first = "UserDefinedText";
    Attribut.second = QString::number(UserDefinedText);
    Attributes.push_back(Attribut);
    if(UserDefinedText)
        Text = text();
    return true;

}

void QLabelD::ConnectToID(DataManagementSetClass* DM, QString ID)
{

}

void QLabelD::RemoveUserText()
{
    UserDefinedText = false;
    QLabel::setText(OrginialText);
}

void QLabelD::setText(const QString &text)
{
    blockSignals(true);
    if(OrginialText.isEmpty())
    {
        OrginialText = this->text();
    }
    UserDefinedText = true;
    QLabel::setText(text);
    blockSignals(false);

}
