#ifndef MAINWINDOWDOCKPRESENTATION_H
#define MAINWINDOWDOCKPRESENTATION_H

class MainWindow;
class QObject;
class QDockWidget;
class QEvent;

class MainWindowDockPresentation
{
public:
    static void UpdateTopLevelState(MainWindow& mainWindow, QDockWidget* dock, bool isFloating);
    static void HandleEvent(MainWindow& mainWindow, QObject* object, QEvent* event);
};

#endif // MAINWINDOWDOCKPRESENTATION_H
