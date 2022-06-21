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

#include "QTableWidgeD.h"
#include "CreateID.h"
#include "../mainwindow.h"
#include "QLineEdit.h"
#include "QCheckBox.h"
#include "QLed.h"
#include <QTimer>
#include <functional>

QTableWidgeD::QTableWidgeD(QWidget *parent):QTableWidget(parent)
{
    this->setAlternatingRowColors(true);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );

    this->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this->verticalHeader(),
            SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(customHeaderMenuRequested(QPoint)));


    return;
}


void QTableWidgeD::contextMenu(QPoint pos)
{
    QMenu* menu = new QMenu(this);

    menu->addSeparator();
    menu->addAction("Clear Table", this , SLOT(RemoveConnection()));
    menu->popup(this->mapToGlobal(pos));

}

void QTableWidgeD::RemoveSelectedRows()
{
    MainWindow *MW = GetMainWindow();

    QItemSelectionModel *selections = this->selectionModel();
    QModelIndexList selected = selections->selectedRows();

    QList<int> DelRows;

    for(int ii = selected.size()-1; ii >= 0 ;ii = ii-1)
    {
        int i = selected[ii].row();
        DelRows.push_back(i);
        for(auto j = 0; j < this->columnCount(); j++)
        {
            if(this->cellWidget(i,j))
            {
                auto widgetList = this->cellWidget(i,j)->children();
                for(auto itt: widgetList)
                {
                        QObject* FoundObject = MW->findChild<QObject*>(itt->objectName());
                        if(FoundObject)
                            MW->GetLogic()->DeleteEntryOfObject(FoundObject);

                }
            }
        }
    }

    qSort(DelRows);

    for(int ii = DelRows.size()-1; ii >= 0 ;ii = ii-1)
        this->removeRow(DelRows[ii]);



}

void QTableWidgeD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();


    for(auto i = 0; i < this->rowCount();i++)
    {
        for(auto j = 0; j < this->columnCount(); j++)
        {
            if(this->cellWidget(i,j))
            {
                auto widgetList = this->cellWidget(i,j)->children();
                for(auto itt: widgetList)
                {
                        QObject* FoundObject = MW->findChild<QObject*>(itt->objectName());
                        if(FoundObject)
                            MW->GetLogic()->DeleteEntryOfObject(FoundObject);
                }
            }

        }
    }
    this->setRowCount(0);
    this->setColumnCount(0);


    MW->GetLogic()->DeleteEntryOfObject(this);
}


void QTableWidgeD::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();


    QStringList Ids = CreateIDs(event->source());
    if(Ids.size()==0)
        return;

    bool accept = false;
    for(auto itt : Ids)
    {
        if(GetMainWindow()->GetLogic()->GetContainer(itt)->IsNumeric())
            accept = true;
        else
            accept = false;

        if(!accept)
            break;
    }
    if(accept)
        event->acceptProposedAction();
}

void QTableWidgeD::customHeaderMenuRequested(QPoint pos)
{
        QItemSelectionModel *selections = this->selectionModel();
        QModelIndexList selected = selections->selectedRows();
        QMenu *menu=new QMenu(this);
        if(selected.size())
            menu->addAction("Delete Selected Rows", this, SLOT(RemoveSelectedRows()));

        menu->addAction("Clear Table", this , SLOT(RemoveConnection()));
        menu->popup(this->verticalHeader()->viewport()->mapToGlobal(pos));

}

void QTableWidgeD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QTableWidgeD::CreateRow( QString VText, QPoint Pos)
{
    auto MW = GetMainWindow();

    size_t columns = 0;
    QString ID0 = VText + "::";
    QString tmpID = ID0 + "" + QString::number(columns);
    while( MW->GetLogic()->ElementExists(tmpID))
    {
         columns++;
         tmpID = ID0 + QString::number(columns);
    }
    if(columns == 0)
        return;

    if(columns>this->columnCount())
    {
        this->setColumnCount(columns);
    }

    int r = this->rowAt((Pos).y());


    if(r == -1)
    {
        this->setRowCount( this->rowCount()+1);
        r = this->rowCount()-1;       }
    else
    {
        this->insertRow(r);
    }

    QTableWidgetItem *newheader = new QTableWidgetItem();
    newheader->setText(VText);
    this->setVerticalHeaderItem(r,newheader);

    for(auto i = 0; i < this->columnCount(); i++)
    {
        tmpID =  QString::number(i);
        newheader = new QTableWidgetItem();
        newheader->setText(tmpID);
        this->setHorizontalHeaderItem(i, newheader);
    }

    auto counter = 0;
    tmpID = ID0 + "" + QString::number(counter);

    while( MW->GetLogic()->ElementExists(tmpID))
    {
        counter++;

        QWidget *pWidget = new QWidget();

        if(GetMainWindow()->GetLogic()->GetContainer(tmpID)->IsSigedNumber() ||
           GetMainWindow()->GetLogic()->GetContainer(tmpID)->IsFloatingPointNumber() ||
           GetMainWindow()->GetLogic()->GetContainer(tmpID)->IsUnsigedNumber() )
        {
            QLineEditD *tmp = new QLineEditD();
            if(GetMainWindow()->GetLogic()->GetContainer(tmpID)->IsEditable()==0)
                tmp->setReadOnly(true);

            QHBoxLayout *pLayout = new QHBoxLayout(pWidget);
            pLayout->addWidget(tmp);
            pLayout->setAlignment(Qt::AlignCenter);
            pLayout->setContentsMargins(0,0,0,0);
            pWidget->setLayout(pLayout);
            this->setCellWidget(r,counter-1,pWidget);
            tmp->setObjectName(this->objectName() + "r" + QString::number(this->rowCount()) + "c" + QString::number(counter));
            MW->GetLogic()->AddElementToContainerEntry(tmp->objectName(),tmpID,tmp->metaObject()->className(),tmp);
            tmp->ConnectToID(MW->GetLogic(), tmpID);
            QString text = "1234678.1234";
            QFontMetrics fm(tmp->font());
            int pixelsWide = fm.width(text);
            tmp->setFixedWidth(pixelsWide);
            tmp->adjustSize();
        }
        if(GetMainWindow()->GetLogic()->GetContainer(tmpID)->IsBool())
        {
            if(GetMainWindow()->GetLogic()->GetContainer(tmpID)->IsEditable())
            {
                QCheckBoxD *tmp = new QCheckBoxD(0,0);
                QHBoxLayout *pLayout = new QHBoxLayout(pWidget);
                pLayout->addWidget(tmp);
                pLayout->setAlignment(Qt::AlignCenter);
                pLayout->setContentsMargins(0,0,0,0);
                pWidget->setLayout(pLayout);
                this->setCellWidget(r,counter-1,pWidget);
                tmp->setObjectName(this->objectName() + "r" + QString::number(this->rowCount()) + "c" + QString::number(counter));
                MW->GetLogic()->AddElementToContainerEntry(tmp->objectName(),tmpID,tmp->metaObject()->className(),tmp);
                tmp->ConnectToID(MW->GetLogic(), tmpID);
            }
            else
            {
                QLed *tmp = new QLed();
                QHBoxLayout *pLayout = new QHBoxLayout(pWidget);
                pLayout->addWidget(tmp);
                pLayout->setAlignment(Qt::AlignCenter);
                pLayout->setContentsMargins(0,0,0,0);
                pWidget->setLayout(pLayout);
                this->setCellWidget(r,counter-1,pWidget);
                tmp->setObjectName(this->objectName() + "r" + QString::number(this->rowCount()) + "c" + QString::number(counter));
                MW->GetLogic()->AddElementToContainerEntry(tmp->objectName(),tmpID,tmp->metaObject()->className(),tmp);
                tmp->ConnectToID(MW->GetLogic(), tmpID);
            }

        }

        tmpID = ID0 + QString::number(counter);
        // dot instead of comma

    }
    this->setToolTip("");
    this->resizeColumnsToContents();
    this->resizeRowsToContents();
}


void QTableWidgeD::dropEvent(QDropEvent *event)
{
    auto MW = GetMainWindow();


    QStringList IDs =  CreateIDs(event->source());
    for(auto ID : IDs)
    {
        auto IdParts = ID.split("::");
        QString ID0;
        for(auto i = 0; i< IdParts.size()-1; i++)
        {
            ID0 += IdParts[i];
            ID0 += "::";
        }

        size_t columns = 0;
        QString tmpID = ID0 + QString::number(columns);
        while( MW->GetLogic()->ElementExists(tmpID))
        {
             columns++;
             tmpID = ID0 + QString::number(columns);
        }
        if(columns == 0)
            return;

        QString VText = ID0;
        VText.remove(VText.size()-2,2);

        CreateRow(VText, event->pos());

    }


}

void QTableWidgeD::SetVariantData(ToFormMapper Data)
{
    //TODO Minmax übernehmen
    //HIer weiter
    blockSignals(true);

    if(Data.IsEditable())
    {
        auto MW = GetMainWindow();
        double value;
        if(Data.IsFloatingPointNumber())
             value = (Data.GetFloatingPointData());
        else if(Data.IsSigedNumber())
            value =  (double)Data.GetSignedData();
        else if(Data.IsUnsigedNumber())
           value =  (double)Data.GetUnsignedData();
    }
    blockSignals(false);
}

void QTableWidgeD::GetVariantData(ToFormMapper *Data)
{


}


bool QTableWidgeD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    for(auto i =0; i<Attributes.size(); i++)
        if(Attributes[i].first.contains("Connected_ID"))
        {
            QTimer::singleShot(2000, this, std::bind(&QTableWidgeD::CreateRow, this, Attributes[i].second, QPoint(-1,-1)));

        }

    return true;

}

bool QTableWidgeD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    for(auto i = this->rowCount()-1; i >=0 ; i--)
        Attributes.push_back(std::pair<QString, QString>("Connected_ID"+QString::number(i), this->verticalHeaderItem(i)->text()));

    return true;

}


void QTableWidgeD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    auto MW = GetMainWindow();
    QLocale::setDefault(QLocale::c());
    connect(this, SIGNAL(valueChanged(int)), DM, SLOT(SendNewValue()) );
    RequestUpdate();
}
