#ifndef MAINWINDOWCONTEXTMENUS_H
#define MAINWINDOWCONTEXTMENUS_H

#include <functional>

class QDockWidget;
class QObject;
class QPoint;
class QString;
class QTreeWidget;
class QWidget;

class MainWindowContextMenus
{
public:
    static void ShowParameter(QTreeWidget& tree, QDockWidget& dock, const QPoint& position,
                              QWidget& menuParent, QObject* slotReceiver);
    static void ShowState(QTreeWidget& tree, QDockWidget& dock, const QPoint& position,
                          QWidget& menuParent, QObject* slotReceiver);
    static void ShowData(QTreeWidget& tree, QDockWidget& dock, const QPoint& position, QWidget& menuParent,
                         const std::function<QString(const QString&)>& aliasFor,
                         const std::function<void(const QString&)>& setAlias,
                         const std::function<void(const QString&)>& removeAlias,
                         QObject* slotReceiver);
};

#endif // MAINWINDOWCONTEXTMENUS_H
