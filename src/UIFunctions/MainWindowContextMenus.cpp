#include "MainWindowContextMenus.h"

#include "UIFunctions/MainWindowTreePath.h"

#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QTreeWidget>

namespace {

void ShowRemoveDeviceMenu(QTreeWidgetItem* item, QDockWidget& dock, const QPoint& position,
                          QWidget& menuParent, QObject* slotReceiver)
{
    QMenu* menu = new QMenu(&menuParent);
    menu->setObjectName(item->text(0));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction("Remove Device", slotReceiver, SLOT(RemoveDevice()));
    menu->popup(dock.mapToGlobal(position));
}

}

void MainWindowContextMenus::ShowParameter(QTreeWidget& tree, QDockWidget& dock,
                                           const QPoint& position, QWidget& menuParent,
                                           QObject* slotReceiver)
{
    const QList<QTreeWidgetItem*> selectedItems = tree.selectedItems();
    if (selectedItems.size() != 1)
        return;

    QTreeWidgetItem* item = selectedItems[0];
    if (item->childCount() == 0) {
        QMenu* menu = new QMenu(&menuParent);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->addAction("Change Min/Max Values", slotReceiver, SLOT(ChangeMinMaxValue()));
        menu->popup(dock.mapToGlobal(position));
    } else if (!item->parent()) {
        ShowRemoveDeviceMenu(item, dock, position, menuParent, slotReceiver);
    }
}

void MainWindowContextMenus::ShowState(QTreeWidget& tree, QDockWidget& dock,
                                       const QPoint& position, QWidget& menuParent,
                                       QObject* slotReceiver)
{
    const QList<QTreeWidgetItem*> selectedItems = tree.selectedItems();
    if (selectedItems.size() == 1 && !selectedItems[0]->parent())
        ShowRemoveDeviceMenu(selectedItems[0], dock, position, menuParent, slotReceiver);
}

void MainWindowContextMenus::ShowData(QTreeWidget& tree, QDockWidget& dock, const QPoint& position,
                                      QWidget& menuParent,
                                      const std::function<QString(const QString&)>& aliasFor,
                                      const std::function<void(const QString&)>& setAlias,
                                      const std::function<void(const QString&)>& removeAlias,
                                      QObject* slotReceiver)
{
    const QList<QTreeWidgetItem*> selectedItems = tree.selectedItems();
    if (selectedItems.isEmpty())
        return;

    QTreeWidgetItem* firstItem = selectedItems[0];
    if (firstItem->childCount() == 0) {
        QMenu* menu = new QMenu(&menuParent);
        menu->setAttribute(Qt::WA_DeleteOnClose);

        QString ids;
        for (QTreeWidgetItem* item : selectedItems) {
            if (item->childCount() == 0)
                ids.push_back(MainWindowTreePath::IdForItem(item));
        }

        QAction* setAliasAction = new QAction(menu);
        QObject::connect(setAliasAction, &QAction::triggered, [=] { setAlias(aliasFor(ids)); });
        QAction* removeAliasAction = new QAction(menu);
        QObject::connect(removeAliasAction, &QAction::triggered, [=] { removeAlias(ids); });
        setAliasAction->setText("Set Alias");
        removeAliasAction->setText("Remove Alias");
        menu->addAction(setAliasAction);

        if (aliasFor(ids).compare(ids))
            menu->addAction(removeAliasAction);

        menu->popup(tree.mapToGlobal(position));
    } else if (!firstItem->parent()) {
        ShowRemoveDeviceMenu(firstItem, dock, position, menuParent, slotReceiver);
    }
}
