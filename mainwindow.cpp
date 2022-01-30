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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qstringlist.h"
#include  "DropWidgets/Plots/PlotWidget.h"
#include "UIFunctions/SubPlotMainWindow.h"

#include "DropWidgets/DropWidgets.h"
#include "DropWidgets/DropWidgetsUiLoader.h"
#include <QFileInfo>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{


    //Set Object Name for Main Form
    this->setObjectName("LabAnalyser");

    //Create Tray Icon
    icon = new QSystemTrayIcon(QIcon(":/icons/sym.png"), this);
    auto menu = new QMenu(this);
    restore = new QAction("Restore",this);
    restore->setEnabled(false);
    connect(restore,SIGNAL(triggered()),this,SLOT(showNormal()));
    menu->addAction(restore);
    menu->addSeparator();
    auto  quitAction = new QAction(QIcon(":/icons/icons/Exit.png"), "Quit",this);
    connect(quitAction,SIGNAL(triggered()),this,SLOT(close()));
    menu->addAction(quitAction);    
    icon->setContextMenu(menu);
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

    ParseInputArguments();

}


void MainWindow::OutputTextMenu( QPoint p )
{
    // Start with the standard menu.
     QMenu * pMenu = ui->OutputText->createStandardContextMenu();
     QAction * pAction;

     pMenu->addSeparator();
     pAction = new QAction( "Clear Output", this );
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
                    auto sit = items.at(i);
                    QString Name;
                    while(sit)
                    {
                        Name.insert(0,sit->text( 0 ));
                        sit = sit->parent();
                        if(sit)
                            Name.insert(0,"::");
                    }
                    Ids.push_back(Name);
                }
            }
            QAction *SetAliasAction = new QAction;
                connect(SetAliasAction, &QAction::triggered, [=]{
                    SetAlias(GetLogic()->GetAlias(Ids));});

            QAction *RemoveAliasAction = new QAction;
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
      QString ID;
      auto sit = selectedItems[0];
      while(sit)
      {
          ID.insert(0,sit->text( 0 ));
          sit = sit->parent();
          if(sit)
              ID.insert(0,"::");
      }
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
    auto ID = sender->parent()->objectName();
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
    delete ui;
}

void MainWindow::on_actionLoad_Form_triggered()
{
    auto UiFileName = QFileDialog::getOpenFileName(this,
             tr("Open UI File"), this->StdSavePath, tr("UI Files (*.ui)"));
    QFileInfo fi = UiFileName;
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

void MainWindow::DeleteFigure(SubPlotMainWindow* FigurePointer)
{
    this->GetLogic()->DeletePlotWindow(FigurePointer->objectName());
}

void MainWindow::on_actionCreate_Subplot_triggered()
{
    //Row Column Dialog
    QDialog * d = new QDialog(0, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    QWidget * uw = new QWidget(d);
    QHBoxLayout * hbox = new QHBoxLayout(uw);
    QVBoxLayout * vbox = new QVBoxLayout(d);
    QComboBox * comboBoxA = new QComboBox();
    comboBoxA->addItems(QStringList() << "1" << "2" << "3");
    QComboBox * comboBoxB = new QComboBox();
    comboBoxB->addItems(QStringList() << "1" << "2" << "3" << "4");
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                        | QDialogButtonBox::Cancel);
    QLabel *LR = new QLabel();
    LR->setText(QString("Rows:"));
    QLabel *LC = new QLabel();
    LC->setText(QString("Columns:"));
    QObject::connect(buttonBox, SIGNAL(accepted()), d, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), d, SLOT(reject()));

    hbox->addWidget(LR);
    hbox->addWidget(comboBoxA);
    hbox->addWidget(LC);
    hbox->addWidget(comboBoxB);
    vbox->addWidget(uw);
    vbox->addWidget(buttonBox);
    d->setLayout(vbox);

    int result = d->exec();
    if(result == QDialog::Accepted)
    {
        CreateSubPlotWindow(comboBoxA->currentText().toInt(),comboBoxB->currentText().toInt());
    }
}

SubPlotMainWindow* MainWindow::CreateSubPlotWindow(int rows, int cols)
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
            PlotWidget *PW = new PlotWidget(this, NW, MW->GetStatusBar());
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

//Create unique names in every tab, by appending the FileName (AppendSuffix)
void MainWindow::AppendWidgetNames(QObjectList Childs, QString AppendSuffix)
{
    for(int i = 0; i < Childs.size(); i++)
    {
        if(Childs[i]->children().size()>0)
        {
            AppendWidgetNames(Childs[i]->children(), AppendSuffix);
        }
         QString name = Childs[i]->objectName();
         if(name.size())
         {
            Childs[i]->setObjectName(name.append("_").append(AppendSuffix));            
         }
         if(qobject_cast<PlotWidget*>(Childs[i]))
         {
             this->GetLogic()->AddPlotPointer(Childs[i]->objectName(),Childs[i]);
         }
    }
}


void MainWindow::LoadFormFromXML(QString Path)
{
    LoadFormFromXML(Path, QString());
}

void MainWindow::LoadFormFromXML(QString UiFileName, QString LastFormName, bool skip)
{
    UiFileName.replace("\\","/");
    auto FilePartsI = UiFileName.split("/");
    auto FilePartsII = FilePartsI.at(FilePartsI.size()-1);
    auto FilePartsIII = FilePartsII.split(".");
    auto FileNameS = FilePartsIII.at(FilePartsIII.size()-2);

    QWidget* tab = new QWidget();
    QFileInfo fi(UiFileName);
    QDir::setCurrent( fi.absoluteDir().absolutePath());
    //tab->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    DropWidgetsUiLoader loader(this);
    QFile file(UiFileName);
    loader.setWorkingDirectory(fi.absoluteDir().absolutePath());
    QWidget *formWidget = loader.load(&file, tab);
    file.close();

    if(!formWidget)
    {
        Error("Corrupt Form File");
        return;
    }

    QTime time = QTime::currentTime();
    QDate Date = QDate::currentDate();

    QString DateTime;
    DateTime.append(Date.toString("_yyyy_MM_dd_"));
    DateTime.append(time.toString("hh_mm_ss"));

    QString FormName;
    if(LastFormName.size())
        FormName = LastFormName;
    else
        FormName = FileNameS + DateTime;

    //Append Date and Time to make it unique
    AppendWidgetNames(formWidget->children(),FormName) ;

    tab->setObjectName(FormName);

    QGridLayout *gridLayout0 = new QGridLayout();
    gridLayout0->setSpacing(6);
    gridLayout0->setContentsMargins(11, 35, 11, 11);

    QWidget *client0 = new QWidget;
    client0->setLayout(gridLayout0);

    QWidget *client = formWidget;
    QScrollArea *scrollArea = new QScrollArea;

    QPalette pal;
    pal.setColor(QPalette::Window,QColor(0,0,0,0));
    scrollArea->setPalette(pal);
    scrollArea->setBackgroundRole(QPalette::Window);
    scrollArea->setFrameShape(QFrame::NoFrame);

    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(client0);
    client0->layout()->addWidget(client);

    QWidget *pageWidget = tab;
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    pageWidget->setLayout(gridLayout);
    pageWidget->layout()->addWidget(scrollArea);
    QPalette pal2; pal2.setColor(QPalette::Background, Qt::white);
    pageWidget->setPalette(pal2);
    pageWidget->setAutoFillBackground(true);

    ui->centralWidget->hide();
    QDockWidget* NewDock = new QDockWidget(this);
    NewDock->setObjectName(FormName);
    NewDock->setWindowTitle(FileNameS);
    NewDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetClosable);
    NewDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    QGridLayout *gridLayoutDock = new QGridLayout();
    gridLayoutDock->setSpacing(6);
    gridLayoutDock->setContentsMargins(0, 0, 0, 0);
    NewDock->setWidget(pageWidget);
    NewDock->setAttribute(Qt::WA_DeleteOnClose);
    NewDock->installEventFilter(this);

    NewDock->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding);

    this->ui->centralWidget->hide();
    this->addDockWidget(Qt::LeftDockWidgetArea, NewDock);

    QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
    for(auto itt: dockWidgets)
    {
        if(this->dockWidgetArea(itt) == Qt::LeftDockWidgetArea)
        {
            if(!(itt->isFloating()) && isVisible() && NewDock!=itt) //Front window
            {
                QMainWindow::tabifyDockWidget(itt,NewDock);
                NewDock->show();
                NewDock->raise();
            }
        }
    }
    connect(NewDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockWidget_topLevelChanged(bool)));
    connect(NewDock, SIGNAL(destroyed(QObject*)), this, SLOT(dockWidget_destroyed(QObject*)));

    //Save Filename
    GetLogic()->AddFormFile(std::pair<QString, QString>(FormName, UiFileName));
    GetLogic()->AddSkipFormFile(FormName,skip);
    QApplication::processEvents();

}

void MainWindow::on_actionLoadPlugin_triggered()
{

    QString FileName = QFileDialog::getOpenFileName(this, tr("Load Plugin/Device File"), this->StdSavePath, tr("Plugin/Device Files (*.LAdev)"));
    if(!FileName.size())
        return;
    QFileInfo fi = FileName;
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
                int ChildIndex = 0;
                for (int i = 0; i < CurrentItem->childCount() && !ChildFound; i++)
                {
                    if(CurrentItem->child(i)->text(0).compare(Parts[j]) == 0)
                    {
                        ChildIndex = i;
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
     ui->ParameterTreeWidget->setUpdatesEnabled( false );
     ui->ParameterTreeWidget->setSortingEnabled(false);

     ui->DataTreeWidget->setUpdatesEnabled( false );
     ui->DataTreeWidget->setSortingEnabled(false);

     ui->StateTreeWidget->setUpdatesEnabled( false );
     ui->StateTreeWidget->setSortingEnabled(false);


 }

 void MainWindow::PublishFinished()
 {
     QTreeWidget *SelTreeWidget = NULL;
     ui->ParameterTreeWidget->setUpdatesEnabled( true );
     ui->DataTreeWidget->setUpdatesEnabled( true );
     ui->StateTreeWidget->setUpdatesEnabled( true );


    SelTreeWidget = ui->ParameterTreeWidget;
    int Width = ui->ParameterDock->width();
    SelTreeWidget->setColumnWidth(0,Width*0.6);
    SelTreeWidget->setColumnWidth(1,Width*0.2);
    SelTreeWidget->setColumnWidth(2,Width*0.1);
    SelTreeWidget->sortByColumn(0, Qt::AscendingOrder);
    SelTreeWidget->setSortingEnabled(true);

    SelTreeWidget = ui->DataTreeWidget;
    Width = ui->DataDock->width();
    SelTreeWidget->setColumnWidth(0,Width*0.6);
    SelTreeWidget->setColumnWidth(1,Width*0.2);
    SelTreeWidget->setColumnWidth(2,Width*0.1);
    SelTreeWidget->sortByColumn(0, Qt::AscendingOrder);
    SelTreeWidget->setSortingEnabled(true);

    SelTreeWidget = ui->StateTreeWidget;
    Width = ui->StateDock->width();
    SelTreeWidget->setColumnWidth(0,Width*0.6);
    SelTreeWidget->setColumnWidth(1,Width*0.2);
    SelTreeWidget->setColumnWidth(2,Width*0.1);
    SelTreeWidget->sortByColumn(0, Qt::AscendingOrder);



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

    QFileInfo fi = Path;
    this->StdSavePath = fi.absolutePath();
    if(Path.size())
    {
        ExtendedDataManagement->SaveExperiment(Path);
        this->SavePath = Path;
        ChangeForSaveDetected = false;
    }
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
            QFileInfo fi = UiFileName;
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

    QFileInfo fi = Path;
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
        QDockWidget* DW =  this->findChild<QDockWidget*>(this->GetLogic()->GetFormFileEntry(0).first);
        if(DW)
            DW->close();
        delete DW;
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

//Todo: export umbauen
void MainWindow::on_actionDaten_Exportieren_mat_triggered()
{
    QStringList Ids;
    {
        QList<QTreeWidgetItem*> selectedItems = this->ui->ParameterTreeWidget->selectedItems();
        QString ID;
        std::map<QString, DataPair> Data;
        int f = 0;
        for(auto si : selectedItems)
        {
            if(si->childCount() == 0)
            {
                auto sit = si;
                while(sit)
                {
                    ID.insert(0,sit->text( 0 ));
                    sit = sit->parent();
                    if(sit)
                        ID.insert(0,"::");
                }
                if(this->GetLogic()->GetContainer(ID))
                {
                    Ids.push_back(ID);
                }
                ID.clear();
            }
        }
    }
    {
        QList<QTreeWidgetItem*> selectedItems = this->ui->DataTreeWidget->selectedItems();
        QString ID;
        std::map<QString, DataPair> Data;
        int f = 0;
        for(auto si : selectedItems)
        {
            if(si->childCount() == 0)
            {
                auto sit = si;
                while(sit)
                {
                    ID.insert(0,sit->text( 0 ));
                    sit = sit->parent();
                    if(sit)
                        ID.insert(0,"::");
                }
                if(this->GetLogic()->GetContainer(ID))
                {
                    Ids.push_back(ID);
                }
                ID.clear();
            }
        }
    }

    if(!Ids.size())
    {
        Error("Please select the Data in the Explorer that shell be exported.");
        return;
    }

    auto UiFileName = QFileDialog::getSaveFileName(this,
                                                   tr("Export Data to *.mat"), this->StdSavePath, tr("Mat Files (*.mat)"));
    QFileInfo fi = UiFileName;
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
    QTreeWidgetItem *LastItem = NULL;
    QTreeWidget *SelTreeWidget = NULL;
    QDockWidget *SelDockWidget = NULL;

    for(int r = 0; r < 3 ; r++)
    {
        LastItem = NULL;
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
                        LastItem  = CurrentItem;
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

void MainWindow::RemoveConnection(QString ID)
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

    QTime time = QTime::currentTime();
    QString line =time.toString("hh:mm:ss") % " " % ID % ":&nbsp;&nbsp;&nbsp; ";
    QString line2 =Data;

    if(line2.size()>10000)
    {
        line2 = line2.left(1000);
        line2.prepend("<font color=\"Red\"><b> WARNING: The Data was trimmed by LabAnalyser, because of its size. It might be due to a babbling idiot.</b></font><br>");
    }
    line2.prepend("<br>");
    QTextCursor cursor = ui->OutputText->textCursor();
    QString alertHtml = "<font color=\"Red\">";
    QString endHtml = "</font> ";
    line = alertHtml % "<b>" % line;
    line = line % "</b>" %  endHtml;
    const bool atBottom = ui->OutputText->verticalScrollBar()->value() == ui->OutputText->verticalScrollBar()->maximum();
    QTextDocument* doc = ui->OutputText->document();
    QTextCursor tcursor(doc);
    tcursor.movePosition(QTextCursor::End);
    tcursor.beginEditBlock();
    tcursor.insertHtml(line);
    tcursor.insertHtml(alertHtml%line2%endHtml);
    tcursor.insertBlock();

    tcursor.endEditBlock();
    if (atBottom) {
       QScrollBar* bar =  ui->OutputText->verticalScrollBar();
       bar->setValue(bar->maximum());
    }
    ui->OutputDock->raise();
}

void MainWindow::InfoWriter(const QString &ID, const QString Data)
{

    QTime time = QTime::currentTime();
    QString line =time.toString("hh:mm:ss") % " " % ID % ":&nbsp;&nbsp;&nbsp; ";
    QString line2 =Data;

    if(line2.size()>10000)
    {
        line2 = line2.left(1000);
        line2.prepend("<font color=\"Red\"><b> WARNING: The Data was trimmed by LabAnalyser, because of its size. It might be due to a babbling idiot.</b></font><br>");
    }
    line2.prepend("<br>");
    QString infoHtml = "<font color=\"Black\">";
    QString endHtml = "</font>";
    line = infoHtml% "<b>" % line;
    line = line% "</b>" % endHtml;
    const bool atBottom = ui->OutputText->verticalScrollBar()->value() == ui->OutputText->verticalScrollBar()->maximum();
    QTextDocument* doc = ui->OutputText->document();
    QTextCursor tcursor(doc);
    tcursor.movePosition(QTextCursor::End);
    tcursor.beginEditBlock();
    tcursor.insertHtml(line);
    tcursor.insertHtml(infoHtml%line2%endHtml);
    tcursor.insertBlock();
    tcursor.endEditBlock();
    if (atBottom)
    {
       QScrollBar* bar =  ui->OutputText->verticalScrollBar();
       bar->setValue(bar->maximum());
    }

}

void MainWindow::NotificationWriter(const QString &ID, const QString Data)
{


    QTime time = QTime::currentTime();
    QString line =time.toString("hh:mm:ss") % " " % ID % ":&nbsp;&nbsp;&nbsp; ";
    QString line2 =Data;
    if(line2.size()>10000)
    {
        line2 = line2.left(1000);
        line2.prepend("<font color=\"Red\"><b> WARNING: The Data was trimmed by LabAnalyser, because of its size. It might be due to a babbling idiot.</b></font><br>");
    }
    line2.prepend("<br>");

    QTextCursor cursor = ui->OutputText->textCursor();
    QString notifyHtml = "<font color=\"DarkBlue\">";
    QString endHtml = "</font>";
    line = notifyHtml % "<b>" % line;
    line = line% "</b>" % endHtml;
    const bool atBottom = ui->OutputText->verticalScrollBar()->value() == ui->OutputText->verticalScrollBar()->maximum();
    QTextDocument* doc = ui->OutputText->document();
    QTextCursor tcursor(doc);
    tcursor.movePosition(QTextCursor::End);
    tcursor.beginEditBlock();
    tcursor.insertHtml(line);
    tcursor.insertHtml(notifyHtml%line2%endHtml);
    tcursor.insertBlock();

    tcursor.endEditBlock();
    if (atBottom) {
       QScrollBar* bar =  ui->OutputText->verticalScrollBar();
       bar->setValue(bar->maximum());
    }
    ui->OutputDock->raise();
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
                    QFileInfo fi = Path;
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
                QFileInfo fi = Path;
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
    QFileInfo fi = Filename;
    this->StdSavePath = fi.absolutePath();

    if(Filename.size())
    {
        GetLogic()->ImportFromXml(Filename);
    }
}



void MainWindow::on_actionSave_Parameter_Set_triggered()
{

    QStringList Ids;
    {
        QList<QTreeWidgetItem*> selectedItems = this->ui->ParameterTreeWidget->selectedItems();
        QString ID;
        std::map<QString, DataPair> Data;
        int f = 0;
        for(auto si : selectedItems)
        {
            if(si->childCount() == 0)
            {
                auto sit = si;
                while(sit)
                {
                    ID.insert(0,sit->text( 0 ));
                    sit = sit->parent();
                    if(sit)
                        ID.insert(0,"::");
                }
                if(this->GetLogic()->GetContainer(ID))
                {
                    Ids.push_back(ID);
                }
                ID.clear();
            }
        }
    }
    if(!Ids.size())
    {
        Error("Please select the Parameter in the Explorer that shell be exported to xml.");
        return;
    }
    QString Path = QFileDialog::getSaveFileName(this,
             tr("Export Parameter Set"), this->StdSavePath, tr("Parameter Files (*.LAparam)"));

    QFileInfo fi = Path;
    this->StdSavePath = fi.absolutePath();
    if(Path.size())
    {
        GetLogic()->Export2Xml(Path, Ids);
    }
}


void MainWindow::on_ParameterTreeWidget_customContextMenuRequested(const QPoint &pos)
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
    about->setFixedSize(518,460);
    aboutUi.label_5->setText(QString("<html><head/><body><p><span style=\"  font-size:12pt; font-weight:600;\">LabAnalyser %1</span></p></body></html>").arg(GIT_VERSION));

    about->exec();

}

void MainWindow::on_pushButton_clicked()
{

}

void MainWindow::on_actionExport_Data_h5_triggered()
{
    QStringList Ids;
    {
        QList<QTreeWidgetItem*> selectedItems = this->ui->ParameterTreeWidget->selectedItems();
        QString ID;
        std::map<QString, DataPair> Data;
        int f = 0;
        for(auto si : selectedItems)
        {
            if(si->childCount() == 0)
            {
                auto sit = si;
                while(sit)
                {
                    ID.insert(0,sit->text( 0 ));
                    sit = sit->parent();
                    if(sit)
                        ID.insert(0,"::");
                }
                if(this->GetLogic()->GetContainer(ID))
                {
                    Ids.push_back(ID);
                }
                ID.clear();
            }
        }
    }
    {
        QList<QTreeWidgetItem*> selectedItems = this->ui->DataTreeWidget->selectedItems();
        QString ID;
        std::map<QString, DataPair> Data;
        int f = 0;
        for(auto si : selectedItems)
        {
            if(si->childCount() == 0)
            {
                auto sit = si;
                while(sit)
                {
                    ID.insert(0,sit->text( 0 ));
                    sit = sit->parent();
                    if(sit)
                        ID.insert(0,"::");
                }
                if(this->GetLogic()->GetContainer(ID))
                {
                    Ids.push_back(ID);
                }
                ID.clear();
            }
        }
    }

    if(!Ids.size())
    {
        Error("Please select the Data in the Explorer that shell be exported.");
        return;
    }

    auto UiFileName = QFileDialog::getSaveFileName(this,
                                                   tr("Export Data to *.h5"), this->StdSavePath, tr("HDF5 Files (*.h5)"));
    QFileInfo fi = UiFileName;
    this->StdSavePath = fi.absolutePath();

    if(UiFileName.size())
    {
       GetLogic()->Export2Hdf5(UiFileName,Ids);
    }

    return;

}
