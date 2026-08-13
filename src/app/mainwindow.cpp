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
#include "UIFunctions/MainWindowTreeModel.h"
#include "UIFunctions/MainWindowExplorerValues.h"
#include "UIFunctions/MainWindowFigureFactory.h"
#include "UIFunctions/MainWindowDockPresentation.h"
#include "UIFunctions/MainWindowTrayController.h"
#include "UIFunctions/MainWindowProjectCleanup.h"
#include "UIFunctions/MainWindowContextMenus.h"
#include "UIFunctions/MainWindowProjectActions.h"

#include "DropWidgets/DropWidgets.h"
#include "DropWidgets/DropWidgetsUiLoader.h"
#include <QFileInfo>
#include <QTimer>



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
    ui->ParameterTreeWidget->setColumnCount(MainWindowExplorerValues::StateColumn + 1);
    ui->ParameterTreeWidget->headerItem()->setText(MainWindowExplorerValues::NameColumn, "Item");
    ui->ParameterTreeWidget->headerItem()->setText(MainWindowExplorerValues::ValueColumn, "Value");
    ui->ParameterTreeWidget->headerItem()->setText(MainWindowExplorerValues::TypeColumn, "Data Type");
    ui->ParameterTreeWidget->headerItem()->setText(MainWindowExplorerValues::StateColumn, "State");

    ui->DataTreeWidget->setColumnCount(MainWindowExplorerValues::StateColumn + 1);
    ui->DataTreeWidget->headerItem()->setText(MainWindowExplorerValues::NameColumn, "Item");
    ui->DataTreeWidget->headerItem()->setText(MainWindowExplorerValues::ValueColumn, "Value");
    ui->DataTreeWidget->headerItem()->setText(MainWindowExplorerValues::TypeColumn, "Data Type");
    ui->DataTreeWidget->headerItem()->setText(MainWindowExplorerValues::StateColumn, "State");

    ui->DataTreeWidget->setAlternatingRowColors(true);
    ui->ParameterTreeWidget->setAlternatingRowColors(true);
    ui->StateTreeWidget->setAlternatingRowColors(true);



    //Parameter Widget should have Context Menu for Min/Max Value editing
    ui->ParameterTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ParameterTreeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuTreeWidget(QPoint)));
    connect(ui->ParameterTreeWidget, &QTreeWidget::itemDoubleClicked,
            this, &MainWindow::EditParameterValue);


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

    ExplorerValueRefreshTimer = new QTimer(this);
    ExplorerValueRefreshTimer->setObjectName("ExplorerValueRefreshTimer");
    ExplorerValueRefreshTimer->setInterval(500);
    connect(ExplorerValueRefreshTimer, &QTimer::timeout, this, &MainWindow::RefreshExplorerValues);
    ExplorerValueRefreshTimer->start();

    ParseInputArguments();

}


void MainWindow::OutputTextMenu( QPoint p )
{
    MainWindowOutputLog::ShowContextMenu(*ui->OutputText, p);
}


//Resize of the Main Window shell rezisze the dock widget columns as well
void MainWindow::resizeEvent(QResizeEvent *event)
{
    //Resize the dockables when window resizes
    QMainWindow::resizeEvent(event);
    MainWindowExplorerValues::ConfigureColumns(*ui->ParameterTreeWidget, ui->ParameterDock->width());
    MainWindowExplorerValues::ConfigureColumns(*ui->DataTreeWidget, ui->DataDock->width());
}

void MainWindow::RefreshExplorerValues()
{
    if (!ExtendedDataManagement)
        return;
    MainWindowExplorerValues::RefreshVisible(*ui->ParameterTreeWidget, *ExtendedDataManagement);
    MainWindowExplorerValues::RefreshVisible(*ui->DataTreeWidget, *ExtendedDataManagement);
}


//This function is connected to the contextMenuRequest of the DockWidgets
void MainWindow::contextMenuTreeWidget(QPoint pos)
{
    MainWindowContextMenus::ShowParameter(*ui->ParameterTreeWidget, *ui->ParameterDock,
                                          pos, *this, this);
}

void MainWindow::contextMenuTreeWidgetState(QPoint pos)
{
    MainWindowContextMenus::ShowState(*ui->StateTreeWidget, *ui->ParameterDock, pos, *this, this);
}


//This function is connected to the contextMenuRequest of the DockWidgets
void MainWindow::contextMenuTreeWidgetData(QPoint pos)
{
    MainWindowContextMenus::ShowData(*ui->DataTreeWidget, *ui->DataDock, pos, *this,
                                     [this](const QString& id) { return GetLogic()->GetAlias(id); },
                                     [this](const QString& id) { SetAlias(id); },
                                     [this](const QString& id) { RemoveAlias(id); }, this);
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

void MainWindow::EditParameterValue(QTreeWidgetItem* item, int)
{
    if (!item || item->childCount() != 0)
        return;

    const QString id = MainWindowTreePath::IdForItem(item);
    ToFormMapper* mapper = GetLogic()->GetContainer(id);
    if (!mapper || !mapper->IsNumeric())
        return;

    InterfaceData value(*mapper);
    bool accepted = false;
    if (value.IsBool()) {
        const QString selected = QInputDialog::getItem(this, tr("Set Parameter Value"), id,
                                                       {QStringLiteral("false"), QStringLiteral("true")},
                                                       value.GetBool() ? 1 : 0, false, &accepted);
        if (accepted)
            value.SetDataKeepType(selected == QStringLiteral("true"));
    } else {
        const QString entered = QInputDialog::getText(this, tr("Set Parameter Value"), id,
                                                      QLineEdit::Normal,
                                                      QString::number(value.GetFloatingPointData(), 'g',
                                                                      value.GetTypeInfo() == QStringLiteral("float") ? 7 : 12),
                                                      &accepted);
        if (!accepted)
            return;

        bool valid = false;
        const QString type = value.GetTypeInfo();
        if (value.IsSigedNumber()) {
            const qlonglong number = entered.toLongLong(&valid);
            if (valid) {
                if (type == QStringLiteral("int64_t")) value.SetDataKeepType(static_cast<int64_t>(number));
                else value.SetDataKeepType(static_cast<int32_t>(number));
            }
        } else if (value.IsUnsigedNumber()) {
            const qulonglong number = entered.toULongLong(&valid);
            if (valid) {
                if (type == QStringLiteral("uint64_t")) value.SetDataKeepType(static_cast<uint64_t>(number));
                else value.SetDataKeepType(static_cast<uint32_t>(number));
            }
        } else if (value.IsFloatingPointNumber()) {
            const double number = entered.toDouble(&valid);
            if (valid) {
                if (type == QStringLiteral("float")) value.SetDataKeepType(static_cast<float>(number));
                else value.SetDataKeepType(number);
            }
        }
        if (!valid)
            return;
    }

    if (accepted) {
        GetLogic()->GetMessenger()->MessageReceiver(QStringLiteral("set"), id, value);
        RefreshExplorerValues();
    }
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
    return MainWindowFigureFactory::Create(*this, rows, cols, IsFFTPlot);
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
    MainWindowTreeModel::AddElement(SelTreeWidget, ID.split("::"), Data);
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
    MainWindowTreeModel::RemoveElement({ui->ParameterTreeWidget, ui->DataTreeWidget,
                                        ui->StateTreeWidget}, ID.split("::"));
 }

void MainWindow::on_actionSave_Experiment_triggered()
{
    MainWindowProjectActions::SaveExperiment(*this, StdSavePath, ChangeForSaveDetected,
                                              [this](const QString& path) { emit SaveExperiment(path); });
}

void MainWindow::on_actionLoadExperiment_triggered()
{
    MainWindowProjectActions::LoadExperiment(
            *this, StdSavePath, SavePath, ChangeForSaveDetected, isloading,
            [this] { CloseProject(); },
            [this](const QString& path) { GetLogic()->SaveExperiment(path); },
            [this](const QString& path) { return ExtendedDataManagement->LoadExperiment(path); });

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

    MainWindowProjectCleanup::Close(*this, *GetLogic(), *ui->ParameterTreeWidget,
                                    *ui->DataTreeWidget, *ui->StateTreeWidget);
}

void MainWindow::on_Close_Project_triggered()
{
    MainWindowProjectActions::CloseProjectWithPrompt(
            *this, StdSavePath, ChangeForSaveDetected,
            [this] { return GetLogic()->GetFormFileCount(); },
            [this](const QString& path) { ExtendedDataManagement->SaveExperiment(path); },
            [this] { CloseProject(); });
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
    MainWindowDockPresentation::UpdateTopLevelState(*this,
        qobject_cast<QDockWidget*>(QObject::sender()), isFloating);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  MainWindowDockPresentation::HandleEvent(*this, obj, event);
  return QWidget::eventFilter(obj, event);
}


void MainWindow::HighLightConnection(QString ID)
{
    MainWindowTreeModel::Highlight({{ui->ParameterTreeWidget, ui->ParameterDock},
                                    {ui->DataTreeWidget, ui->DataDock},
                                    {ui->StateTreeWidget, ui->StateDock}}, ID.split("::"));
}

void MainWindow::RemoveConnection(QString)
{
}

void MainWindow::on_actionMinimize_to_Tray_triggered()
{
    MainWindowTrayController::MinimizeToTray(*this, *restore);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    MainWindowTrayController::UpdateRestoreAction(*this, *restore);
}

void MainWindow::TrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    MainWindowTrayController::HandleActivation(*this, *restore, reason);
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
    MainWindowContextMenus::ShowState(*ui->StateTreeWidget, *ui->StateDock, pos, *this, this);
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


