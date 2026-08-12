#include "MainWindowProjectCleanup.h"

#include "mainwindow.h"
#include "DataManagement/UIDataManagementSetClass.h"
#include "UIFunctions/SubPlotMainWindow.h"

#include <QDockWidget>
#include <QPointer>
#include <QTreeWidget>

namespace {

void ClearTree(QTreeWidget& tree)
{
    while (tree.topLevelItemCount())
        delete tree.topLevelItem(0);
}

}

void MainWindowProjectCleanup::Close(MainWindow& mainWindow,
                                     UIDataManagementSetClass& logic,
                                     QTreeWidget& parameterTree,
                                     QTreeWidget& dataTree,
                                     QTreeWidget& stateTree)
{
    const auto figures = mainWindow.findChildren<SubPlotMainWindow*>();
    for (SubPlotMainWindow* figure : figures)
        figure->close();

    while (logic.GetFormFileCount()) {
        const QString formName = logic.GetFormFileEntry(0).first;
        QDockWidget* dockWidget = mainWindow.findChild<QDockWidget*>(formName);
        if (dockWidget) {
            QPointer<QDockWidget> dock(dockWidget);
            dockWidget->close();
            if (dock)
                delete dock;
        } else {
            logic.RemoveFormFile(formName);
        }
    }

    ClearTree(parameterTree);
    ClearTree(dataTree);
    ClearTree(stateTree);
    logic.CloseProjectLogic();
    mainWindow.SavePath.clear();
    mainWindow.ChangeForSaveDetected = false;
}
