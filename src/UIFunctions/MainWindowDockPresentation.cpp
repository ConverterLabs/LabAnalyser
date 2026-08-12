#include "MainWindowDockPresentation.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDockWidget>
#include <QEvent>
#include <QTreeWidget>

namespace {

bool IsStandardDock(const QDockWidget* dock)
{
    return dock->objectName().compare("ParameterDock") == 0
        || dock->objectName().compare("ParameterDock") == 0
        || dock->objectName().compare("DataDock") == 0
        || dock->objectName().compare("OutputDock") == 0;
}

bool IsDynamicDock(const QDockWidget* dock)
{
    return dock->objectName().compare("ParameterDock")
        && dock->objectName().compare("StateDock")
        && dock->objectName().compare("DataDock")
        && dock->objectName().compare("OutputDock");
}

void UpdateCentralWidgetVisibility(MainWindow& mainWindow, const QDockWidget* sender)
{
    int found = 0;
    for (QDockWidget* dock : mainWindow.findChildren<QDockWidget*>()) {
        if (mainWindow.dockWidgetArea(dock) == Qt::LeftDockWidgetArea) {
            if (IsDynamicDock(sender) && !dock->isFloating())
                ++found;
        }
    }
    mainWindow.UI()->centralWidget->setHidden(found != 0);
}

} // namespace

void MainWindowDockPresentation::UpdateTopLevelState(MainWindow& mainWindow, QDockWidget* dock, bool isFloating)
{
    if (!dock)
        return;

    if (isFloating) {
        dock->setMaximumWidth(16555);
        dock->setMaximumHeight(16555);
        dock->setWindowFlags(Qt::Window);
        if (IsDynamicDock(dock)) {
            dock->setWindowFlags(dock->windowFlags() | Qt::CustomizeWindowHint |
                                 Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                                 Qt::WindowCloseButtonHint);
        } else {
            dock->setWindowFlags((dock->windowFlags() | Qt::CustomizeWindowHint |
                                  Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint)
                                 & ~Qt::WindowCloseButtonHint);
        }
        dock->show();
    } else {
        mainWindow.UI()->centralWidget->hide();
        if (IsStandardDock(dock)) {
            if (mainWindow.dockWidgetArea(dock) == Qt::BottomDockWidgetArea) {
                dock->setMaximumWidth(16555);
                dock->setMaximumHeight(300);
            } else if (mainWindow.dockWidgetArea(dock) == Qt::RightDockWidgetArea) {
                dock->setMaximumHeight(16555);
                dock->setMaximumWidth(600);
            }
        }
    }
    UpdateCentralWidgetVisibility(mainWindow, dock);
    QApplication::processEvents();
}

void MainWindowDockPresentation::HandleEvent(MainWindow& mainWindow, QObject* object, QEvent* event)
{
    if (!event)
        return;
    QDockWidget* dock = qobject_cast<QDockWidget*>(object);
    if (!dock)
        return;
    if (event->type() == QEvent::Close)
        mainWindow.dockWidget_destroyed(dock);
    if (event->type() == QEvent::Leave) {
        int found = 0;
        for (QDockWidget* candidate : mainWindow.findChildren<QDockWidget*>()) {
            if (mainWindow.dockWidgetArea(candidate) == Qt::LeftDockWidgetArea)
                ++found;
        }
        mainWindow.UI()->centralWidget->setHidden(found != 0);
    }
    if (dock->objectName().compare("ParameterDock") == 0 && event->type() == QEvent::Resize) {
        const int width = mainWindow.UI()->ParameterDock->width();
        mainWindow.UI()->ParameterTreeWidget->setColumnWidth(0, width * 0.45);
        mainWindow.UI()->ParameterTreeWidget->setColumnWidth(1, width * 0.3);
        mainWindow.UI()->ParameterTreeWidget->setColumnWidth(2, width * 0.2);
    }
    if (dock->objectName().compare("DataDock") == 0 && event->type() == QEvent::Resize) {
        const int width = mainWindow.UI()->DataTreeWidget->width();
        mainWindow.UI()->DataTreeWidget->setColumnWidth(0, width * 0.45);
        mainWindow.UI()->DataTreeWidget->setColumnWidth(1, width * 0.3);
        mainWindow.UI()->DataTreeWidget->setColumnWidth(2, width * 0.2);
    }
}
