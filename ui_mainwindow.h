/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "treewidgetcustomdrop.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionBeenden;
    QAction *actionLoad_Form;
    QAction *actionCreatePlot;
    QAction *actionCreate_Subplot;
    QAction *actionLoadPlugin;
    QAction *actionSave_Experiment;
    QAction *actionLoadExperiment;
    QAction *Close_Project;
    QAction *actionDaten_Exportieren_mat;
    QAction *actionSave;
    QAction *actionMinimize_to_Tray;
    QAction *actionAbout_LabAnalyzer;
    QAction *actionLoad_Parameter_File;
    QAction *actionSave_Parameter_Set;
    QAction *actionAbout;
    QAction *actionExport_Data_h5;
    QWidget *centralWidget;
    QGridLayout *gridLayout_5;
    QMenuBar *menuBarI;
    QMenu *menu;
    QMenu *menuPlot;
    QMenu *menuHilfe;
    QStatusBar *statusBar;
    QToolBar *toolBar;
    QDockWidget *ParameterDock;
    QWidget *dockWidgetContents_10;
    QHBoxLayout *horizontalLayout_3;
    TreeWidgetCustomDrop *ParameterTreeWidget;
    QDockWidget *StateDock;
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout;
    TreeWidgetCustomDrop *StateTreeWidget;
    QDockWidget *DataDock;
    QWidget *dockWidgetContents_2;
    QGridLayout *gridLayout_2;
    TreeWidgetCustomDrop *DataTreeWidget;
    QDockWidget *OutputDock;
    QWidget *dockWidgetContents_3;
    QGridLayout *gridLayout_3;
    QPlainTextEdit *OutputText;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1099, 684);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(10);
        font.setKerning(true);
        MainWindow->setFont(font);
        MainWindow->setTabShape(QTabWidget::Triangular);
        actionBeenden = new QAction(MainWindow);
        actionBeenden->setObjectName(QString::fromUtf8("actionBeenden"));
        actionLoad_Form = new QAction(MainWindow);
        actionLoad_Form->setObjectName(QString::fromUtf8("actionLoad_Form"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/collage-line.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoad_Form->setIcon(icon);
        actionCreatePlot = new QAction(MainWindow);
        actionCreatePlot->setObjectName(QString::fromUtf8("actionCreatePlot"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icons/plot.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCreatePlot->setIcon(icon1);
        actionCreate_Subplot = new QAction(MainWindow);
        actionCreate_Subplot->setObjectName(QString::fromUtf8("actionCreate_Subplot"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/icons/subplot.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCreate_Subplot->setIcon(icon2);
        actionLoadPlugin = new QAction(MainWindow);
        actionLoadPlugin->setObjectName(QString::fromUtf8("actionLoadPlugin"));
        actionLoadPlugin->setCheckable(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/icons/base-station-line.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoadPlugin->setIcon(icon3);
        actionSave_Experiment = new QAction(MainWindow);
        actionSave_Experiment->setObjectName(QString::fromUtf8("actionSave_Experiment"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/icons/save-3-line.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_Experiment->setIcon(icon4);
        actionLoadExperiment = new QAction(MainWindow);
        actionLoadExperiment->setObjectName(QString::fromUtf8("actionLoadExperiment"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/icons/folder-open-line.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoadExperiment->setIcon(icon5);
        Close_Project = new QAction(MainWindow);
        Close_Project->setObjectName(QString::fromUtf8("Close_Project"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/icons/close-fill.png"), QSize(), QIcon::Normal, QIcon::Off);
        Close_Project->setIcon(icon6);
        actionDaten_Exportieren_mat = new QAction(MainWindow);
        actionDaten_Exportieren_mat->setObjectName(QString::fromUtf8("actionDaten_Exportieren_mat"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/icons/file-code-line (1).png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDaten_Exportieren_mat->setIcon(icon7);
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons/icons/save-3-fill.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon8);
        actionMinimize_to_Tray = new QAction(MainWindow);
        actionMinimize_to_Tray->setObjectName(QString::fromUtf8("actionMinimize_to_Tray"));
        actionAbout_LabAnalyzer = new QAction(MainWindow);
        actionAbout_LabAnalyzer->setObjectName(QString::fromUtf8("actionAbout_LabAnalyzer"));
        actionLoad_Parameter_File = new QAction(MainWindow);
        actionLoad_Parameter_File->setObjectName(QString::fromUtf8("actionLoad_Parameter_File"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons/icons/list-settings-line.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoad_Parameter_File->setIcon(icon9);
        actionSave_Parameter_Set = new QAction(MainWindow);
        actionSave_Parameter_Set->setObjectName(QString::fromUtf8("actionSave_Parameter_Set"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/icons/icons/list-settings-fill.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_Parameter_Set->setIcon(icon10);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/icons/icons/sym_about.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAbout->setIcon(icon11);
        actionExport_Data_h5 = new QAction(MainWindow);
        actionExport_Data_h5->setObjectName(QString::fromUtf8("actionExport_Data_h5"));
        actionExport_Data_h5->setIcon(icon7);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy1);
        gridLayout_5 = new QGridLayout(centralWidget);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        gridLayout_5->setContentsMargins(0, 10, 0, 10);
        MainWindow->setCentralWidget(centralWidget);
        menuBarI = new QMenuBar(MainWindow);
        menuBarI->setObjectName(QString::fromUtf8("menuBarI"));
        menuBarI->setGeometry(QRect(0, 0, 1099, 21));
        menu = new QMenu(menuBarI);
        menu->setObjectName(QString::fromUtf8("menu"));
        menuPlot = new QMenu(menuBarI);
        menuPlot->setObjectName(QString::fromUtf8("menuPlot"));
        menuHilfe = new QMenu(menuBarI);
        menuHilfe->setObjectName(QString::fromUtf8("menuHilfe"));
        MainWindow->setMenuBar(menuBarI);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        QFont font1;
        font1.setPointSize(9);
        font1.setBold(false);
        font1.setWeight(50);
        font1.setKerning(true);
        toolBar->setFont(font1);
        toolBar->setAcceptDrops(false);
        toolBar->setIconSize(QSize(50, 24));
        toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolBar->setFloatable(true);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);
        ParameterDock = new QDockWidget(MainWindow);
        ParameterDock->setObjectName(QString::fromUtf8("ParameterDock"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(ParameterDock->sizePolicy().hasHeightForWidth());
        ParameterDock->setSizePolicy(sizePolicy2);
        QFont font2;
        font2.setPointSize(9);
        font2.setKerning(true);
        ParameterDock->setFont(font2);
        ParameterDock->setFloating(false);
        ParameterDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        ParameterDock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents_10 = new QWidget();
        dockWidgetContents_10->setObjectName(QString::fromUtf8("dockWidgetContents_10"));
        horizontalLayout_3 = new QHBoxLayout(dockWidgetContents_10);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        ParameterTreeWidget = new TreeWidgetCustomDrop(dockWidgetContents_10);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(2, QString::fromUtf8("3"));
        __qtreewidgetitem->setText(1, QString::fromUtf8("2"));
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        ParameterTreeWidget->setHeaderItem(__qtreewidgetitem);
        ParameterTreeWidget->setObjectName(QString::fromUtf8("ParameterTreeWidget"));
        ParameterTreeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ParameterTreeWidget->setProperty("showDropIndicator", QVariant(true));
        ParameterTreeWidget->setDragEnabled(true);
        ParameterTreeWidget->setDragDropMode(QAbstractItemView::DragOnly);
        ParameterTreeWidget->setDefaultDropAction(Qt::CopyAction);
        ParameterTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ParameterTreeWidget->setAnimated(false);
        ParameterTreeWidget->setColumnCount(3);
        ParameterTreeWidget->header()->setVisible(false);
        ParameterTreeWidget->header()->setCascadingSectionResizes(true);
        ParameterTreeWidget->header()->setDefaultSectionSize(100);
        ParameterTreeWidget->header()->setStretchLastSection(true);

        horizontalLayout_3->addWidget(ParameterTreeWidget);

        ParameterDock->setWidget(dockWidgetContents_10);
        MainWindow->addDockWidget(Qt::BottomDockWidgetArea, ParameterDock);
        StateDock = new QDockWidget(MainWindow);
        StateDock->setObjectName(QString::fromUtf8("StateDock"));
        StateDock->setFont(font2);
        StateDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        StateDock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        gridLayout = new QGridLayout(dockWidgetContents);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        StateTreeWidget = new TreeWidgetCustomDrop(dockWidgetContents);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem();
        __qtreewidgetitem1->setText(0, QString::fromUtf8("1"));
        StateTreeWidget->setHeaderItem(__qtreewidgetitem1);
        StateTreeWidget->setObjectName(QString::fromUtf8("StateTreeWidget"));
        StateTreeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        StateTreeWidget->setDragEnabled(true);
        StateTreeWidget->setAnimated(false);
        StateTreeWidget->header()->setVisible(false);

        gridLayout->addWidget(StateTreeWidget, 0, 0, 1, 1);

        StateDock->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(Qt::BottomDockWidgetArea, StateDock);
        DataDock = new QDockWidget(MainWindow);
        DataDock->setObjectName(QString::fromUtf8("DataDock"));
        DataDock->setFont(font2);
        DataDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        DataDock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QString::fromUtf8("dockWidgetContents_2"));
        gridLayout_2 = new QGridLayout(dockWidgetContents_2);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        DataTreeWidget = new TreeWidgetCustomDrop(dockWidgetContents_2);
        QTreeWidgetItem *__qtreewidgetitem2 = new QTreeWidgetItem();
        __qtreewidgetitem2->setText(1, QString::fromUtf8("2"));
        __qtreewidgetitem2->setText(0, QString::fromUtf8("1"));
        DataTreeWidget->setHeaderItem(__qtreewidgetitem2);
        DataTreeWidget->setObjectName(QString::fromUtf8("DataTreeWidget"));
        DataTreeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        DataTreeWidget->setDragEnabled(true);
        DataTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        DataTreeWidget->setAnimated(false);
        DataTreeWidget->setColumnCount(2);
        DataTreeWidget->header()->setVisible(false);
        DataTreeWidget->header()->setCascadingSectionResizes(true);

        gridLayout_2->addWidget(DataTreeWidget, 0, 0, 1, 1);

        DataDock->setWidget(dockWidgetContents_2);
        MainWindow->addDockWidget(Qt::BottomDockWidgetArea, DataDock);
        OutputDock = new QDockWidget(MainWindow);
        OutputDock->setObjectName(QString::fromUtf8("OutputDock"));
        OutputDock->setFont(font2);
        OutputDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        OutputDock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::RightDockWidgetArea);
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QString::fromUtf8("dockWidgetContents_3"));
        gridLayout_3 = new QGridLayout(dockWidgetContents_3);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        OutputText = new QPlainTextEdit(dockWidgetContents_3);
        OutputText->setObjectName(QString::fromUtf8("OutputText"));
        OutputText->setUndoRedoEnabled(false);
        OutputText->setTextInteractionFlags(Qt::TextSelectableByMouse);

        gridLayout_3->addWidget(OutputText, 0, 0, 1, 1);

        OutputDock->setWidget(dockWidgetContents_3);
        MainWindow->addDockWidget(Qt::BottomDockWidgetArea, OutputDock);

        menuBarI->addAction(menu->menuAction());
        menuBarI->addAction(menuPlot->menuAction());
        menuBarI->addAction(menuHilfe->menuAction());
        menu->addAction(actionLoadPlugin);
        menu->addAction(actionLoad_Form);
        menu->addSeparator();
        menu->addAction(actionLoadExperiment);
        menu->addAction(actionSave);
        menu->addAction(actionSave_Experiment);
        menu->addAction(Close_Project);
        menu->addSeparator();
        menu->addAction(actionLoad_Parameter_File);
        menu->addAction(actionSave_Parameter_Set);
        menu->addSeparator();
        menu->addAction(actionDaten_Exportieren_mat);
        menu->addAction(actionExport_Data_h5);
        menu->addSeparator();
        menu->addAction(actionMinimize_to_Tray);
        menu->addAction(actionBeenden);
        menuPlot->addAction(actionCreatePlot);
        menuPlot->addAction(actionCreate_Subplot);
        menuHilfe->addAction(actionAbout);
        toolBar->addAction(actionLoadExperiment);
        toolBar->addAction(actionSave);
        toolBar->addAction(Close_Project);
        toolBar->addSeparator();
        toolBar->addAction(actionCreatePlot);
        toolBar->addAction(actionCreate_Subplot);
        toolBar->addSeparator();
        toolBar->addAction(actionLoadPlugin);
        toolBar->addAction(actionLoad_Form);
        toolBar->addSeparator();
        toolBar->addAction(actionSave_Parameter_Set);
        toolBar->addAction(actionLoad_Parameter_File);
        toolBar->addSeparator();
        toolBar->addAction(actionDaten_Exportieren_mat);
        toolBar->addAction(actionExport_Data_h5);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "LabAnalyser", nullptr));
        actionBeenden->setText(QCoreApplication::translate("MainWindow", "Quit", nullptr));
#if QT_CONFIG(shortcut)
        actionBeenden->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLoad_Form->setText(QCoreApplication::translate("MainWindow", "Load User Interface", nullptr));
        actionLoad_Form->setIconText(QCoreApplication::translate("MainWindow", "Load\n"
"UI", nullptr));
#if QT_CONFIG(tooltip)
        actionLoad_Form->setToolTip(QCoreApplication::translate("MainWindow", "Load User Interface Files, created with QT Designer", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionLoad_Form->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+U", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCreatePlot->setText(QCoreApplication::translate("MainWindow", "Plot", nullptr));
#if QT_CONFIG(tooltip)
        actionCreatePlot->setToolTip(QCoreApplication::translate("MainWindow", "Create Plot Window", nullptr));
#endif // QT_CONFIG(tooltip)
        actionCreate_Subplot->setText(QCoreApplication::translate("MainWindow", "Sub Plot", nullptr));
#if QT_CONFIG(tooltip)
        actionCreate_Subplot->setToolTip(QCoreApplication::translate("MainWindow", "Create SubPlot Window", nullptr));
#endif // QT_CONFIG(tooltip)
        actionLoadPlugin->setText(QCoreApplication::translate("MainWindow", "Load Device Plugin", nullptr));
        actionLoadPlugin->setIconText(QCoreApplication::translate("MainWindow", "Load\n"
"Device", nullptr));
#if QT_CONFIG(shortcut)
        actionLoadPlugin->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+D", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave_Experiment->setText(QCoreApplication::translate("MainWindow", "Save Experiment As", nullptr));
        actionLoadExperiment->setText(QCoreApplication::translate("MainWindow", "Load Experiment", nullptr));
        actionLoadExperiment->setIconText(QCoreApplication::translate("MainWindow", "Load\n"
"Experiment", nullptr));
#if QT_CONFIG(tooltip)
        actionLoadExperiment->setToolTip(QCoreApplication::translate("MainWindow", "Load Experiment", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionLoadExperiment->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        Close_Project->setText(QCoreApplication::translate("MainWindow", "Close Experiment", nullptr));
        Close_Project->setIconText(QCoreApplication::translate("MainWindow", "Close\n"
"Experiment", nullptr));
#if QT_CONFIG(tooltip)
        Close_Project->setToolTip(QCoreApplication::translate("MainWindow", "Close Project", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        Close_Project->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDaten_Exportieren_mat->setText(QCoreApplication::translate("MainWindow", "Export Data (*.mat)", nullptr));
        actionDaten_Exportieren_mat->setIconText(QCoreApplication::translate("MainWindow", "Export\n"
"(*.mat)", nullptr));
#if QT_CONFIG(shortcut)
        actionDaten_Exportieren_mat->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+E", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave->setText(QCoreApplication::translate("MainWindow", "Save Experiment", nullptr));
        actionSave->setIconText(QCoreApplication::translate("MainWindow", "Save\n"
"Experiment", nullptr));
#if QT_CONFIG(shortcut)
        actionSave->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionMinimize_to_Tray->setText(QCoreApplication::translate("MainWindow", "Minimize to Tray", nullptr));
        actionAbout_LabAnalyzer->setText(QCoreApplication::translate("MainWindow", "About LabAnalyser", nullptr));
        actionLoad_Parameter_File->setText(QCoreApplication::translate("MainWindow", "Load Parameter File", nullptr));
        actionLoad_Parameter_File->setIconText(QCoreApplication::translate("MainWindow", "Load\n"
"Parameter", nullptr));
#if QT_CONFIG(shortcut)
        actionLoad_Parameter_File->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+L", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave_Parameter_Set->setText(QCoreApplication::translate("MainWindow", "Save Parameter Set", nullptr));
        actionSave_Parameter_Set->setIconText(QCoreApplication::translate("MainWindow", "Save\n"
"Parameter", nullptr));
#if QT_CONFIG(shortcut)
        actionSave_Parameter_Set->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAbout->setText(QCoreApplication::translate("MainWindow", "About", nullptr));
        actionExport_Data_h5->setText(QCoreApplication::translate("MainWindow", "Export Data (*.h5)", nullptr));
        actionExport_Data_h5->setIconText(QCoreApplication::translate("MainWindow", "Export \n"
"(*.h5)", nullptr));
        menu->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuPlot->setTitle(QCoreApplication::translate("MainWindow", "Plot", nullptr));
        menuHilfe->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
        ParameterDock->setWindowTitle(QCoreApplication::translate("MainWindow", "Parameter Explorer", nullptr));
        StateDock->setWindowTitle(QCoreApplication::translate("MainWindow", "State Explorer", nullptr));
        DataDock->setWindowTitle(QCoreApplication::translate("MainWindow", "Data Explorer", nullptr));
        OutputDock->setWindowTitle(QCoreApplication::translate("MainWindow", "Outputs", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
