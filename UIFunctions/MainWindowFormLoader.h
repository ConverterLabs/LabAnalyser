#ifndef MAINWINDOWFORMLOADER_H
#define MAINWINDOWFORMLOADER_H

#include <QString>

class MainWindow;

class MainWindowFormLoader
{
public:
    static void Load(MainWindow& mainWindow, QString uiFileName, QString lastFormName, bool skip);
};

#endif // MAINWINDOWFORMLOADER_H
