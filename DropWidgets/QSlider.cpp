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

#include "QSlider.h"
#include "CreateID.h"
#include "../mainwindow.h"




QSliderD::QSliderD(QWidget *parent):QSlider(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );
    this->setMaximum(0);
    this->setMaximum(100);
      return;
}


void QSliderD::contextMenu(QPoint pos)
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

void QSliderD::RemoveConnection()
{

    this->setToolTip("");
    this->setToolTipDuration(0);
    MainWindow *MW = GetMainWindow();
    MW->GetLogic()->DeleteEntryOfObject(this);
    this->setValue(0);
}


void QSliderD::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget)
        return;
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    if (selectedItems[0]->childCount() == 0)
    {
        QString ID = CreateID(event->source());
        QString Type = GetMainWindow()->GetLogic()->GetContainer(ID)->GetDataType();
        if(GetMainWindow()->GetLogic()->GetContainer(ID)->IsFloatingPointNumber() ||
           GetMainWindow()->GetLogic()->GetContainer(ID)->IsUnsigedNumber() ||
           GetMainWindow()->GetLogic()->GetContainer(ID)->IsSigedNumber())
                event->acceptProposedAction();
    }
}

void QSliderD::dragMoveEvent(QDragMoveEvent *de)
{
    de->accept();
}

void QSliderD::dropEvent(QDropEvent *event)
{
               this->disconnect();
                connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
                connect(this, SIGNAL(RequestUpdate()), GetMainWindow()->GetLogic(), SLOT(UpdateRequest()) );


               QString ID =  CreateID(event->source());
               this->setToolTip(ID);
               this->setToolTipDuration(2000);
               auto MW = GetMainWindow();
               MW->GetLogic()->DeleteEntryOfObject(this);


               // dot instead of comma
               QLocale::setDefault(QLocale::c());

               MinMax = MW->GetLogic()->MinMaxValue(ID);
               double MinValue = MinMax.first;
               double MaxValue = MinMax.second;
               if(MinMax.first == MinMax.second)
               {
                    MinValue = QInputDialog::getDouble(this, tr("Minimal Value of"),
                                                        ID, 0,-2147483647, 2147483647,5);
                    MaxValue = QInputDialog::getDouble(this, tr("Maximum Value of"),
                                                        ID, 0,-2147483647, 2147483647,5);
                   MW->GetLogic()->SetMinMaxValue(ID,MinValue,MaxValue);
               }

               MinMax = MW->GetLogic()->MinMaxValue(ID);
               MW->GetLogic()->AddElementToContainerEntry(this->objectName(),ID,this->metaObject()->className(),this);
               connect(this, SIGNAL(valueChanged(int)), MW->GetLogic(),SLOT(SendNewValue()) );
               MW->ChangeForSaveDetected = true;
               ConnectedID = ID;
               emit RequestUpdate();



}

void QSliderD::SetVariantData(ToFormMapper Data)
{
    //TODO Minmax übernehmen
    //HIer weiter
    if(Data.IsEditable())
    {
        auto MW = GetMainWindow();
        if(ConnectedID.size())
            if(MW->GetLogic()->ElementExists(ConnectedID))
                MinMax = MW->GetLogic()->MinMaxValue(ConnectedID);

        double value;
        if(Data.IsFloatingPointNumber())
             value = (Data.GetFloatingPointData());
        else if(Data.IsSigedNumber())
            value =  (double)Data.GetSignedData();
        else if(Data.IsUnsigedNumber())
           value =  (double)Data.GetUnsignedData();
        int valueC = (int) round((value - MinMax.first)/(MinMax.second-MinMax.first)*100.0);
        setValue((int)valueC );
    }



}

void QSliderD::GetVariantData(ToFormMapper *Data)
{
     int valuei = value();
     std::pair<double, double> MinMax = std::pair<double, double>(Data->MinValue,Data->MaxValue);
     double ValueScaled = ((double) valuei)/100.0 * (MinMax.second - MinMax.first) + MinMax.first;

     Data->SetDataKeepType(ValueScaled);
}


bool QSliderD::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    return false;

}

bool QSliderD::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    return false;

}


void QSliderD::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    setToolTip(ID);
    auto MW = GetMainWindow();
    QLocale::setDefault(QLocale::c());
    ConnectedID = ID;
    MinMax = MW->GetLogic()->MinMaxValue(ID);
    connect(this, SIGNAL(valueChanged(int)), DM, SLOT(SendNewValue()) );
    RequestUpdate();
}
