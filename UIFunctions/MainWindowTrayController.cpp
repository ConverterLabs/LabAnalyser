#include "MainWindowTrayController.h"

#include <QAction>
#include <QMainWindow>
#include <QSystemTrayIcon>

void MainWindowTrayController::MinimizeToTray(QMainWindow& window, QAction& restoreAction)
{
    window.showMinimized();
    window.hide();
    restoreAction.setEnabled(true);
}

void MainWindowTrayController::UpdateRestoreAction(QMainWindow& window, QAction& restoreAction)
{
    if (window.isVisible())
        restoreAction.setEnabled(false);
}

void MainWindowTrayController::HandleActivation(QMainWindow& window, QAction& restoreAction,
                                                QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::DoubleClick)
        return;

    window.show();
    window.showNormal();
    window.raise();
    restoreAction.setEnabled(false);
}
