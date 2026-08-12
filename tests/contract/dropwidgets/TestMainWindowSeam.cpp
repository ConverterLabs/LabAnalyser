// Test-only host seam. Every adapter constructor connects to GetMainWindow()
// although these characterization tests exercise no UI loading or drag/drop.
// The real MainWindow pulls plots and application workflows, so this target
// supplies only a live DataManagementSetClass for those constructor connects.
#include "mainwindow.h"
#include "DropWidgets/DropWidget.h"
#include "UIFunctions/MainWindowOutputLog.h"
#include "UIFunctions/MainWindowTreeViewState.h"

DataManagementSetClass::DataManagementSetClass(QObject* parent)
    : DataManagementClass(parent) {}
void DataManagementSetClass::SetData(const QString& id)
{
    ToFormMapper* element = GetContainer(id);
    if (!element)
        return;
    const ToFormMapper data = *element;
    for (const ObjectStruct& object : element->Objects) {
        if (auto* widget = dynamic_cast<VariantDropWidget*>(object.FormP))
            widget->SetVariantData(data);
    }
}
void DataManagementSetClass::SendNewValue()
{
    QObject* source = sender();
    const QString id = GetContainerID(source);
    if (id.isEmpty())
        return;
    if (GetContainer(id)->GetType() == "Data") {
        emit MessageSender("get", id, InterfaceData());
        return;
    }
    if (auto* widget = dynamic_cast<VariantDropWidget*>(source)) {
        widget->GetVariantData(GetContainer(source));
        emit MessageSender("set", id, GetInterfaceData(source));
    }
}
void DataManagementSetClass::UpdateRequest()
{
    QObject* source = sender();
    if (source)
        emit MessageSender("get", GetContainerID(source->objectName()), InterfaceData());
}
void DataManagementSetClass::UpdateRequest(QString id) { emit MessageSender("get", id, InterfaceData()); }

UIDataManagementSetClass::UIDataManagementSetClass(QObject* parent)
    : DataManagementSetClass(parent) {}
UIDataManagementSetClass::~UIDataManagementSetClass() {}
bool UIDataManagementSetClass::SaveExperiment(QString) { return false; }

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(nullptr), Remote(nullptr), icon(nullptr), restore(nullptr)
{
    ExtendedDataManagement = new UIDataManagementSetClass(this);
}
MainWindow::~MainWindow() {}
QStatusBar* MainWindow::GetStatusBar() { return statusBar(); }
void MainWindow::HighLightConnection(QString) {}
void MainWindow::Error(QString) {}
void MainWindow::Info(QString) {}
void MainWindow::closeEvent(QCloseEvent*) {}
void MainWindow::resizeEvent(QResizeEvent*) {}
void MainWindow::changeEvent(QEvent*) {}
void MainWindow::dockWidget_topLevelChanged(bool) {}
void MainWindow::dockWidget_destroyed(QObject*) {}
void MainWindow::RemoveConnection(QString) {}
void MainWindow::TrayIconActivated(QSystemTrayIcon::ActivationReason) {}
void MainWindow::AddElementToWidget(QString, InterfaceData) {}
void MainWindow::UpdateElementValue(QString) {}
void MainWindow::LoadFormFromXML(QString, QString, bool) {}
void MainWindow::LoadFormFromXML(QString) {}
void MainWindow::PublishFinished() {}
void MainWindow::PublishStart() {}
void MainWindow::OutputTextMenu(QPoint) {}
void MainWindow::ErrorWriter(const QString&, QString) {}
void MainWindow::InfoWriter(const QString&, QString) {}
void MainWindow::CloseProject() {}
void MainWindow::contextMenuTreeWidget(QPoint) {}
void MainWindow::contextMenuTreeWidgetData(QPoint) {}
void MainWindow::contextMenuTreeWidgetState(QPoint) {}
void MainWindow::on_actionLoad_Form_triggered() {}
void MainWindow::on_actionBeenden_triggered() {}
void MainWindow::on_actionCreatePlot_triggered() {}
void MainWindow::on_actionCreate_Subplot_triggered() {}
void MainWindow::on_actionDaten_Exportieren_mat_triggered() {}
void MainWindow::on_actionSave_triggered() {}
void MainWindow::on_actionSave_Experiment_triggered() {}
void MainWindow::on_actionLoadExperiment_triggered() {}
void MainWindow::on_Close_Project_triggered() {}
void MainWindow::on_actionLoadPlugin_triggered() {}
void MainWindow::on_actionMinimize_to_Tray_triggered() {}
void MainWindow::NotificationWriter(const QString&, QString) {}
void MainWindow::on_actionLoad_Parameter_File_triggered() {}
void MainWindow::on_ParameterTreeWidget_customContextMenuRequested(const QPoint&) {}
void MainWindow::on_StateTreeWidget_customContextMenuRequested(const QPoint&) {}
void MainWindow::on_actionSave_Parameter_Set_triggered() {}
void MainWindow::on_actionAbout_LabAnalyzer_triggered() {}
void MainWindow::on_actionAbout_triggered() {}
void MainWindow::on_pushButton_clicked() {}
void MainWindow::on_actionExport_Data_h5_triggered() {}
void MainWindow::on_actionRemote_Connection_Port_2_triggered() {}
void MainWindow::on_actionFFT_triggered() {}
bool MainWindow::eventFilter(QObject*, QEvent*) { return false; }
void MainWindow::ChangeMinMaxValue() {}
void MainWindow::RemoveDevice() {}
void MainWindow::SetAlias(QString) {}
void MainWindow::RemoveAlias(QString) {}
