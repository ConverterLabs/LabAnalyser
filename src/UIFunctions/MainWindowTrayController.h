#ifndef MAINWINDOWTRAYCONTROLLER_H
#define MAINWINDOWTRAYCONTROLLER_H

#include <QSystemTrayIcon>

class QAction;
class QMainWindow;

class MainWindowTrayController
{
public:
    static void MinimizeToTray(QMainWindow& window, QAction& restoreAction);
    static void UpdateRestoreAction(QMainWindow& window, QAction& restoreAction);
    static void HandleActivation(QMainWindow& window, QAction& restoreAction,
                                 QSystemTrayIcon::ActivationReason reason);
};

#endif // MAINWINDOWTRAYCONTROLLER_H
