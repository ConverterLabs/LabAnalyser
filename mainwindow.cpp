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

#include <QTime>
#include <QList>
#include <QPointer>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qstringlist.h"
#include  "DropWidgets/Plots/PlotWidget.h"
#include "UIFunctions/SubPlotMainWindow.h"
#include "UIFunctions/MainWindowOutputLog.h"
#include "UIFunctions/MainWindowTreePath.h"
#include "UIFunctions/MainWindowTreeViewState.h"
#include "UIFunctions/MainWindowSubplotDialog.h"
#include "UIFunctions/MainWindowFormLoader.h"

#include "DropWidgets/DropWidgets.h"
#include "DropWidgets/DropWidgetsUiLoader.h"
#include <QFileInfo>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Set the object name for the main form
    this->setObjectName("LabAnalyser");

    // Create tray icon
    icon = new QSystemTrayIcon(QIcon(":/icons/sym.png"), this);

    // Create menu for tray icon
    auto menu = new QMenu(this);

    // Create restore action and disable it initially
    restore = new QAction("Restore",this);
    restore->setEnabled(false);

    // Connect the restore action's triggered signal to the showNormal slot of the main form
    connect(restore, SIGNAL(triggered()), this, SLOT(showNormal()));

    // Add the restore action to the menu
    menu->addAction(restore);

    // Add a separator to the menu
    menu->addSeparator();

    // Create quit action
    auto quitAction = new QAction(QIcon(":/icons/icons/Exit.png"), "Quit",this);

    // Connect the quit action's triggered signal to the close slot of the main form
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    // Add the quit action to the menu
    menu->addAction(quitAction);

    // Set the tray icon's context menu to be the menu created above
    icon->setContextMenu(menu);

    // Connect the tray icon's activated signal to the
//

    connect(this->icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(TrayIconActivated(QSystemTrayIcon::ActivationReason)));
    icon->show();
    //End Tray Icon


    //Userinterface
    ui->setupUi(this);

    //Dock Widgets
    addDockWidget(Qt::RightDockWidgetArea, ui->ParameterDock);
    addDockWidget(Qt::RightDockWidgetArea, ui->DataDock);
    addDockWidget(Qt::RightDockWidgetArea, ui->StateDock);
    addDockWidget(Qt::RightDockWidgetArea, ui->OutputDock);


    //combine the dock widgets
    QMainWindow::tabifyDockWidget(ui->ParameterDock,ui->DataDock);
    QMainWindow::tabifyDockWidget(ui->ParameterDock,ui->StateDock);
    QMainWindow::tabifyDockWidget(ui->ParameterDock,ui->OutputDock);

    //raise Parameter Dock as standard
    qobject_cast<QWidget*>( ui->ParameterDock)->raise();

    ui->ParameterDock->setMaximumWidth(600);
    ui->DataDock->setMaximumWidth(600);
    ui->OutputDock->setMaximumWidth(600);
    ui->StateDock->setMaximumWidth(600);

    ui->ParameterDock->setSizePolicy(QSizePolicy::Policy::Fixed,QSizePolicy::Policy::Minimum);
    ui->DataDock->setSizePolicy(QSizePolicy::Policy::Fixed,QSizePolicy::Policy::Minimum);
    ui->OutputDock->setSizePolicy(QSizePolicy::Policy::Fixed,QSizePolicy::Policy::Minimum);
    ui->StateDock->setSizePolicy(QSizePolicy::Policy::Fixed,QSizePolicy::Policy::Minimum);

    ui->ParameterDock->installEventFilter(this);
    ui->DataDock->installEventFilter(this);

    connect(ui->ParameterDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockWidget_topLevelChanged(bool)));
    connect(ui->DataDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockWidget_topLevelChanged(bool)));
    connect(ui->OutputDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockWidget_topLevelChanged(bool)));
    connect(ui->StateDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockWidget_topLevelChanged(bool)));

    //Set Parameter Dock Header
    ui->ParameterTreeWidget->headerItem()->setText(0, "Item");
    ui->ParameterTreeWidget->headerItem()->setText(1, "Data Type");


    ui->DataTreeWidget->headerItem()->setText(0, "Item");
    ui->DataTreeWidget->headerItem()->setText(1, "Data Type");

    ui->DataTreeWidget->setAlternatingRowColors(true);
    ui->ParameterTreeWidget->setAlternatingRowColors(true);
    ui->StateTreeWidget->setAlternatingRowColors(true);



    //Parameter Widget should have Context Menu for Min/Max Value editing
    ui->ParameterTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ParameterTreeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuTreeWidget(QPoint)));


    //StateTreeWidget Widget should have Context Menu for Min/Max Value editing
    ui->StateTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
   // connect(ui->StateTreeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuTreeWidgetState(QPoint)));

    ui->DataTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->DataTreeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuTreeWidgetData(QPoint)));

    this->StdSavePath = QDir::currentPath();

    //create the GUI Logic Object
    ExtendedDataManagement = new UIDataManagementSetClass(this);
//    ExtendedDataManagement->SetMessenger(Messenger);

    Remote = new RemoteControlServer(this->GetLogic()->GetContainerPointer());
    connect(Remote, SIGNAL(MessageSender(QString,QString,InterfaceData)),this->ExtendedDataManagement->GetMessenger(),SLOT(MessageTransmitter(QString,QString,InterfaceData)));


    ui->OutputText->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( ui->OutputText, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( OutputTextMenu( QPoint ) ) );
    //ui->OutputText->setWordWrapMode(QTextOption::WrapAnywhere);
    ui->OutputText->setMaximumBlockCount(100);
    OutputLog.reset(new MainWindowOutputLog(ui->OutputText, ui->OutputDock));
    TreeViewState.reset(new MainWindowTreeViewState(ui->ParameterTreeWidget, ui->DataTreeWidget,
        ui->StateTreeWidget, ui->ParameterDock, ui->DataDock, ui->StateDock));

    ParseInputArguments();

}


void MainWindow::OutputTextMenu( QPoint p )
{
    // Start with the standard menu.
     QMenu * pMenu = ui->OutputText->createStandardContextMenu();
     QAction * pAction;

     pMenu->addSeparator();
     pAction = new QAction( "Clear Output", pMenu );
     connect( pAction, SIGNAL( triggered() ), ui->OutputText, SLOT( clear() ) );
     pMenu->addAction( pAction );

     // Show the menu.
     QPoint q = ui->OutputText->mapToGlobal( p );
     pMenu->exec( q );

     delete pMenu;
}


//Resize of the Main Window shell rezisze the dock widget columns as well
void MainWindow::resizeEvent(QResizeEvent *event)
{
    //Resize the dockables when window resizes
    QMainWindow::resizeEvent(event);
    int Width = ui->ParameterDock->width();
    ui->ParameterTreeWidget->setColumnWidth(0,Width*0.6);
    ui->ParameterTreeWidget->setColumnWidth(1,Width*0.2);
    ui->ParameterTreeWidget->setColumnWidth(2,Width*0.1);
}


//This function is connected to the contextMenuRequest of the DockWidgets
void MainWindow::contextMenuTreeWidget(QPoint pos)
{
    //Add a Min Max Context Menu when an element without childs is selected
    //the tree repesents always the ID e.g ID Parameter::One --> Parameter -> one
    QList<QTreeWidgetItem*> selectedItems = ui->ParameterTreeWidget->selectedItems();
    if(selectedItems.size() == 1)
    {
        if (selectedItems[0]->childCount() == 0)
        {
            QMenu *menu = new QMenu(this);
            menu->setAttribute(Qt::WA_DeleteOnClose);
            menu->addAction("Change Min/Max Values", this, SLOT(ChangeMinMaxValue()));
            menu->popup(ui->ParameterDock->mapToGlobal(pos));
        }
        else if(!(selectedItems[0]->parent()))
        {

            //this is top level, we can remove the device here
            QMenu *menu = new QMenu(this);
            menu->setObjectName(selectedItems[0]->text(0));
            menu->setAttribute(Qt::WA_DeleteOnClose);
            menu->addAction("Remove Device", this, SLOT(RemoveDevice()));
            menu->popup(ui->ParameterDock->mapToGlobal(pos));
        }
    }
}

void MainWindow::contextMenuTreeWidgetState(QPoint pos)
{
    //Add a Min Max Context Menu when an element without childs is selected
    //the tree repesents always the ID e.g ID Parameter::One --> Parameter -> one
    QList<QTreeWidgetItem*> selectedItems = ui->StateTreeWidget->selectedItems();
    if(selectedItems.size() == 1)
    {
        if(!(selectedItems[0]->parent()))
        {
            //this is top level, we can remove the device here
            QMenu *menu = new QMenu(this);
            menu->setObjectName(selectedItems[0]->text(0));
            menu->setAttribute(Qt::WA_DeleteOnClose);
            menu->addAction("Remove Device", this, SLOT(RemoveDevice()));
            menu->popup(ui->ParameterDock->mapToGlobal(pos));
        }
    }
}


//This function is connected to the contextMenuRequest of the DockWidgets
void MainWindow::contextMenuTreeWidgetData(QPoint pos)
{
    //Add a Min Max Context Menu when an element without childs is selected
    //the tree repesents always the ID e.g ID Parameter::One --> Parameter -> one
    QList<QTreeWidgetItem*> selectedItems = ui->DataTreeWidget->selectedItems();
    if(selectedItems.size())
    {
        if (selectedItems[0]->childCount() == 0)
        {
            QMenu *menu = new QMenu(this);
            menu->setAttribute(Qt::WA_DeleteOnClose);

            QList<QTreeWidgetItem*> items = selectedItems;
            QString Ids;
            for(int i = 0; i < items.size(); i++)
            {
                if(items.at(i)->childCount()==0)
                {
                    Ids.push_back(MainWindowTreePath::IdForItem(items.at(i)));
                }
            }
            QAction *SetAliasAction = new QAction(menu);
                connect(SetAliasAction, &QAction::triggered, [=]{
                    SetAlias(GetLogic()->GetAlias(Ids));});

            QAction *RemoveAliasAction = new QAction(menu);
                connect(RemoveAliasAction, &QAction::triggered, [=]{
                    RemoveAlias(Ids);});

            SetAliasAction->setText("Set Alias");
            RemoveAliasAction->setText("Remove Alias");
            menu->addAction( SetAliasAction);

            if(this->GetLogic()->GetAlias(Ids).compare(Ids))
                menu->addAction( RemoveAliasAction);

            menu->popup(ui->DataTreeWidget->mapToGlobal(pos));
        }
        else if(!(selectedItems[0]->parent()))
        {
            //this is top level, we can remove the device here
            QMenu *menu = new QMenu(this);
            menu->setObjectName(selectedItems[0]->text(0));

            menu->setAttribute(Qt::WA_DeleteOnClose);
            menu->addAction("Remove Device", this, SLOT(RemoveDevice()));
            menu->popup(ui->DataTreeWidget->mapToGlobal(pos));
        }
    }
}

void MainWindow::RemoveAlias(QString ID)
{
    this->GetLogic()->SetAlias(ID, ID);
}

void MainWindow::SetAlias(QString ID)
{
    bool ok;
    QString Alias = QInputDialog::getText(this, tr("New Alias"),
                                             tr("Alias:"), QLineEdit::Normal,
                                             ID, &ok);
    if (ok && !Alias.isEmpty())
        this->GetLogic()->SetAlias(ID, Alias);
}

void MainWindow::ChangeMinMaxValue()
{
      //Change Min Max value for the selected Element (needed for filder and spinbox
      QList<QTreeWidgetItem*> selectedItems = ui->ParameterTreeWidget->selectedItems();
      if (selectedItems.size() != 1 || !selectedItems[0] || selectedItems[0]->childCount() != 0)
          return;
      const QString ID = MainWindowTreePath::IdForItem(selectedItems[0]);
      if (!GetLogic()->GetContainer(ID))
          return;
      std::pair<double,double> MinMax = GetLogic()->MinMaxValue(ID);
      double MinValue = MinMax.first;
      double MaxValue = MinMax.second;
      QLocale::setDefault(QLocale::c());
      MinValue = QInputDialog::getDouble(this, tr("Minimal Value of"),
                                           ID, MinValue);
      MaxValue = QInputDialog::getDouble(this, tr("Maximum Value of"),
                                           ID, MaxValue);
      GetLogic()->SetMinMaxValue(ID,MinValue,MaxValue);
      ExtendedDataManagement->UpdateRequest(ID);

}

void MainWindow::RemoveDevice( )
{
    auto sender = QObject::sender();
    if (!sender || !sender->parent())
        return;
    auto ID = sender->parent()->objectName();
    if (ID.isEmpty())
        return;
    qDebug() << sender->objectName();




      //Löschen der Einträge
      for(int i = this->ui->ParameterTreeWidget->topLevelItemCount()-1; i >= 0; i-- )
      {
          if(this->ui->ParameterTreeWidget->topLevelItem(i)->text(0).compare(ID) == 0)
              delete this->ui->ParameterTreeWidget->topLevelItem(i);
      }
      for(int i = this->ui->DataTreeWidget->topLevelItemCount()-1; i >= 0; i-- )
      {
          if(this->ui->DataTreeWidget->topLevelItem(i)->text(0).compare(ID) == 0)
              delete this->ui->DataTreeWidget->topLevelItem(i);
      }
      for(int i = this->ui->StateTreeWidget->topLevelItemCount()-1; i >= 0; i-- )
      {
          if(this->ui->StateTreeWidget->topLevelItem(i)->text(0).compare(ID) == 0)
              delete this->ui->StateTreeWidget->topLevelItem(i);
      }

      ExtendedDataManagement->CloseDevice(ID);

}

void MainWindow::closeEvent ( QCloseEvent * event )
{   //Clear the Logic and the plots when main window closes
    if(this->GetLogic()->GetFormFileCount())
    {
        auto answer = QMessageBox::Discard;
        if(this->ChangeForSaveDetected)
            answer =  QMessageBox::question(this, "Exit", "Do you want to save the Project?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (QMessageBox::Save == answer)
        {
            auto UiFileName = QFileDialog::getSaveFileName(this,
                                                           tr("Save Experiment"), this->StdSavePath, tr("Expermiment Files (*.LAexp)"));
            ExtendedDataManagement->SaveExperiment(UiFileName);
            this->GetLogic()->RemoveDevices();
            this->SavePath.clear();
            event->accept();
        }
        if (QMessageBox::Discard == answer)
        {
            this->GetLogic()->RemoveDevices();
            this->SavePath.clear();
            event->accept();
        }
        else if (QMessageBox::Cancel == answer)
            event->ignore();
    }
    else
    {
        this->GetLogic()->RemoveDevices();
        this->SavePath.clear();
        event->accept();
    }
}

MainWindow::~MainWindow()
{
    delete Remote;
    TreeViewState.reset();
    OutputLog.reset();
    delete ui;
}

void MainWindow::on_actionLoad_Form_triggered()
{
    auto UiFileName = QFileDialog::getOpenFileName(this,
             tr("Open UI File"), this->StdSavePath, tr("UI Files (*.ui)"));
    QFileInfo fi(UiFileName);
    this->StdSavePath = fi.absolutePath();

    if(UiFileName.size())
        this->LoadFormFromXML(UiFileName);
}

void MainWindow::Info(QString text)
{
    InterfaceData Data;
    Data.SetData(text);
    GetLogic()->GetMessenger()->MessageReceiver("info", this->objectName(), Data);
}

void MainWindow::Error(QString text)
{
    InterfaceData Data;
    Data.SetData(text);
    GetLogic()->GetMessenger()->MessageReceiver("error", this->objectName(), Data);
}

void MainWindow::on_actionBeenden_triggered()
{
    this->close();
}

void MainWindow::on_actionCreatePlot_triggered()
{
      CreateSubPlotWindow(1,1);
}

void MainWindow::on_actionFFT_triggered()
{
    CreateFFTPlotWindow();
}



void MainWindow::DeleteFigure(SubPlotMainWindow* FigurePointer)
{
    if (!FigurePointer)
        return;
    this->GetLogic()->DeletePlotWindow(FigurePointer->objectName());
}

void MainWindow::on_actionCreate_Subplot_triggered()
{
    int rows = 0;
    int columns = 0;
    if (MainWindowSubplotDialog::SelectDimensions(rows, columns))
        CreateSubPlotWindow(rows, columns);
}

void MainWindow::CreateFFTPlotWindow()
{
    CreateSubPlotWindow(1,1, true);
}

SubPlotMainWindow* MainWindow::CreateSubPlotWindow(int rows, int cols, bool IsFFTPlot)
{
    //Create New Window
    SubPlotMainWindow* MW = new SubPlotMainWindow(this,this);

    QWidget* NW  = new QWidget();
    QGridLayout *gridLayout1 = new QGridLayout(NW);
    gridLayout1->setSpacing(0);
    gridLayout1->setContentsMargins(0, 0, 0, 0);
    gridLayout1->setObjectName(QStringLiteral("gridLayout"));
    //Add the plot widgets
    for(int i = 0; i <  rows; i++ )
    {
        for(int j = 0; j <  cols; j++ )
        {
            PlotWidget *PW = new PlotWidget(this, NW, MW->GetStatusBar(), IsFFTPlot);
            //Create unique name
            int Number = GetLogic()->GetUniquePlotNumber();
            QString PlotName;          
            PlotName = "Plot#";
            PlotName.append(QString::number( Number + 1));
            //Set Name
            PW->setObjectName(PlotName);
            //Add the plot to the logic map
            GetLogic()->AddPlotPointer(PlotName, qobject_cast<QObject*>(PW),Number);
            //add plot widget to layout
            gridLayout1->addWidget(PW,i,j,1,1);
        }
    }

    MW->setCentralWidget(NW);
    MW->resize(600,400);

    //Create unique name
    QString FigureName("Figure#");
    auto Wnumber = GetLogic()->GetPlotWindowsIncrementer();
    FigureName.append(QString::number( Wnumber ));
    MW->setObjectName(FigureName);
    this->GetLogic()->AddPlotWindow(MW->objectName(),rows,cols, Wnumber);
    QString FigureTitle("Figure ");
    FigureTitle.append(QString::number( Wnumber + 1));
    MW->setWindowTitle(FigureTitle);
    //Save SubplotWindow and rows and cols to be able to save them
    MW->show();
    return MW;
}

void MainWindow::LoadFormFromXML(QString Path)
{
    LoadFormFromXML(Path, QString());
}

void MainWindow::LoadFormFromXML(QString UiFileName, QString LastFormName, bool skip)
{
    MainWindowFormLoader::Load(*this, UiFileName, LastFormName, skip);
}

void MainWindow::on_actionLoadPlugin_triggered()
{

    QString FileName = QFileDialog::getOpenFileName(this, tr("Load Plugin/Device File"), this->StdSavePath, tr("Plugin/Device Files (*.LAdev)"));
    if(!FileName.size())
        return;
    QFileInfo fi(FileName);
    this->StdSavePath = fi.absolutePath();

    GetLogic()->LoadPlugin(FileName);
}


 void MainWindow::AddElementToWidget(QString ID, InterfaceData Data)
 {
    QStringList Parts = ID.split("::");
    TreeWidgetItem *CurrentItem = nullptr;
    QTreeWidget *SelTreeWidget = nullptr;

    if(Data.GetType().compare("Parameter")==0)
    {
        SelTreeWidget = ui->ParameterTreeWidget;
    }
    else if(Data.GetType().compare("Data")==0)
    {
        SelTreeWidget = ui->DataTreeWidget;
    }
    else if(Data.GetType().compare("State")==0)
    {
        SelTreeWidget = ui->StateTreeWidget;
    }
        for(int j = 0; j < Parts.size() && SelTreeWidget;j++)
        {
            int topCount =  SelTreeWidget->topLevelItemCount();

            //Check element already exists
            if(j == 0)
            {
                for (int i = 0; i < topCount; i++)
                {
                    QTreeWidgetItem *item = SelTreeWidget->topLevelItem(i);
                    if(item->text(0).compare(Parts[j])==0)
                    {
                       CurrentItem = (TreeWidgetItem*)(item);
                    }                    
                }
                if(!CurrentItem)
                {
                    CurrentItem = new TreeWidgetItem;
                    CurrentItem->setText(0,Parts[j]);
                    SelTreeWidget->addTopLevelItem(CurrentItem);
                }
            }
            else
            {
                bool ChildFound = 0;
                for (int i = 0; i < CurrentItem->childCount() && !ChildFound; i++)
                {
                    if(CurrentItem->child(i)->text(0).compare(Parts[j]) == 0)
                    {
                        CurrentItem = (TreeWidgetItem*)CurrentItem->child(i);
                        ChildFound = true;
                    }
                }
                if(!ChildFound)
                {
                    TreeWidgetItem *NewChild = new TreeWidgetItem;
                    NewChild->setText(0,Parts[j]);
                    if(j == Parts.size()-1)
                    {
                         NewChild->setText(1,Data.GetDataType());
                         NewChild->setText(2,Data.GetStateDependency());
                    }
                    CurrentItem->addChild(NewChild);
                    CurrentItem = NewChild;
                }
                else
                {
                    if(j == Parts.size()-1)
                    {
                         CurrentItem->setText(1,Data.GetDataType());
                         CurrentItem->setText(2,Data.GetStateDependency());
                    }
                }
            }
        }      
 }

 void MainWindow::PublishStart()
 {
     TreeViewState->BeginPublish();
 }

 void MainWindow::PublishFinished()
 {
     TreeViewState->EndPublish();
 }

 void MainWindow::RemoveElementFromWidget(QString ID)
 {
    QStringList Parts = ID.split("::");
    QTreeWidgetItem *CurrentItem = nullptr;
    QTreeWidgetItem *LastItem = nullptr;
    QTreeWidget *SelTreeWidget = nullptr;

    for(int r = 0; r < 3 ; r++)
    {
        LastItem = nullptr;
        if(r== 0)
            SelTreeWidget = ui->ParameterTreeWidget;
        else if (r == 1)
            SelTreeWidget = ui->DataTreeWidget;
        else if(r==2)
            SelTreeWidget = ui->StateTreeWidget;

        int found = 0;
        CurrentItem = nullptr;
        for(int j = 0; j < Parts.size() && SelTreeWidget;j++)
        {
            if(!CurrentItem)
            {
                int topCount =  SelTreeWidget->topLevelItemCount();
                for (int i = 0; i < topCount; i++)
                {
                    QTreeWidgetItem *item = SelTreeWidget->topLevelItem(i);
                    if(item->text(0).compare(Parts[j])==0)
                    {
                        CurrentItem = item;
                        found++;
                    }
                }
            }
            else
            {
                for (int i = 0; i < CurrentItem->childCount(); i++)
                {
                    if(CurrentItem->child(i)->text(0).compare(Parts[j]) == 0)
                    {
                        LastItem  = CurrentItem;
                        CurrentItem = CurrentItem->child(i);
                        found++;
                    }
                }
            }
        }
        if(found == Parts.size() )
        {
            if(LastItem)
            {
                LastItem->removeChild(CurrentItem);
            }
            else
                SelTreeWidget->removeItemWidget(CurrentItem,0);

            delete CurrentItem;
        }
    }

 }

void MainWindow::on_actionSave_Experiment_triggered()
{
    QString Path = QFileDialog::getSaveFileName(this,
             tr("Save Experiment"), this->StdSavePath, tr("Expermiment Files (*.LAexp)"));

    QFileInfo fi(Path);
    this->StdSavePath = fi.absolutePath();

    emit SaveExperiment(Path);

    /*ExtendedDataManagement->SaveExperiment(Path);
    this->SavePath = Path;
    */
    ChangeForSaveDetected = false;

}

void MainWindow::on_actionLoadExperiment_triggered()
{

    if( this->ChangeForSaveDetected )
    {
        auto answer = QMessageBox::Discard;
        if(this->ChangeForSaveDetected)
          answer =  QMessageBox::question(this, "Load other Project", "Save the actual Project?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (QMessageBox::Save == answer)
        {
            auto UiFileName = QFileDialog::getSaveFileName(this,
                                                           tr("Save Experiment"), this->StdSavePath, tr("Expermiment Files (*.LAexp)"));
            QFileInfo fi(UiFileName);
            this->StdSavePath = fi.absolutePath();

            GetLogic()->SaveExperiment(UiFileName);

        }
        if (QMessageBox::Discard == answer)
        {

        }
        else if (QMessageBox::Cancel == answer)
            return;
    }

    auto Path = QFileDialog::getOpenFileName(this,
             tr("Load Experiment"), this->StdSavePath, tr("Expermiment Files (*.LAexp)"));

    if(!Path.size())
        return;

    CloseProject();

    QFileInfo fi(Path);
    this->StdSavePath = fi.absolutePath();
    this->isloading = true;
    if(!ExtendedDataManagement->LoadExperiment(Path))
        this->SavePath = Path;
    this->isloading = false;



}

QStatusBar* MainWindow::GetStatusBar()
{
       return this->ui->statusBar;
}

void MainWindow::CloseProject(void)
{
    if(this->isloading)
    {
        QTimer::singleShot(100, this, SLOT(CloseProject()));
        return;

    }

      auto cti = (this->findChildren<SubPlotMainWindow*>());
      for(SubPlotMainWindow* itt : cti)
          itt->close();

      while(this->GetLogic()->GetFormFileCount())
      {
        const QString formName = this->GetLogic()->GetFormFileEntry(0).first;
        QDockWidget* DW = this->findChild<QDockWidget*>(formName);
        if(DW)
        {
            QPointer<QDockWidget> dock(DW);
            DW->close();
            if (dock)
                delete dock;
        }
        else
            this->GetLogic()->RemoveFormFile(formName);
      }

      while (this->ui->ParameterTreeWidget->topLevelItemCount())
      {
          delete this->ui->ParameterTreeWidget->topLevelItem(0);
      }
      while (this->ui->DataTreeWidget->topLevelItemCount())
      {
          delete this->ui->DataTreeWidget->topLevelItem(0);
      }
      while (this->ui->StateTreeWidget->topLevelItemCount())
      {
          delete this->ui->StateTreeWidget->topLevelItem(0);
      }
      this->GetLogic()->CloseProjectLogic();
      this->SavePath.clear();
      this->ChangeForSaveDetected = false;
      //Todo ChangeForSaveDetected
}

void MainWindow::on_Close_Project_triggered()
{
    if(this->GetLogic()->GetFormFileCount())
    {
        auto answer = QMessageBox::Discard;
        if(this->ChangeForSaveDetected)
            answer =  QMessageBox::question(this, "Close Project", "Do you want to save the Project?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (QMessageBox::Save == answer)
        {
            auto UiFileName = QFileDialog::getSaveFileName(this,
                                                           tr("Save Experiment"), this->StdSavePath, tr("Expermiment Files (*.LAexp)"));
            ExtendedDataManagement->SaveExperiment(UiFileName);
            CloseProject();
        }
        if (QMessageBox::Discard == answer)
        {
            CloseProject();
        }
    }
    else
         CloseProject();

    ChangeForSaveDetected = false;
}

void MainWindow::AddSelectedItems(QTreeWidgetItem* elemtent, QStringList &itt)
{
    if (!elemtent)
        return;
    for(int c = 0; c<elemtent->childCount(); c++)
    {
        AddSelectedItems(elemtent->child(c), itt);
    }
    const QString ID = MainWindowTreePath::IdForItem(elemtent);
    if(this->GetLogic()->GetContainer(ID))
    {
        itt.push_back(ID);
    }
    itt.removeDuplicates();
}

void MainWindow::SelectedItems(QStringList &Ids, QList<QTreeWidgetItem*> selit)
{
    QList<QTreeWidgetItem*> selectedItems = selit;
   //Durch alle elemente duchitterieren bis ebende null und IDs speichern, wenn child = 0;
    for(auto si : selectedItems)
        AddSelectedItems(si, Ids);


}
//Todo: export umbauen
void MainWindow::on_actionDaten_Exportieren_mat_triggered()
{
    QStringList Ids;
    SelectedItems(Ids, this->ui->ParameterTreeWidget->selectedItems());
    SelectedItems(Ids, this->ui->DataTreeWidget->selectedItems());

    if(!Ids.size())
    {
        Error("Please select the Data in the Explorer that shell be exported.");
        return;
    }

    auto UiFileName = QFileDialog::getSaveFileName(this,
                                                   tr("Export Data to *.mat"), this->StdSavePath, tr("Mat Files (*.mat)"));
    QFileInfo fi(UiFileName);
    this->StdSavePath = fi.absolutePath();

    if(UiFileName.size())
    {
       GetLogic()->Export2Mat(UiFileName,Ids);
    }

return;

}

void MainWindow::on_actionSave_triggered()
{
    if(this->SavePath.size())
    {
        ExtendedDataManagement->SaveExperiment(this->SavePath);
        ChangeForSaveDetected = false;
    }
    else
    {
        on_actionSave_Experiment_triggered();
    }

}

void MainWindow::dockWidget_destroyed(QObject* Sen)
{
    if (!Sen)
        return;
    QObject *SenderOC = Sen;
    auto cti = (SenderOC->findChildren<PlotWidget*>());
    for(int i = 0; i < cti.size(); i++)
    {
         PlotWidget* PW = qobject_cast<PlotWidget*> (cti[i]);
         if(PW)
             PW->ClearAllGraphs();
    }

    //Remove all connections
    auto Ch = SenderOC->findChildren<QWidget*>();
    for(int i = 0; i < Ch.size(); i++)
    {
        this->GetLogic()->DeleteEntryOfObject(Ch[i]);
    }
    //ui->tabWidget->removeTab(number);
    this->GetLogic()->RemoveFormFile(SenderOC->objectName());

    if(!this->GetLogic()->GetFormFileCount())
        this->ui->centralWidget->setHidden(false);
}

//this Function is used to change the WindowFlags, to make the Window maximizable
void MainWindow::dockWidget_topLevelChanged(bool isFloating){

    QDockWidget *SenderOC = qobject_cast<QDockWidget*>(QObject::sender());
    if (!SenderOC)
        return;

    if(isFloating)
    {
        SenderOC->setMaximumWidth(16555);
        SenderOC->setMaximumHeight(16555);
        SenderOC->setWindowFlags(Qt::Window);
        // setWindowFlags calls setParent() when changing the flags for a window, causing the widget to be hidden.
        // You must call show() to make the widget visible again

        if(SenderOC->objectName().compare("ParameterDock") &&
                SenderOC->objectName().compare("StateDock") &&
                SenderOC->objectName().compare("DataDock")&&
                SenderOC->objectName().compare("OutputDock"))
        {
            SenderOC->setWindowFlags(SenderOC->windowFlags() | Qt::CustomizeWindowHint |
                                           Qt::WindowMinimizeButtonHint |
                                           Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
        }
        else
        {
            SenderOC->setWindowFlags((SenderOC->windowFlags() | Qt::CustomizeWindowHint |
                                      Qt::WindowMinimizeButtonHint |
                                      Qt::WindowMaximizeButtonHint)& ~Qt::WindowCloseButtonHint );
        }
        SenderOC->show();
    }
    else
    {
        this->ui->centralWidget->hide();
        if(SenderOC->objectName().compare("ParameterDock") == 0 ||
                SenderOC->objectName().compare("ParameterDock") == 0 ||
                SenderOC->objectName().compare("DataDock") == 0 ||
                SenderOC->objectName().compare("OutputDock") == 0 )
        {
            if(this->dockWidgetArea(SenderOC) == Qt::BottomDockWidgetArea)
            {
                SenderOC->setMaximumWidth(16555);
                SenderOC->setMaximumHeight(300);
            }
            else if(this->dockWidgetArea(SenderOC) ==Qt::RightDockWidgetArea)
            {
                SenderOC->setMaximumHeight(16555);
                SenderOC->setMaximumWidth(600);
            }
        }
    }
    int i = 0;
    QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
    for(auto itt: dockWidgets)
    {
        if(this->dockWidgetArea(itt) == Qt::LeftDockWidgetArea)
        {
            if(SenderOC->objectName().compare("ParameterDock") &&
                    SenderOC->objectName().compare("StateDock") &&
                    SenderOC->objectName().compare("DataDock")&&
                    SenderOC->objectName().compare("OutputDock"))
                if(!(itt->isFloating()))
                    i++;
        }
    }
    if(!i)
        this->ui->centralWidget->show();
    else
         this->ui->centralWidget->hide();

    QApplication::processEvents();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {

  if (!event)
      return false;

  QDockWidget* DW = qobject_cast<QDockWidget*>(obj);
  if(DW)
  {
      if (event->type() == QEvent::Show ) {
      }
      if (event->type() == QEvent::Close ) {
         this->dockWidget_destroyed(DW);
      }
      if (event->type() == QEvent::Leave ) {
         QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
         int found = 0;
         for(auto itt : dockWidgets)
         {
             if(this->dockWidgetArea(itt) == Qt::LeftDockWidgetArea)
                found++;
         }
         if(!found)
             this->ui->centralWidget->setHidden(false);
         else
             this->ui->centralWidget->setHidden(true);

      }
      if(DW->objectName().compare("ParameterDock") == 0)
      {
           if (event->type() == QEvent::Resize )
           {
               int Width = ui->ParameterDock->width();
               ui->ParameterTreeWidget->setColumnWidth(0,Width*0.6);
               ui->ParameterTreeWidget->setColumnWidth(1,Width*0.2);
               ui->ParameterTreeWidget->setColumnWidth(2,Width*0.1);
           }
      }
      if(DW->objectName().compare("DataDock") == 0)
      {
           if (event->type() == QEvent::Resize )
           {
               int Width = ui->DataTreeWidget->width();
               ui->DataTreeWidget->setColumnWidth(0,Width*0.6);
               ui->DataTreeWidget->setColumnWidth(1,Width*0.3);
           }
      }
  }
  return QWidget::eventFilter(obj, event);
}


void MainWindow::HighLightConnection(QString ID)
{
    QStringList Parts = ID.split("::");
    QTreeWidgetItem *CurrentItem = NULL;
    QTreeWidget *SelTreeWidget = NULL;
    QDockWidget *SelDockWidget = NULL;

    for(int r = 0; r < 3 ; r++)
    {
        if(r== 0)
        {
            SelTreeWidget = ui->ParameterTreeWidget;
            SelDockWidget = ui->ParameterDock;
        }
        else if (r == 1)
        {
            SelTreeWidget = ui->DataTreeWidget;
            SelDockWidget = ui->DataDock;
        }
        else if(r==2)
        {
            SelDockWidget = ui->StateDock;
            SelTreeWidget = ui->StateTreeWidget;
        }

        int found = 0;
        CurrentItem = NULL;
        for(int j = 0; j < Parts.size() && SelTreeWidget;j++)
        {
            if(!CurrentItem)
            {
                int topCount =  SelTreeWidget->topLevelItemCount();
                for (int i = 0; i < topCount; i++)
                {
                    QTreeWidgetItem *item = SelTreeWidget->topLevelItem(i);
                    if(item->text(0).compare(Parts[j])==0)
                    {
                        CurrentItem = item;
                        found++;
                    }
                }
            }
            else
            {
                for (int i = 0; i < CurrentItem->childCount(); i++)
                {
                    if(CurrentItem->child(i)->text(0).compare(Parts[j]) == 0)
                    {
                        CurrentItem = CurrentItem->child(i);
                        found++;
                    }
                }
            }
        }
        if(found == Parts.size() )
        {
            SelDockWidget->raise();
            SelTreeWidget->setCurrentItem(CurrentItem,QItemSelectionModel::ClearAndSelect);
        }
    }
}

void MainWindow::RemoveConnection(QString)
{
}

void MainWindow::on_actionMinimize_to_Tray_triggered()
{
    this->showMinimized();
    hide();
    restore->setEnabled(true);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    if(this->isVisible())
    {
        restore->setEnabled(false);
    }
}

void MainWindow::TrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{

    if(reason == QSystemTrayIcon::DoubleClick)
    {
      this->show();
      this->showNormal();
      this->raise();
      restore->setEnabled(false);
    }
}

void MainWindow::ErrorWriter(const QString &ID, const QString Data)
{

    if(this->isloading)
    {
        QTimer::singleShot(100, this, [=]() {ErrorWriter(ID, Data); });
        return;
    }

    OutputLog->Append(MainWindowOutputLog::Kind::Error, ID, Data);
}

void MainWindow::InfoWriter(const QString &ID, const QString Data)
{
    OutputLog->Append(MainWindowOutputLog::Kind::Info, ID, Data);
}

void MainWindow::NotificationWriter(const QString &ID, const QString Data)
{
    OutputLog->Append(MainWindowOutputLog::Kind::Notification, ID, Data);
}


void MainWindow::ParseInputArguments()
{
    QApplication::processEvents();

    bool Tray = false;

    if (QApplication::arguments().size() > 1) {
        int i = 1;
        while(i < QApplication::arguments().size())
        {
            const QString command = QApplication::arguments().at(i);

            if(command.compare("-load") == 0)
            {
                i++;
                if(i < QApplication::arguments().size())
                {
                    const QString Path = QApplication::arguments().at(i);
                    if(!Path.size())
                        return;
                    CloseProject();
                    QFileInfo fi(Path);
                    this->StdSavePath = fi.absolutePath();
                    this->isloading = true;
                    if(!ExtendedDataManagement->LoadExperiment(Path))
                        this->SavePath = Path;
                    this->isloading = false;



                }
            }
            else if(command.compare("-tray") == 0)
            {
                this->on_actionMinimize_to_Tray_triggered();
                Tray = true;
            }
            else if(command.contains(".LAexp"))
            {
                const QString Path = QApplication::arguments().at(i);
                if(!Path.size())
                    return;
                CloseProject();
                QFileInfo fi(Path);
                this->StdSavePath = fi.absolutePath();
                this->isloading = true;
                if(!ExtendedDataManagement->LoadExperiment(Path))
                    this->SavePath = Path;
                this->isloading = false;

            }
            i++;
        }
    // FILENAME now contains path and name of the file to open.
    }


    int i = 0;
    QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
    for(auto itt: dockWidgets)
    {
        if(this->dockWidgetArea(itt) == Qt::LeftDockWidgetArea)
        {
                if(!(itt->isFloating()))
                    i++;
        }
    }
    if(!i)
        this->ui->centralWidget->show();
    else
         this->ui->centralWidget->hide();

    QApplication::processEvents();
    if(!Tray)
        this->showMaximized();
    else
        this->restore->setEnabled(true);
}

void MainWindow::on_actionLoad_Parameter_File_triggered()
{
    QString Filename;
    Filename = QFileDialog::getOpenFileName(this,
             tr("Load Parameter Set"), this->StdSavePath, tr("Parameter Files (*.LAparam)"));
    QFileInfo fi(Filename);
    this->StdSavePath = fi.absolutePath();

    if(Filename.size())
    {
        GetLogic()->ImportFromXml(Filename);
    }
}



void MainWindow::on_actionSave_Parameter_Set_triggered()
{

    QStringList Ids;
    SelectedItems(Ids, this->ui->ParameterTreeWidget->selectedItems());

    if(!Ids.size())
    {
        Error("Please select the Parameter in the Explorer that shell be exported to xml.");
        return;
    }
    QString Path = QFileDialog::getSaveFileName(this,
             tr("Export Parameter Set"), this->StdSavePath, tr("Parameter Files (*.LAparam)"));

    QFileInfo fi(Path);
    this->StdSavePath = fi.absolutePath();
    if(Path.size())
    {
        GetLogic()->Export2Xml(Path, Ids);
    }
}


void MainWindow::on_ParameterTreeWidget_customContextMenuRequested(const QPoint &)
{

}

void MainWindow::on_StateTreeWidget_customContextMenuRequested(const QPoint &pos)
{
    //Add a Min Max Context Menu when an element without childs is selected
    //the tree repesents always the ID e.g ID Parameter::One --> Parameter -> one
    QList<QTreeWidgetItem*> selectedItems = ui->StateTreeWidget->selectedItems();
    if(selectedItems.size() == 1)
    {
      if(!(selectedItems[0]->parent()))
        {
          //this is top level, we can remove the device here
            QMenu *menu = new QMenu(this);
            menu->setObjectName(selectedItems[0]->text(0));

            menu->setAttribute(Qt::WA_DeleteOnClose);
            menu->addAction("Remove Device", this, SLOT(RemoveDevice()));
            menu->popup(ui->StateDock->mapToGlobal(pos));
        }
    }
}

void MainWindow::on_actionAbout_LabAnalyzer_triggered()
{
}

void MainWindow::on_actionAbout_triggered()
{
    auto about = new QDialog(0,Qt::WindowSystemMenuHint | Qt::WindowTitleHint |  Qt::WindowCloseButtonHint);
    about->setWindowTitle("About LabAnaylser");
    Ui::About aboutUi;
    aboutUi.setupUi(about);
    about->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    about->setFixedSize(490,545);
   //aboutUi.label_3->setFixedSize(400,400);
    aboutUi.label_5->setText(QString("<html><head/><body><p><span style=\"  font-size:12pt; font-weight:600;\">LabAnalyser %1</span></p></body></html>").arg(GIT_VERSION));

    about->exec();
    delete about;

}

void MainWindow::on_pushButton_clicked()
{

}

void MainWindow::on_actionExport_Data_h5_triggered()
{
    QStringList Ids;
    SelectedItems(Ids, this->ui->ParameterTreeWidget->selectedItems());
    SelectedItems(Ids, this->ui->DataTreeWidget->selectedItems());


    if(!Ids.size())
    {
        Error("Please select the Data in the Explorer that shell be exported.");
        return;
    }

    auto UiFileName = QFileDialog::getSaveFileName(this,
                                                   tr("Export Data to *.h5"), this->StdSavePath, tr("HDF5 Files (*.h5)"));
    QFileInfo fi(UiFileName);
    this->StdSavePath = fi.absolutePath();

    if(UiFileName.size())
    {
       GetLogic()->Export2Hdf5(UiFileName,Ids);
    }

    return;

}

void MainWindow::on_actionRemote_Connection_Port_2_triggered()
{
    QMessageBox::information(this, tr("Remote Connection Port"),
                                   QString("TCP Port: ") + QString::number(Remote->GetPort()),
                                   QMessageBox::Ok );
}


