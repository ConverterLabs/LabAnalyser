#include <QtTest>
#include <QAction>
#include <QAbstractButton>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDockWidget>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QMenuBar>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPointer>
#include <QSettings>
#include <QScrollBar>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>

#include "DropWidgets/QLineEdit.h"
#include "DataManagement/DataMessengerClass.h"
#include "mainwindow.h"
#include "plugins/platforminterface.h"

namespace
{
class Gui19Device final : public Platform_Interface
{
public:
    Gui19Device(const QString& name, int* destroyed) : destroyed_(destroyed)
    {
        object_.setObjectName(name);
    }
    ~Gui19Device() override { ++*destroyed_; }
    InterfaceData* GetSymbol(const QString&) override { return nullptr; }
    QObject* GetObject() override { return &object_; }
    void MessageReceiver(const QString&, const QString&, InterfaceData) override {}
    void MessageSender(const QString&, const QString&, InterfaceData) override {}

private:
    QObject object_;
    int* destroyed_;
};
}

class MainWindowIntegrationTests final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void GUI_001_construct_destroy_and_isolated_settings();
    void GUI_002_identity_hierarchy_menus_actions_and_defaults();
    void GUI_003_safe_action_connections_and_project_reset();
    void GUI_004_docks_visibility_and_floating_lifecycle();
    void GUI_005_plot_subwindows_repeat_close_and_shutdown();
    void GUI_006_dynamic_form_docks_repeat_ownership_and_loader_errors();
    void GUI_007_loaded_dropwidgets_manager_messenger_and_close_project();
    void GUI_008_loaded_form_direct_drag_events_are_rejected_without_source();
    void GUI_009_modal_actions_open_once_and_abort_without_output();
    void GUI_010_cancelled_file_dialogs_currently_mutate_selected_state();
    void GUI_011_unsaved_close_project_cancel_and_discard();
    void GUI_012_subplot_action_cancel_valid_sizes_and_repeat();
    void GUI_013_tray_restore_and_output_menu_contract();
    void GUI_014_nested_trees_messenger_updates_and_valid_highlighting();
    void GUI_015_form_dock_recreate_from_temporary_fixture();
    void GUI_016_valid_leaf_context_actions_and_multi_selection();
    void GUI_017_plot_action_routing_and_repeat_targets();
    void GUI_018_notification_status_scroll_and_timer_lifetime();
    void GUI_019_parameter_min_max_dialog_contract();
    void GUI_020_data_alias_and_multi_selection_contract();
    void GUI_021_device_context_action_removes_without_confirmation();
    void GUI_SAFE_001_senderless_and_invalid_selection_actions_are_noops();
    void GUI_022_publish_tree_view_state_contract();
    void GUI_SAFE_002_null_figure_deletion_is_a_noop();
    void GUI_SAFE_003_extensionless_form_path_is_rejected_safely();
    void GUI_SAFE_004_null_dock_cleanup_is_a_noop();
    void GUI_SAFE_005_orphaned_form_record_close_project_is_safe();
    void GUI_SAFE_006_output_context_actions_are_not_retained();
    void GUI_SAFE_007_failed_form_load_does_not_retain_a_toplevel_tab();
    void cleanup();
    void cleanupTestCase();

private:
    struct ModalResult {
        int count = 0;
        QStringList titles;
        QPointer<QDialog> lastDialog;
    };

    QString fixturePath(const QString& fileName) const;
    QDockWidget* dynamicDock(MainWindow& window, const QString& name) const;
    QLineEditD* loadedLine(QDockWidget* dock, const QString& originalName, const QString& formName) const;
    ModalResult triggerAndRejectModal(QAction* action,
        QMessageBox::StandardButton messageBoxButton = QMessageBox::NoButton) const;
    QString originalWorkingDirectory;
};

QString MainWindowIntegrationTests::fixturePath(const QString& fileName) const
{
    const QString repositoryRoot = qEnvironmentVariable("LABANALYSER_TEST_REPOSITORY_ROOT");
    if (!repositoryRoot.isEmpty())
        return QDir::cleanPath(repositoryRoot + "/tests/fixtures/gui/" + fileName);
    return QDir::cleanPath(QCoreApplication::applicationDirPath()
        + "/../../../tests/fixtures/gui/" + fileName);
}

QDockWidget* MainWindowIntegrationTests::dynamicDock(MainWindow& window, const QString& name) const
{
    QDockWidget* dock = window.findChild<QDockWidget*>(name);
    if (dock && window.dockWidgetArea(dock) != Qt::LeftDockWidgetArea)
        return nullptr;
    return dock;
}

QLineEditD* MainWindowIntegrationTests::loadedLine(QDockWidget* dock, const QString& originalName, const QString& formName) const
{
    if (!dock)
        return nullptr;
    QLineEditD* line = dock->findChild<QLineEditD*>(originalName + "_" + formName);
    return line;
}

MainWindowIntegrationTests::ModalResult MainWindowIntegrationTests::triggerAndRejectModal(
    QAction* action, QMessageBox::StandardButton messageBoxButton) const
{
    ModalResult result;
    QTimer modalCloser;
    modalCloser.setInterval(0);
    connect(&modalCloser, &QTimer::timeout, [&] {
        QDialog* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        ++result.count;
        result.titles << dialog->windowTitle();
        result.lastDialog = dialog;
        modalCloser.stop();
        if (QMessageBox* box = qobject_cast<QMessageBox*>(dialog)) {
            if (messageBoxButton != QMessageBox::NoButton) {
                if (QAbstractButton* button = box->button(messageBoxButton)) {
                    QMetaObject::invokeMethod(button, "click", Qt::DirectConnection);
                    return;
                }
            }
        }
        dialog->reject();
    });
    modalCloser.start();
    action->trigger();
    modalCloser.stop();
    return result;
}

void MainWindowIntegrationTests::initTestCase()
{
    QCOMPARE(qEnvironmentVariable("QT_QPA_PLATFORM"), QString("offscreen"));
    QCOMPARE(QSettings::defaultFormat(), QSettings::IniFormat);
    QVERIFY(QStandardPaths::isTestModeEnabled());
    originalWorkingDirectory = qEnvironmentVariable("LABANALYSER_TEST_WORKING_ROOT");
    QVERIFY(QDir(originalWorkingDirectory).exists());
}

void MainWindowIntegrationTests::GUI_001_construct_destroy_and_isolated_settings()
{
    QSettings settings;
    QVERIFY(settings.fileName().startsWith(qEnvironmentVariable("LABANALYSER_TEST_SETTINGS_ROOT")));
    QPointer<MainWindow> window(new MainWindow);
    QVERIFY(window);
    QCOMPARE(window->GetLogic()->parent(), window.data());
    QCOMPARE(window->GetLogic()->GetContainerCount(), 0);
    delete window;
    QVERIFY(window.isNull());
    for (QWidget* topLevel : QApplication::topLevelWidgets())
        QVERIFY(qobject_cast<MainWindow*>(topLevel) == nullptr);
}

void MainWindowIntegrationTests::GUI_002_identity_hierarchy_menus_actions_and_defaults()
{
    MainWindow window;
    QCOMPARE(window.objectName(), QString("LabAnalyser"));
    QCOMPARE(window.windowTitle(), QString("LabAnalyser"));
    QVERIFY(window.centralWidget());
    QCOMPARE(window.centralWidget()->objectName(), QString("centralWidget"));
    QCOMPARE(window.GetStatusBar()->objectName(), QString("statusBar"));
    QMenuBar* menuBar = window.findChild<QMenuBar*>("menuBarI"); QVERIFY(menuBar);
    QCOMPARE(menuBar->findChildren<QMenu*>().size(), 3);
    QToolBar* toolBar = window.findChild<QToolBar*>("toolBar"); QVERIFY(toolBar);
    QCOMPARE(toolBar->iconSize(), QSize(50, 24));
    const QStringList dockNames {"ParameterDock", "DataDock", "StateDock", "OutputDock"};
    for (const QString& name : dockNames) {
        QDockWidget* dock = window.findChild<QDockWidget*>(name); QVERIFY2(dock, qPrintable(name));
        QCOMPARE(window.dockWidgetArea(dock), Qt::RightDockWidgetArea); QVERIFY(dock->features().testFlag(QDockWidget::DockWidgetMovable)); QVERIFY(dock->features().testFlag(QDockWidget::DockWidgetFloatable)); QVERIFY(!dock->features().testFlag(QDockWidget::DockWidgetClosable));
    }
    const QStringList actionNames {"actionBeenden", "actionLoad_Form", "actionCreatePlot", "actionCreate_Subplot", "actionLoadPlugin", "actionSave_Experiment", "actionLoadExperiment", "Close_Project", "actionDaten_Exportieren_mat", "actionSave", "actionMinimize_to_Tray", "actionAbout_LabAnalyzer", "actionLoad_Parameter_File", "actionSave_Parameter_Set", "actionAbout", "actionExport_Data_h5", "actionRemote_Connection_Port_2", "actionFFT"};
    for (const QString& name : actionNames) { QAction* action = window.findChild<QAction*>(name); QVERIFY2(action, qPrintable(name)); QVERIFY(action->isEnabled()); QVERIFY(!action->isCheckable()); QVERIFY(!action->isChecked()); }
    QCOMPARE(window.findChild<QAction*>("actionBeenden")->text(), QString("Quit"));
    QCOMPARE(window.findChild<QAction*>("actionCreatePlot")->text(), QString("Plot"));
}

void MainWindowIntegrationTests::GUI_003_safe_action_connections_and_project_reset()
{
    MainWindow window;
    QAction* closeProject = window.findChild<QAction*>("Close_Project"); QVERIFY(closeProject);
    QSignalSpy triggered(closeProject, &QAction::triggered); closeProject->trigger(); QCOMPARE(triggered.count(), 1);
    QCOMPARE(window.GetLogic()->GetContainerCount(), 0); QVERIFY(window.SavePath.isEmpty()); QVERIFY(!window.ChangeForSaveDetected);
    QAction* minimize = window.findChild<QAction*>("actionMinimize_to_Tray"); QVERIFY(minimize);
    window.show(); QTRY_VERIFY(window.isVisible()); minimize->trigger(); QTRY_VERIFY(!window.isVisible());
    window.TrayIconActivated(QSystemTrayIcon::DoubleClick); QTRY_VERIFY(window.isVisible());
}

void MainWindowIntegrationTests::GUI_004_docks_visibility_and_floating_lifecycle()
{
    MainWindow window; window.show(); QTRY_VERIFY(window.isVisible());
    QDockWidget* parameter = window.findChild<QDockWidget*>("ParameterDock"); QVERIFY(parameter);
    parameter->hide(); QTRY_VERIFY(!parameter->isVisible()); parameter->show(); QTRY_VERIFY(parameter->isVisible());
    parameter->setFloating(true); QTRY_VERIFY(parameter->isFloating()); QVERIFY(parameter->windowFlags().testFlag(Qt::Window));
    parameter->setFloating(false); QTRY_VERIFY(!parameter->isFloating()); QCOMPARE(window.dockWidgetArea(parameter), Qt::RightDockWidgetArea);
}

void MainWindowIntegrationTests::GUI_005_plot_subwindows_repeat_close_and_shutdown()
{
    MainWindow window;
    QPointer<SubPlotMainWindow> first(window.CreateSubPlotWindow(1, 1)); QVERIFY(first); QCOMPARE(first->objectName(), QString("Figure#0")); QCOMPARE(first->windowTitle(), QString("Figure 1"));
    QPointer<SubPlotMainWindow> second(window.CreateSubPlotWindow(1, 1)); QVERIFY(second); QCOMPARE(second->objectName(), QString("Figure#1")); QCOMPARE(second->windowTitle(), QString("Figure 2"));
    first->close(); second->close(); QTRY_VERIFY(first.isNull()); QTRY_VERIFY(second.isNull());
}

void MainWindowIntegrationTests::GUI_006_dynamic_form_docks_repeat_ownership_and_loader_errors()
{
    MainWindow window;
    const QString bindings = fixturePath("mainwindow-bindings.ui");
    const QString standard = fixturePath("standard-widgets.ui");
    QVERIFY2(QFileInfo::exists(bindings), qPrintable(bindings));
    QVERIFY2(QFileInfo::exists(standard), qPrintable(standard));

    const QString workingDirectoryBeforeLoad = QDir::currentPath();
    window.LoadFormFromXML(bindings, "GUI6_A", true);
    QVERIFY(QDir::currentPath() != workingDirectoryBeforeLoad);
    QCOMPARE(QDir::currentPath(), QFileInfo(bindings).absolutePath());
    window.LoadFormFromXML(bindings, "GUI6_B", true);
    window.LoadFormFromXML(standard, "GUI6_C", false);
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 3);
    QCOMPARE(window.GetLogic()->GetFormFileEntry(0).first, QString("GUI6_A"));
    QCOMPARE(window.GetLogic()->GetFormFileEntry(0).second, bindings);
    QVERIFY(window.GetLogic()->GetSkipFormFile("GUI6_A"));
    QCOMPARE(window.GetLogic()->GetFormFileEntry(2).first, QString("GUI6_C"));
    QCOMPARE(window.GetLogic()->GetFormFileEntry(2).second, standard);
    QVERIFY(!window.GetLogic()->GetSkipFormFile("GUI6_C"));

    QDockWidget* first = dynamicDock(window, "GUI6_A");
    QDockWidget* second = dynamicDock(window, "GUI6_B");
    QDockWidget* third = dynamicDock(window, "GUI6_C");
    QVERIFY(first);
    QVERIFY(second);
    QVERIFY(third);
    QCOMPARE(first->windowTitle(), QString("mainwindow-bindings"));
    QCOMPARE(third->windowTitle(), QString("standard-widgets"));
    QVERIFY(first->features().testFlag(QDockWidget::DockWidgetClosable));
    QVERIFY(first->widget());
    QCOMPARE(first->widget()->objectName(), QString("GUI6_A"));
    QVERIFY(first->findChild<QScrollArea*>());
    QLineEditD* firstLine = loadedLine(first, "primaryValue", "GUI6_A");
    QLineEditD* secondLine = loadedLine(second, "primaryValue", "GUI6_B");
    QVERIFY(firstLine);
    QVERIFY(secondLine);
    QVERIFY(first->widget()->isAncestorOf(firstLine));
    QCOMPARE(secondLine->objectName(), QString("primaryValue_GUI6_B"));

    QPointer<QDockWidget> removed(first);
    first->close();
    QTRY_VERIFY(removed.isNull());
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 2);
    window.LoadFormFromXML(bindings, "GUI6_A_again", true);
    QVERIFY(dynamicDock(window, "GUI6_A_again"));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 3);

    QSignalSpy errors(window.GetLogic()->GetMessenger(), &MessengerClass::ErrorWriter);
    window.LoadFormFromXML(fixturePath("missing.ui"), "GUI6_Missing", true);
    QCOMPARE(errors.count(), 1);
    QCOMPARE(errors.at(0).at(1).toString(), QString("Corrupt Form File"));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 3);
    window.LoadFormFromXML(fixturePath("malformed.ui"), "GUI6_Malformed", true);
    QCOMPARE(errors.count(), 2);
    QCOMPARE(errors.at(1).at(1).toString(), QString("Corrupt Form File"));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 3);

    window.LoadFormFromXML(fixturePath("unsupported-widget.ui"), "GUI6_Unsupported", true);
    QDockWidget* unsupported = dynamicDock(window, "GUI6_Unsupported");
    QVERIFY(unsupported);
    QVERIFY(!unsupported->findChild<QWidget*>("unsupported"));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 4);
}

void MainWindowIntegrationTests::GUI_007_loaded_dropwidgets_manager_messenger_and_close_project()
{
    MainWindow window;
    const QString bindings = fixturePath("mainwindow-bindings.ui");
    window.LoadFormFromXML(bindings, "GUI7_A", true);
    window.LoadFormFromXML(bindings, "GUI7_B", true);
    QDockWidget* dockA = dynamicDock(window, "GUI7_A");
    QDockWidget* dockB = dynamicDock(window, "GUI7_B");
    QVERIFY(dockA);
    QVERIFY(dockB);
    QLineEditD* first = loadedLine(dockA, "primaryValue", "GUI7_A");
    QLineEditD* second = loadedLine(dockB, "primaryValue", "GUI7_B");
    QVERIFY(first);
    QVERIFY(second);
    const QString id("GUI7::sharedText");
    auto* manager = window.GetLogic();
    manager->AddContainerElement(id, "QString", "Parameter", "");
    manager->AddElementToContainerEntry(first->objectName(), id, first->metaObject()->className(), first);
    manager->AddElementToContainerEntry(second->objectName(), id, second->metaObject()->className(), second);

    QSignalSpy messages(manager, &DataManagementClass::MessageSender);
    first->ConnectToID(manager, id);
    second->ConnectToID(manager, id);
    QCOMPARE(messages.count(), 2);
    QCOMPARE(messages.at(0).at(0).toString(), QString("get"));
    QCOMPARE(messages.at(0).at(1).toString(), id);
    QCOMPARE(messages.at(1).at(0).toString(), QString("get"));
    QCOMPARE(messages.at(1).at(1).toString(), id);
    QCOMPARE(manager->GetContainerID(first), id);
    QCOMPARE(manager->GetContainerID(second), id);

    InterfaceData incoming("", "Parameter");
    incoming.SetData(QString("from-manager"));
    manager->GetMessenger()->MessageReceiver("set", id, incoming);
    QCOMPARE(first->text(), QString("from-manager"));
    QCOMPARE(second->text(), QString("from-manager"));

    messages.clear();
    first->setText("from-widget");
    QVERIFY(QMetaObject::invokeMethod(first, "editingFinished"));
    QCOMPARE(messages.count(), 1);
    QCOMPARE(messages.at(0).at(0).toString(), QString("set"));
    QCOMPARE(messages.at(0).at(1).toString(), id);
    QCOMPARE(messages.at(0).at(2).value<InterfaceData>().GetString(), QString("from-widget"));
    QCOMPARE(manager->GetContainer(id)->GetString(), QString("from-widget"));

    QPointer<QDockWidget> firstDock(dockA);
    QPointer<QDockWidget> secondDock(dockB);
    window.ChangeForSaveDetected = false;
    window.CloseProject();
    QTRY_VERIFY(firstDock.isNull());
    QTRY_VERIFY(secondDock.isNull());
    QCOMPARE(manager->GetFormFileCount(), 0);
    QVERIFY(window.centralWidget()->isVisible());
    QCOMPARE(manager->GetContainerCount(), 0);
}

void MainWindowIntegrationTests::GUI_008_loaded_form_direct_drag_events_are_rejected_without_source()
{
    MainWindow window;
    window.LoadFormFromXML(fixturePath("mainwindow-bindings.ui"), "GUI8", true);
    QDockWidget* dock = dynamicDock(window, "GUI8");
    QVERIFY(dock);
    QLineEditD* target = loadedLine(dock, "primaryValue", "GUI8");
    QVERIFY(target);
    QMimeData data;
    data.setText("GUI8::candidate");
    QDragEnterEvent dragEnter(QPoint(1, 1), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(target, &dragEnter);
    QVERIFY(!dragEnter.isAccepted());
    QDropEvent drop(QPointF(1, 1), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(target, &drop);
    QVERIFY(!drop.isAccepted());
    QVERIFY(window.GetLogic()->GetContainerID(target).isEmpty());
}

void MainWindowIntegrationTests::GUI_009_modal_actions_open_once_and_abort_without_output()
{
    MainWindow window;
    QTemporaryDir outputDirectory;
    QVERIFY(outputDirectory.isValid());
    const QString rememberedPath = outputDirectory.filePath("remembered");
    QVERIFY(QDir().mkpath(rememberedPath));
    window.StdSavePath = rememberedPath;
    const QString cwdBefore = QDir::currentPath();

    const QStringList fileDialogActions {
        "actionLoad_Form", "actionLoadExperiment", "actionSave_Experiment",
        "actionSave", "actionLoadPlugin", "actionLoad_Parameter_File"
    };
    for (const QString& actionName : fileDialogActions) {
        QAction* action = window.findChild<QAction*>(actionName);
        QVERIFY2(action, qPrintable(actionName));
        QSignalSpy triggered(action, &QAction::triggered);
        const ModalResult dialog = triggerAndRejectModal(action);
        QCOMPARE(triggered.count(), 1);
        QCOMPARE(dialog.count, 1);
        QVERIFY(!dialog.titles.constFirst().isEmpty());
        QCOMPARE(QDir::currentPath(), cwdBefore);
    }

    const QString id("GUI9::parameter");
    InterfaceData parameter("double", "Parameter");
    parameter.SetData(1.0);
    window.GetLogic()->AddContainerElement(id, "double", "Parameter", "");
    window.AddElementToWidget(id, parameter);
    QTreeWidget* parameterTree = window.findChild<QTreeWidget*>("ParameterTreeWidget");
    QVERIFY(parameterTree);
    QVERIFY(parameterTree->topLevelItemCount() == 1);
    QTreeWidgetItem* leaf = parameterTree->topLevelItem(0)->child(0);
    QVERIFY(leaf);
    parameterTree->setCurrentItem(leaf, QItemSelectionModel::ClearAndSelect);
    for (const QString& actionName : {QString("actionDaten_Exportieren_mat"),
                                      QString("actionExport_Data_h5"),
                                      QString("actionSave_Parameter_Set")}) {
        QAction* action = window.findChild<QAction*>(actionName);
        QVERIFY2(action, qPrintable(actionName));
        QSignalSpy triggered(action, &QAction::triggered);
        const ModalResult dialog = triggerAndRejectModal(action);
        QCOMPARE(triggered.count(), 1);
        QCOMPARE(dialog.count, 1);
        QCOMPARE(QDir::currentPath(), cwdBefore);
    }

    QAction* about = window.findChild<QAction*>("actionAbout");
    QVERIFY(about);
    QSignalSpy aboutTriggered(about, &QAction::triggered);
    ModalResult aboutDialog = triggerAndRejectModal(about);
    QCOMPARE(aboutTriggered.count(), 1);
    QCOMPARE(aboutDialog.count, 1);
    QCOMPARE(aboutDialog.titles, QStringList({"Dialog"}));
    QVERIFY(aboutDialog.lastDialog.isNull());

    QAction* remote = window.findChild<QAction*>("actionRemote_Connection_Port_2");
    QVERIFY(remote);
    QSignalSpy remoteTriggered(remote, &QAction::triggered);
    const ModalResult remoteDialog = triggerAndRejectModal(remote, QMessageBox::Ok);
    QCOMPARE(remoteTriggered.count(), 1);
    QCOMPARE(remoteDialog.count, 1);
    QCOMPARE(remoteDialog.titles, QStringList({"Remote Connection Port"}));

    QAction* aboutLabAnalyser = window.findChild<QAction*>("actionAbout_LabAnalyzer");
    QVERIFY(aboutLabAnalyser);
    QSignalSpy emptyAboutTriggered(aboutLabAnalyser, &QAction::triggered);
    const ModalResult emptyAbout = triggerAndRejectModal(aboutLabAnalyser);
    QCOMPARE(emptyAboutTriggered.count(), 1);
    QCOMPARE(emptyAbout.count, 0);

    QCOMPARE(QDir(outputDirectory.path()).entryList(QDir::Files), QStringList());
    QCOMPARE(QDir::currentPath(), cwdBefore);
}

void MainWindowIntegrationTests::GUI_010_cancelled_file_dialogs_currently_mutate_selected_state()
{
    MainWindow window;
    QTemporaryDir outputDirectory;
    QVERIFY(outputDirectory.isValid());
    const QString rememberedPath = outputDirectory.filePath("remembered");
    QVERIFY(QDir().mkpath(rememberedPath));
    window.StdSavePath = rememberedPath;
    window.ChangeForSaveDetected = true;
    const QString cwdBefore = QDir::currentPath();

    QAction* saveAs = window.findChild<QAction*>("actionSave_Experiment");
    QVERIFY(saveAs);
    QSignalSpy saveRequests(&window, &MainWindow::SaveExperiment);
    QCOMPARE(triggerAndRejectModal(saveAs).count, 1);
    QCOMPARE(saveRequests.count(), 1);
    QCOMPARE(saveRequests.at(0).at(0).toString(), QString());
    QVERIFY(!window.ChangeForSaveDetected);
    QCOMPARE(window.StdSavePath, QString());

    window.SavePath.clear();
    QAction* save = window.findChild<QAction*>("actionSave");
    QVERIFY(save);
    QCOMPARE(triggerAndRejectModal(save).count, 1);
    QCOMPARE(saveRequests.count(), 2);
    QCOMPARE(saveRequests.at(1).at(0).toString(), QString());

    window.StdSavePath = rememberedPath;
    QAction* loadParameters = window.findChild<QAction*>("actionLoad_Parameter_File");
    QVERIFY(loadParameters);
    QCOMPARE(triggerAndRejectModal(loadParameters).count, 1);
    QCOMPARE(window.StdSavePath, QString());
    QCOMPARE(QDir::currentPath(), cwdBefore);
    QCOMPARE(QDir(outputDirectory.path()).entryList(QDir::Files), QStringList());
}

void MainWindowIntegrationTests::GUI_011_unsaved_close_project_cancel_and_discard()
{
    const QString bindings = fixturePath("mainwindow-bindings.ui");
    MainWindow cancelledWindow;
    cancelledWindow.LoadFormFromXML(bindings, "GUI11_Cancel", true);
    QPointer<QDockWidget> cancelledDock(dynamicDock(cancelledWindow, "GUI11_Cancel"));
    QVERIFY(cancelledDock);
    cancelledWindow.ChangeForSaveDetected = true;
    QAction* closeAction = cancelledWindow.findChild<QAction*>("Close_Project");
    QVERIFY(closeAction);
    QSignalSpy cancelledTriggered(closeAction, &QAction::triggered);
    const ModalResult cancelDialog = triggerAndRejectModal(closeAction, QMessageBox::Cancel);
    QCOMPARE(cancelledTriggered.count(), 1);
    QCOMPARE(cancelDialog.count, 1);
    QCOMPARE(cancelDialog.titles, QStringList({"Close Project"}));
    QVERIFY(cancelledDock);
    QCOMPARE(cancelledWindow.GetLogic()->GetFormFileCount(), 1);
    QVERIFY(!cancelledWindow.ChangeForSaveDetected);

    cancelledWindow.ChangeForSaveDetected = true;
    const ModalResult repeatedCancel = triggerAndRejectModal(closeAction, QMessageBox::Cancel);
    QCOMPARE(repeatedCancel.count, 1);
    QVERIFY(cancelledDock);
    QCOMPARE(cancelledWindow.GetLogic()->GetFormFileCount(), 1);

    MainWindow discardedWindow;
    discardedWindow.LoadFormFromXML(bindings, "GUI11_Discard", true);
    QPointer<QDockWidget> discardedDock(dynamicDock(discardedWindow, "GUI11_Discard"));
    QVERIFY(discardedDock);
    discardedWindow.ChangeForSaveDetected = true;
    QAction* discardAction = discardedWindow.findChild<QAction*>("Close_Project");
    QVERIFY(discardAction);
    QSignalSpy discardedTriggered(discardAction, &QAction::triggered);
    const ModalResult discardDialog = triggerAndRejectModal(discardAction, QMessageBox::Discard);
    QCOMPARE(discardedTriggered.count(), 1);
    QCOMPARE(discardDialog.count, 1);
    QTRY_VERIFY(discardedDock.isNull());
    QCOMPARE(discardedWindow.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(discardedWindow.GetLogic()->GetContainerCount(), 0);
}

void MainWindowIntegrationTests::GUI_012_subplot_action_cancel_valid_sizes_and_repeat()
{
    MainWindow window;
    QAction* action = window.findChild<QAction*>("actionCreate_Subplot");
    QVERIFY(action);
    const int initialFigures = window.findChildren<SubPlotMainWindow*>().size();
    const int topLevelCount = QApplication::topLevelWidgets().size();
    auto triggerSubplot = [&](bool accept, const QString& rows, const QString& columns) {
        int dialogs = 0;
        QTimer dialogDriver;
        dialogDriver.setInterval(0);
        connect(&dialogDriver, &QTimer::timeout, [&] {
            QDialog* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            if (!dialog)
                return;
            ++dialogs;
            dialogDriver.stop();
            if (!accept) {
                dialog->reject();
                return;
            }
            const QList<QComboBox*> combos = dialog->findChildren<QComboBox*>();
            QCOMPARE(combos.size(), 2);
            combos.at(0)->setCurrentText(rows);
            combos.at(1)->setCurrentText(columns);
            QDialogButtonBox* buttons = dialog->findChild<QDialogButtonBox*>();
            QVERIFY(buttons);
            QAbstractButton* ok = buttons->button(QDialogButtonBox::Ok);
            QVERIFY(ok);
            QMetaObject::invokeMethod(ok, "click", Qt::DirectConnection);
        });
        dialogDriver.start();
        action->trigger();
        dialogDriver.stop();
        return dialogs;
    };

    QCOMPARE(triggerSubplot(false, QString(), QString()), 1);
    QCOMPARE(window.findChildren<SubPlotMainWindow*>().size(), initialFigures);
    QCOMPARE(QApplication::topLevelWidgets().size(), topLevelCount);

    QCOMPARE(triggerSubplot(true, "2", "3"), 1);
    QList<SubPlotMainWindow*> figures = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(figures.size(), initialFigures + 1);
    QPointer<SubPlotMainWindow> first(figures.last());
    QVERIFY(first);
    first->close();
    QTRY_VERIFY(first.isNull());

    QCOMPARE(triggerSubplot(true, "1", "2"), 1);
    figures = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(figures.size(), initialFigures + 1);
    QPointer<SubPlotMainWindow> second(figures.last());
    QVERIFY(second);
    second->close();
    QTRY_VERIFY(second.isNull());
}

void MainWindowIntegrationTests::GUI_013_tray_restore_and_output_menu_contract()
{
    MainWindow window;
    QAction* minimize = window.findChild<QAction*>("actionMinimize_to_Tray");
    QVERIFY(minimize);
    QAction* restore = nullptr;
    for (QAction* candidate : window.findChildren<QAction*>()) {
        if (candidate->text() == "Restore") {
            restore = candidate;
            break;
        }
    }
    QVERIFY(restore);
    for (int cycle = 0; cycle < 2; ++cycle) {
        window.show();
        QTRY_VERIFY(window.isVisible());
        minimize->trigger();
        QTRY_VERIFY(!window.isVisible());
        QVERIFY(restore->isEnabled());
        restore->trigger();
        QTRY_VERIFY(window.isVisible());
        minimize->trigger();
        QTRY_VERIFY(!window.isVisible());
        window.TrayIconActivated(QSystemTrayIcon::DoubleClick);
        QTRY_VERIFY(window.isVisible());
        QVERIFY(!restore->isEnabled());
    }

    QPlainTextEdit* output = window.findChild<QPlainTextEdit*>("OutputText");
    QVERIFY(output);
    output->setPlainText("seed");
    window.ErrorWriter("GUI13", QString(10001, QLatin1Char('x')));
    QVERIFY(output->toPlainText().contains("Data was trimmed by LabAnalyser"));
    for (int i = 0; i < 110; ++i)
        window.InfoWriter("GUI13", QString("entry-%1").arg(i));
    QVERIFY(output->document()->blockCount() <= 100);

    int menus = 0;
    bool clearActionSeen = false;
    QTimer menuDriver;
    menuDriver.setInterval(0);
    connect(&menuDriver, &QTimer::timeout, [&] {
        QMenu* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
        if (!menu)
            return;
        ++menus;
        menuDriver.stop();
        QAction* clear = nullptr;
        for (QAction* candidate : menu->actions()) {
            if (candidate->text() == "Clear Output") {
                clear = candidate;
                break;
            }
        }
        QVERIFY(clear);
        clearActionSeen = true;
        clear->trigger();
        menu->close();
    });
    menuDriver.start();
    window.OutputTextMenu(QPoint(1, 1));
    menuDriver.stop();
    QCOMPARE(menus, 1);
    QVERIFY(clearActionSeen);
    QVERIFY(output->toPlainText().isEmpty());
}

void MainWindowIntegrationTests::GUI_014_nested_trees_messenger_updates_and_valid_highlighting()
{
    MainWindow window;
    auto* manager = window.GetLogic();
    struct TreeCase { QString id; QString category; QString dataType; QString value; QString dock; };
    const QList<TreeCase> cases {
        {"Parameter::Group::Gain", "Parameter", "double", "1.5", "ParameterDock"},
        {"Data::Group::Samples", "Data", "QString", "initial", "DataDock"},
        {"State::Group::Mode", "State", "int32_t", "2", "StateDock"}
    };
    for (const TreeCase& treeCase : cases) {
        InterfaceData data(treeCase.dataType, treeCase.category);
        if (treeCase.dataType == "double") data.SetData(1.5);
        else if (treeCase.dataType == "int32_t") data.SetData(int32_t(2));
        else data.SetData(treeCase.value);
        manager->GetMessenger()->MessageReceiver("publish", treeCase.id, data);
    }

    const QList<QPair<QString, QString>> treeNames {
        {"ParameterTreeWidget", "Parameter"}, {"DataTreeWidget", "Data"}, {"StateTreeWidget", "State"}
    };
    for (const auto& treeName : treeNames) {
        QTreeWidget* tree = window.findChild<QTreeWidget*>(treeName.first);
        QVERIFY(tree);
        QCOMPARE(tree->topLevelItemCount(), 1);
        QTreeWidgetItem* category = tree->topLevelItem(0);
        QCOMPARE(category->text(0), treeName.second);
        QCOMPARE(category->childCount(), 1);
        QTreeWidgetItem* group = category->child(0);
        QCOMPARE(group->text(0), QString("Group"));
        QCOMPARE(group->childCount(), 1);
        QVERIFY(group->child(0)->childCount() == 0);
    }

    QTreeWidget* parameterTree = window.findChild<QTreeWidget*>("ParameterTreeWidget");
    QTreeWidgetItem* gain = parameterTree->topLevelItem(0)->child(0)->child(0);
    QCOMPARE(gain->text(0), QString("Gain"));
    QCOMPARE(gain->text(1), QString("double"));
    InterfaceData update("double", "Parameter");
    update.SetData(2.5);
    manager->GetMessenger()->MessageReceiver("set", "Parameter::Group::Gain", update);
    QCOMPARE(manager->GetContainer("Parameter::Group::Gain")->GetDouble(), 2.5);
    window.AddElementToWidget("Parameter::Group::Gain", update);
    QCOMPARE(parameterTree->topLevelItemCount(), 1);
    QCOMPARE(parameterTree->topLevelItem(0)->child(0)->childCount(), 1);

    for (const TreeCase& treeCase : cases) {
        window.HighLightConnection(treeCase.id);
        const QString treeObject = treeCase.category + "TreeWidget";
        QTreeWidget* tree = window.findChild<QTreeWidget*>(treeObject);
        QVERIFY(tree);
        QCOMPARE(tree->currentItem()->text(0), treeCase.id.split("::").last());
        QDockWidget* dock = window.findChild<QDockWidget*>(treeCase.dock);
        QVERIFY(dock);
    }
}

void MainWindowIntegrationTests::GUI_015_form_dock_recreate_from_temporary_fixture()
{
    MainWindow window;
    const QString form = fixturePath("mainwindow-bindings.ui");
    window.LoadFormFromXML(form, "GUI15_First", true);
    QPointer<QDockWidget> first(dynamicDock(window, "GUI15_First"));
    QVERIFY(first);
    QVERIFY(!window.centralWidget()->isVisible());
    first->close();
    QTRY_VERIFY(first.isNull());
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QVERIFY(window.centralWidget()->isVisible());

    window.LoadFormFromXML(form, "GUI15_Restored", true);
    QDockWidget* restored = dynamicDock(window, "GUI15_Restored");
    QVERIFY(restored);
    QCOMPARE(restored->windowTitle(), QString("mainwindow-bindings"));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 1);
    QCOMPARE(window.GetLogic()->GetFormFileEntry(0).first, QString("GUI15_Restored"));
    QCOMPARE(window.GetLogic()->GetFormFileEntry(0).second, form);
    QVERIFY(window.GetLogic()->GetSkipFormFile("GUI15_Restored"));
    QVERIFY(loadedLine(restored, "primaryValue", "GUI15_Restored"));
}

void MainWindowIntegrationTests::GUI_016_valid_leaf_context_actions_and_multi_selection()
{
    MainWindow window;
    auto* manager = window.GetLogic();
    auto publish = [&](const QString& id, const QString& type, const QString& category, const QVariant& value) {
        InterfaceData data(type, category);
        if (type == "double")
            data.SetData(value.toDouble());
        else
            data.SetData(value.toString());
        manager->GetMessenger()->MessageReceiver("publish", id, data);
    };
    publish("Parameter::Device::Gain", "double", "Parameter", 1.0);
    publish("Data::Device::First", "QString", "Data", "one");
    publish("Data::Device::Second", "QString", "Data", "two");
    publish("State::Device::Mode", "QString", "State", "ready");

    QTreeWidget* parameterTree = window.findChild<QTreeWidget*>("ParameterTreeWidget");
    QTreeWidget* dataTree = window.findChild<QTreeWidget*>("DataTreeWidget");
    QTreeWidget* stateTree = window.findChild<QTreeWidget*>("StateTreeWidget");
    QVERIFY(parameterTree);
    QVERIFY(dataTree);
    QVERIFY(stateTree);
    QTreeWidgetItem* parameterLeaf = parameterTree->topLevelItem(0)->child(0)->child(0);
    QTreeWidgetItem* firstDataLeaf = dataTree->topLevelItem(0)->child(0)->child(0);
    QTreeWidgetItem* secondDataLeaf = dataTree->topLevelItem(0)->child(0)->child(1);
    QTreeWidgetItem* stateLeaf = stateTree->topLevelItem(0)->child(0)->child(0);
    QVERIFY(parameterLeaf);
    QVERIFY(firstDataLeaf);
    QVERIFY(secondDataLeaf);
    QVERIFY(stateLeaf);

    auto closePopup = [] {
        if (QWidget* popup = QApplication::activePopupWidget())
            popup->close();
    };
    parameterTree->setCurrentItem(parameterLeaf, QItemSelectionModel::ClearAndSelect);
    QVERIFY(QMetaObject::invokeMethod(&window, "contextMenuTreeWidget", Qt::DirectConnection,
                                      Q_ARG(QPoint, QPoint(1, 1))));
    QTRY_VERIFY(QApplication::activePopupWidget());
    QMenu* parameterMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    QVERIFY(parameterMenu);
    QCOMPARE(parameterMenu->actions().size(), 1);
    QCOMPARE(parameterMenu->actions().constFirst()->text(), QString("Change Min/Max Values"));
    QVERIFY(parameterMenu->actions().constFirst()->isVisible());
    QVERIFY(parameterMenu->actions().constFirst()->isEnabled());
    closePopup();

    dataTree->setCurrentItem(firstDataLeaf, QItemSelectionModel::ClearAndSelect);
    QVERIFY(QMetaObject::invokeMethod(&window, "contextMenuTreeWidgetData", Qt::DirectConnection,
                                      Q_ARG(QPoint, QPoint(1, 1))));
    QTRY_VERIFY(QApplication::activePopupWidget());
    QMenu* dataMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    QVERIFY(dataMenu);
    QCOMPARE(dataMenu->actions().size(), 1);
    QCOMPARE(dataMenu->actions().constFirst()->text(), QString("Set Alias"));
    QVERIFY(dataMenu->actions().constFirst()->isVisible());
    QVERIFY(dataMenu->actions().constFirst()->isEnabled());
    int aliasDialogs = 0;
    QTimer aliasDriver;
    aliasDriver.setInterval(0);
    connect(&aliasDriver, &QTimer::timeout, [&] {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        ++aliasDialogs;
        aliasDriver.stop();
        dialog->setTextValue("First Alias");
        dialog->accept();
    });
    aliasDriver.start();
    dataMenu->actions().constFirst()->trigger();
    aliasDriver.stop();
    QCOMPARE(aliasDialogs, 1);
    QCOMPARE(manager->GetAlias("Data::Device::First"), QString("First Alias"));

    dataTree->setCurrentItem(firstDataLeaf, QItemSelectionModel::ClearAndSelect);
    QVERIFY(QMetaObject::invokeMethod(&window, "contextMenuTreeWidgetData", Qt::DirectConnection,
                                      Q_ARG(QPoint, QPoint(1, 1))));
    QTRY_VERIFY(QApplication::activePopupWidget());
    dataMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    QVERIFY(dataMenu);
    QCOMPARE(dataMenu->actions().size(), 2);
    QCOMPARE(dataMenu->actions().at(0)->text(), QString("Set Alias"));
    QCOMPARE(dataMenu->actions().at(1)->text(), QString("Remove Alias"));
    closePopup();

    dataTree->setCurrentItem(firstDataLeaf, QItemSelectionModel::ClearAndSelect);
    dataTree->selectionModel()->select(dataTree->indexFromItem(secondDataLeaf), QItemSelectionModel::Select);
    QCOMPARE(dataTree->selectedItems().size(), 2);
    QVERIFY(QMetaObject::invokeMethod(&window, "contextMenuTreeWidgetData", Qt::DirectConnection,
                                      Q_ARG(QPoint, QPoint(1, 1))));
    QTRY_VERIFY(QApplication::activePopupWidget());
    dataMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    QVERIFY(dataMenu);
    QCOMPARE(dataMenu->actions().size(), 1);
    QCOMPARE(dataMenu->actions().constFirst()->text(), QString("Set Alias"));
    closePopup();

    stateTree->setCurrentItem(stateLeaf, QItemSelectionModel::ClearAndSelect);
    QVERIFY(QMetaObject::invokeMethod(&window, "contextMenuTreeWidgetState", Qt::DirectConnection,
                                      Q_ARG(QPoint, QPoint(1, 1))));
    QCoreApplication::processEvents();
    QVERIFY(!QApplication::activePopupWidget());
    QVERIFY(QMetaObject::invokeMethod(&window, "on_StateTreeWidget_customContextMenuRequested", Qt::DirectConnection,
                                      Q_ARG(QPoint, QPoint(1, 1))));
    QCoreApplication::processEvents();
    QVERIFY(!QApplication::activePopupWidget());
}

void MainWindowIntegrationTests::GUI_017_plot_action_routing_and_repeat_targets()
{
    MainWindow window;
    auto* manager = window.GetLogic();
    QAction* standard = window.findChild<QAction*>("actionCreatePlot");
    QAction* fft = window.findChild<QAction*>("actionFFT");
    QVERIFY(standard);
    QVERIFY(fft);
    QSignalSpy standardTriggered(standard, &QAction::triggered);
    QSignalSpy fftTriggered(fft, &QAction::triggered);

    standard->trigger();
    standard->trigger();
    fft->trigger();
    QCOMPARE(standardTriggered.count(), 2);
    QCOMPARE(fftTriggered.count(), 1);
    const QList<SubPlotMainWindow*> figures = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(figures.size(), 3);
    for (int index = 0; index < figures.size(); ++index) {
        SubPlotMainWindow* figure = figures.at(index);
        QVERIFY(figure);
        QCOMPARE(figure->objectName(), QString("Figure#%1").arg(index));
        QCOMPARE(manager->GetPlotWindowRowsCols(figure->objectName()), std::make_pair(1, 1));
        QObject* target = manager->GetPlotByName(QString("Plot#%1").arg(index + 1));
        QVERIFY(target);
        QWidget* plotTarget = qobject_cast<QWidget*>(target);
        QVERIFY(plotTarget);
        QCOMPARE(plotTarget->parentWidget(), figure->centralWidget());
    }
    for (SubPlotMainWindow* figure : figures)
        figure->close();
    QTRY_COMPARE(window.findChildren<SubPlotMainWindow*>().size(), 0);
}

void MainWindowIntegrationTests::GUI_018_notification_status_scroll_and_timer_lifetime()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    auto* messenger = window.GetLogic()->GetMessenger();
    QPlainTextEdit* output = window.findChild<QPlainTextEdit*>("OutputText");
    QVERIFY(output);
    output->clear();
    QSignalSpy notifications(messenger, &MessengerClass::NotificationWriter);
    InterfaceData first("QString", "State");
    first.SetData(QString("first notification"));
    InterfaceData second("QString", "State");
    second.SetData(QString("second notification"));
    messenger->MessageReceiver("notification", "GUI18_A", first);
    messenger->MessageReceiver("notification", "GUI18_B", second);
    QCOMPARE(notifications.count(), 2);
    QCOMPARE(notifications.at(0).at(0).toString(), QString("GUI18_A"));
    QCOMPARE(notifications.at(0).at(1).toString(), QString("first notification"));
    QCOMPARE(notifications.at(1).at(0).toString(), QString("GUI18_B"));
    QCOMPARE(notifications.at(1).at(1).toString(), QString("second notification"));
    QVERIFY(output->toPlainText().contains("first notification"));
    QVERIFY(output->toPlainText().contains("second notification"));

    InterfaceData status("QString", "State");
    status.SetData(QString("working"));
    messenger->MessageReceiver("StatusMessage", "GUI18", status);
    QCOMPARE(window.GetStatusBar()->currentMessage(), QString("GUI18 -> working"));
    QTRY_VERIFY_WITH_TIMEOUT(window.GetStatusBar()->currentMessage().isEmpty(), 1500);

    output->clear();
    for (int index = 0; index < 40; ++index)
        window.InfoWriter("GUI18", QString("line-%1 %2").arg(index).arg(QString(120, QLatin1Char('x'))));
    QScrollBar* scrollBar = output->verticalScrollBar();
    QVERIFY(scrollBar);
    QVERIFY(scrollBar->maximum() > 0);
    QCOMPARE(scrollBar->value(), scrollBar->maximum());
    scrollBar->setValue(scrollBar->minimum());
    window.InfoWriter("GUI18", "not-auto-scrolled-from-top");
    QCOMPARE(scrollBar->value(), scrollBar->minimum());

    QPointer<MainWindow> timedWindow(new MainWindow);
    QVERIFY(timedWindow);
    InterfaceData delayedStatus("QString", "State");
    delayedStatus.SetData(QString("pending"));
    timedWindow->GetLogic()->GetMessenger()->MessageReceiver("StatusMessage", "GUI18", delayedStatus);
    delete timedWindow;
    QVERIFY(timedWindow.isNull());
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindowIntegrationTests::GUI_019_parameter_min_max_dialog_contract()
{
    MainWindow window;
    const QString id("GUI19Device::Parameter::Gain");
    InterfaceData parameter("double", "Parameter");
    parameter.SetData(2.0);
    auto* manager = window.GetLogic();
    manager->GetMessenger()->MessageReceiver("publish", id, parameter);
    manager->SetMinMaxValue(id, 1.25, 5.5);
    QTreeWidget* tree = window.findChild<QTreeWidget*>("ParameterTreeWidget");
    QVERIFY(tree);
    QTreeWidgetItem* leaf = tree->topLevelItem(0)->child(0)->child(0);
    QVERIFY(leaf);
    tree->setCurrentItem(leaf, QItemSelectionModel::ClearAndSelect);

    auto invokeMinMax = [&] {
        QVERIFY(QMetaObject::invokeMethod(&window, "ChangeMinMaxValue", Qt::DirectConnection));
    };
    int cancellationDialogs = 0;
    QList<double> openedValues;
    QStringList openedTitles;
    QTimer cancellationDriver;
    cancellationDriver.setInterval(0);
    connect(&cancellationDriver, &QTimer::timeout, [&] {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        openedValues << dialog->doubleValue();
        openedTitles << dialog->windowTitle();
        ++cancellationDialogs;
        dialog->reject();
        if (cancellationDialogs == 2)
            cancellationDriver.stop();
    });
    cancellationDriver.start();
    invokeMinMax();
    cancellationDriver.stop();
    QCOMPARE(cancellationDialogs, 2);
    QCOMPARE(openedValues, QList<double>({1.3, 5.5}));
    QVERIFY(openedTitles.at(0).startsWith("Minimal Value of"));
    QVERIFY(openedTitles.at(1).startsWith("Maximum Value of"));
    QCOMPARE(manager->MinMaxValue(id), std::make_pair(1.25, 5.5));

    QSignalSpy requests(manager, &DataManagementClass::MessageSender);
    int commitDialogs = 0;
    QTimer commitDriver;
    commitDriver.setInterval(0);
    connect(&commitDriver, &QTimer::timeout, [&] {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        dialog->setDoubleValue(commitDialogs == 0 ? -3.0 : 9.0);
        ++commitDialogs;
        dialog->accept();
        if (commitDialogs == 2)
            commitDriver.stop();
    });
    commitDriver.start();
    invokeMinMax();
    commitDriver.stop();
    QCOMPARE(commitDialogs, 2);
    QCOMPARE(manager->MinMaxValue(id), std::make_pair(-3.0, 9.0));
    QCOMPARE(requests.count(), 1);
    QCOMPARE(requests.at(0).at(0).toString(), QString("get"));
    QCOMPARE(requests.at(0).at(1).toString(), id);

    requests.clear();
    int invertedDialogs = 0;
    QTimer invertedDriver;
    invertedDriver.setInterval(0);
    connect(&invertedDriver, &QTimer::timeout, [&] {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        dialog->setDoubleValue(invertedDialogs == 0 ? 10.0 : -2.0);
        ++invertedDialogs;
        dialog->accept();
        if (invertedDialogs == 2)
            invertedDriver.stop();
    });
    invertedDriver.start();
    invokeMinMax();
    invertedDriver.stop();
    QCOMPARE(invertedDialogs, 2);
    QCOMPARE(manager->MinMaxValue(id), std::make_pair(10.0, -2.0));
    QCOMPARE(requests.count(), 1);
    QCOMPARE(requests.at(0).at(0).toString(), QString("get"));
    QCOMPARE(requests.at(0).at(1).toString(), id);
}

void MainWindowIntegrationTests::GUI_020_data_alias_and_multi_selection_contract()
{
    MainWindow window;
    auto* manager = window.GetLogic();
    const QString firstId("GUI20Device::Data::First");
    const QString secondId("GUI20Device::Data::Second");
    for (const QString& id : {firstId, secondId}) {
        InterfaceData data("QString", "Data");
        data.SetData(id);
        manager->GetMessenger()->MessageReceiver("publish", id, data);
    }
    QTreeWidget* tree = window.findChild<QTreeWidget*>("DataTreeWidget");
    QVERIFY(tree);
    QTreeWidgetItem* first = tree->topLevelItem(0)->child(0)->child(0);
    QTreeWidgetItem* second = tree->topLevelItem(0)->child(0)->child(1);
    QVERIFY(first);
    QVERIFY(second);

    auto showDataMenuForCurrentSelection = [&](QMenu*& menu) {
        QVERIFY(QMetaObject::invokeMethod(&window, "contextMenuTreeWidgetData", Qt::DirectConnection,
                                          Q_ARG(QPoint, QPoint(1, 1))));
        QTRY_VERIFY(QApplication::activePopupWidget());
        menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
        QVERIFY(menu);
    };
    auto closePopup = [] {
        if (QWidget* popup = QApplication::activePopupWidget())
            popup->close();
    };
    tree->setCurrentItem(first, QItemSelectionModel::ClearAndSelect);
    QMenu* menu = nullptr;
    showDataMenuForCurrentSelection(menu);
    QCOMPARE(menu->actions().size(), 1);
    QAction* setAlias = menu->actions().constFirst();
    QCOMPARE(setAlias->text(), QString("Set Alias"));
    int cancelledDialogs = 0;
    QTimer cancelDriver;
    cancelDriver.setInterval(0);
    connect(&cancelDriver, &QTimer::timeout, [&] {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        ++cancelledDialogs;
        QCOMPARE(dialog->textValue(), firstId);
        cancelDriver.stop();
        dialog->reject();
    });
    cancelDriver.start();
    setAlias->trigger();
    cancelDriver.stop();
    QCOMPARE(cancelledDialogs, 1);
    QCOMPARE(manager->GetAlias(firstId), firstId);

    showDataMenuForCurrentSelection(menu);
    setAlias = menu->actions().constFirst();
    int unicodeDialogs = 0;
    const QString unicodeAlias = QString::fromUtf8("Mäßwert Ω");
    QTimer unicodeDriver;
    unicodeDriver.setInterval(0);
    connect(&unicodeDriver, &QTimer::timeout, [&] {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        ++unicodeDialogs;
        unicodeDriver.stop();
        dialog->setTextValue(unicodeAlias);
        dialog->accept();
    });
    unicodeDriver.start();
    setAlias->trigger();
    unicodeDriver.stop();
    QCOMPARE(unicodeDialogs, 1);
    QCOMPARE(manager->GetAlias(firstId), unicodeAlias);
    QCOMPARE(first->text(0), QString("First"));

    showDataMenuForCurrentSelection(menu);
    QCOMPARE(menu->actions().size(), 2);
    QAction* removeAlias = menu->actions().at(1);
    QCOMPARE(removeAlias->text(), QString("Remove Alias"));
    removeAlias->trigger();
    QCOMPARE(manager->GetAlias(firstId), firstId);
    QCOMPARE(first->text(0), QString("First"));
    showDataMenuForCurrentSelection(menu);
    QCOMPARE(menu->actions().size(), 1);
    closePopup();

    showDataMenuForCurrentSelection(menu);
    setAlias = menu->actions().constFirst();
    int emptyDialogs = 0;
    QTimer emptyDriver;
    emptyDriver.setInterval(0);
    connect(&emptyDriver, &QTimer::timeout, [&] {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        ++emptyDialogs;
        emptyDriver.stop();
        dialog->setTextValue(QString());
        dialog->accept();
    });
    emptyDriver.start();
    setAlias->trigger();
    emptyDriver.stop();
    QCOMPARE(emptyDialogs, 1);
    QCOMPARE(manager->GetAlias(firstId), firstId);

    tree->setCurrentItem(first, QItemSelectionModel::ClearAndSelect);
    tree->selectionModel()->select(tree->indexFromItem(second), QItemSelectionModel::Select);
    QCOMPARE(tree->selectedItems().size(), 2);
    const QString combinedId = firstId + secondId;
    showDataMenuForCurrentSelection(menu);
    QCOMPARE(menu->actions().size(), 1);
    setAlias = menu->actions().constFirst();
    int multipleDialogs = 0;
    QTimer multipleDriver;
    multipleDriver.setInterval(0);
    connect(&multipleDriver, &QTimer::timeout, [&] {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (!dialog)
            return;
        ++multipleDialogs;
        QCOMPARE(dialog->textValue(), combinedId);
        multipleDriver.stop();
        dialog->setTextValue("combined alias");
        dialog->accept();
    });
    multipleDriver.start();
    setAlias->trigger();
    multipleDriver.stop();
    QCOMPARE(multipleDialogs, 1);
    QCOMPARE(manager->GetAlias(combinedId), QString("combined alias"));
    QCOMPARE(manager->GetAlias(firstId), firstId);
    QCOMPARE(manager->GetAlias(secondId), secondId);
}

void MainWindowIntegrationTests::GUI_021_device_context_action_removes_without_confirmation()
{
    MainWindow window;
    auto* manager = window.GetLogic();
    const QString deviceName("GUI21Device");
    int destroyed = 0;
    manager->AddDevice(deviceName, "test-device.xml", new Gui19Device(deviceName, &destroyed));
    QCOMPARE(manager->GetDevices(), QList<QString>{deviceName});
    for (const QString& type : {QString("Parameter"), QString("Data"), QString("State")}) {
        InterfaceData data("QString", type);
        data.SetData(type);
        manager->GetMessenger()->MessageReceiver("publish", deviceName + "::Channel", data);
    }
    QTreeWidget* dataTree = window.findChild<QTreeWidget*>("DataTreeWidget");
    QVERIFY(dataTree);
    QTreeWidgetItem* device = dataTree->topLevelItem(0);
    QVERIFY(device);
    QCOMPARE(device->text(0), deviceName);
    dataTree->setCurrentItem(device, QItemSelectionModel::ClearAndSelect);
    QSignalSpy messages(manager, &DataManagementClass::MessageSender);
    QVERIFY(QMetaObject::invokeMethod(&window, "contextMenuTreeWidgetData", Qt::DirectConnection,
                                      Q_ARG(QPoint, QPoint(1, 1))));
    QTRY_VERIFY(QApplication::activePopupWidget());
    QMenu* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    QVERIFY(menu);
    QCOMPARE(menu->actions().size(), 1);
    QAction* remove = menu->actions().constFirst();
    QCOMPARE(remove->text(), QString("Remove Device"));
    QVERIFY(remove->isEnabled());
    QCOMPARE(manager->GetDevice(deviceName)->GetObject()->objectName(), deviceName);
    QCOMPARE(destroyed, 0);
    remove->trigger();
    QCOMPARE(messages.count(), 0);
    QCOMPARE(destroyed, 1);
    QCOMPARE(manager->GetDevice(deviceName), nullptr);
    QVERIFY(!manager->GetDevices().contains(deviceName));
    for (const QString& treeName : {QString("ParameterTreeWidget"), QString("DataTreeWidget"), QString("StateTreeWidget")}) {
        QTreeWidget* tree = window.findChild<QTreeWidget*>(treeName);
        QVERIFY(tree);
        QCOMPARE(tree->topLevelItemCount(), 0);
        QVERIFY(tree->selectedItems().isEmpty());
    }
    QVERIFY(manager->GetContainer(deviceName + "::Channel") != nullptr);
}

void MainWindowIntegrationTests::GUI_SAFE_001_senderless_and_invalid_selection_actions_are_noops()
{
    MainWindow window;
    auto* manager = window.GetLogic();
    QVERIFY(manager);
    QTreeWidget* parameterTree = window.findChild<QTreeWidget*>("ParameterTreeWidget");
    QVERIFY(parameterTree);
    QCOMPARE(parameterTree->selectedItems().size(), 0);
    QCOMPARE(manager->GetDevices().size(), 0);
    QCOMPARE(manager->GetContainerCount(), 0);

    // Former direct-slot calls dereferenced selectedItems()[0] or sender().
    // The approved contract is no dialog, no mutation and no signal.
    QSignalSpy messages(manager, &DataManagementClass::MessageSender);
    QVERIFY(QMetaObject::invokeMethod(&window, "ChangeMinMaxValue", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(&window, "RemoveDevice", Qt::DirectConnection));
    window.dockWidget_topLevelChanged(false);
    QCOMPARE(messages.count(), 0);
    QCOMPARE(manager->GetDevices().size(), 0);
    QCOMPARE(manager->GetContainerCount(), 0);
    QCOMPARE(parameterTree->selectedItems().size(), 0);

    auto* topLevel = new QTreeWidgetItem(parameterTree, QStringList({"unknown-device"}));
    parameterTree->setCurrentItem(topLevel, QItemSelectionModel::ClearAndSelect);
    QVERIFY(QMetaObject::invokeMethod(&window, "ChangeMinMaxValue", Qt::DirectConnection));
    QCOMPARE(messages.count(), 0);
    QCOMPARE(manager->GetContainerCount(), 0);
}

void MainWindowIntegrationTests::GUI_022_publish_tree_view_state_contract()
{
    MainWindow window;
    QTreeWidget* parameter = window.findChild<QTreeWidget*>("ParameterTreeWidget");
    QTreeWidget* data = window.findChild<QTreeWidget*>("DataTreeWidget");
    QTreeWidget* state = window.findChild<QTreeWidget*>("StateTreeWidget");
    QVERIFY(parameter);
    QVERIFY(data);
    QVERIFY(state);

    parameter->setUpdatesEnabled(true);
    data->setUpdatesEnabled(true);
    state->setUpdatesEnabled(true);
    parameter->setSortingEnabled(true);
    data->setSortingEnabled(true);
    state->setSortingEnabled(true);

    window.PublishStart();
    QVERIFY(!parameter->updatesEnabled());
    QVERIFY(!data->updatesEnabled());
    QVERIFY(!state->updatesEnabled());
    QVERIFY(!parameter->isSortingEnabled());
    QVERIFY(!data->isSortingEnabled());
    QVERIFY(!state->isSortingEnabled());

    window.PublishFinished();
    QVERIFY(parameter->updatesEnabled());
    QVERIFY(data->updatesEnabled());
    QVERIFY(state->updatesEnabled());
    QVERIFY(parameter->isSortingEnabled());
    QVERIFY(data->isSortingEnabled());
    QVERIFY(!state->isSortingEnabled());
    QVERIFY(parameter->columnWidth(0) > 0);
    QVERIFY(data->columnWidth(0) > 0);
    QVERIFY(state->columnWidth(0) > 0);
}

void MainWindowIntegrationTests::GUI_SAFE_002_null_figure_deletion_is_a_noop()
{
    MainWindow window;
    const int before = window.GetLogic()->GetPlotWindowsIncrementer();
    window.DeleteFigure(nullptr);
    QCOMPARE(window.GetLogic()->GetPlotWindowsIncrementer(), before);
}

void MainWindowIntegrationTests::GUI_SAFE_003_extensionless_form_path_is_rejected_safely()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("extensionless-form");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("<ui version=\"4.0\"></ui>");
    file.close();

    MainWindow window;
    QSignalSpy errors(window.GetLogic()->GetMessenger(), &MessengerClass::ErrorWriter);
    const QString workingDirectory = QDir::currentPath();
    window.LoadFormFromXML(path, "GUI_SAFE_003", false);
    QCOMPARE(errors.count(), 1);
    QCOMPARE(errors.at(0).at(1).toString(), QString("Corrupt Form File"));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(QDir::currentPath(), workingDirectory);
}

void MainWindowIntegrationTests::GUI_SAFE_004_null_dock_cleanup_is_a_noop()
{
    MainWindow window;
    const int forms = window.GetLogic()->GetFormFileCount();
    window.dockWidget_destroyed(nullptr);
    QCOMPARE(window.GetLogic()->GetFormFileCount(), forms);
}

void MainWindowIntegrationTests::GUI_SAFE_005_orphaned_form_record_close_project_is_safe()
{
    MainWindow window;
    window.GetLogic()->AddFormFile({"GUI_SAFE_005_orphan", "missing-form.ui"});
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 1);

    window.CloseProject();

    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(window.SavePath, QString());
    QVERIFY(!window.ChangeForSaveDetected);
}

void MainWindowIntegrationTests::GUI_SAFE_006_output_context_actions_are_not_retained()
{
    MainWindow window;
    auto* output = window.findChild<QPlainTextEdit*>("OutputText");
    QVERIFY(output);
    const int ownedActionCount = window.findChildren<QAction*>().size();
    int menus = 0;
    QTimer menuDriver;
    menuDriver.setSingleShot(true);
    connect(&menuDriver, &QTimer::timeout, [&] {
        QMenu* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
        QVERIFY(menu);
        ++menus;
        menu->close();
    });

    menuDriver.start();
    window.OutputTextMenu(QPoint(1, 1));
    menuDriver.stop();

    QCOMPARE(menus, 1);
    QCOMPARE(window.findChildren<QAction*>().size(), ownedActionCount);
}

void MainWindowIntegrationTests::GUI_SAFE_007_failed_form_load_does_not_retain_a_toplevel_tab()
{
    MainWindow window;
    const int topLevelCount = QApplication::topLevelWidgets().size();
    QSignalSpy errors(window.GetLogic()->GetMessenger(), &MessengerClass::ErrorWriter);

    window.LoadFormFromXML(fixturePath("missing.ui"), "GUI_SAFE_007", false);

    QCOMPARE(errors.count(), 1);
    QCOMPARE(errors.at(0).at(1).toString(), QString("Corrupt Form File"));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(QApplication::topLevelWidgets().size(), topLevelCount);
}

void MainWindowIntegrationTests::cleanup()
{
    QVERIFY(QDir::setCurrent(originalWorkingDirectory));
}

void MainWindowIntegrationTests::cleanupTestCase()
{
    QVERIFY(QDir::setCurrent(originalWorkingDirectory));
}

int main(int argc, char** argv)
{
    QTemporaryDir settingsDirectory;
    QTemporaryDir workingDirectory;
    if (!settingsDirectory.isValid() || !workingDirectory.isValid())
        return 2;
    const QString startupWorkingDirectory = QDir::currentPath();
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("LABANALYSER_TEST_SETTINGS_ROOT", settingsDirectory.path().toUtf8());
    qputenv("LABANALYSER_TEST_WORKING_ROOT", workingDirectory.path().toUtf8());
    QStandardPaths::setTestModeEnabled(true);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    if (!QDir::setCurrent(workingDirectory.path()))
        return 3;
    QApplication application(argc, argv);
    application.setApplicationName("LabAnalyserMainWindowIntegrationTest");
    application.setOrganizationName("LabAnalyserTests");
    MainWindowIntegrationTests tests;
    const int result = QTest::qExec(&tests, argc, argv);
    QDir::setCurrent(startupWorkingDirectory);
    return result;
}

#include "MainWindowIntegrationTests.moc"
