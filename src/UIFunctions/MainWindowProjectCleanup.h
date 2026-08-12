#ifndef MAINWINDOWPROJECTCLEANUP_H
#define MAINWINDOWPROJECTCLEANUP_H

class MainWindow;
class UIDataManagementSetClass;
class QTreeWidget;

class MainWindowProjectCleanup
{
public:
    static void Close(MainWindow& mainWindow, UIDataManagementSetClass& logic,
                      QTreeWidget& parameterTree, QTreeWidget& dataTree,
                      QTreeWidget& stateTree);
};

#endif // MAINWINDOWPROJECTCLEANUP_H
