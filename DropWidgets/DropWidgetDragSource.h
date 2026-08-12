#ifndef DROPWIDGETDRAGSOURCE_H
#define DROPWIDGETDRAGSOURCE_H

#include <QtCore/qobject.h>
#include <QtWidgets/qtreewidget.h>

#include "CreateID.h"

// The legacy adapters accept only the first selected leaf of a QTreeWidget.
// Keep that selection rule while turning an empty selection into a safe reject
// rather than indexing selectedItems()[0].
namespace DropWidgetDragSource
{
inline bool HasFirstSelectedLeaf(QObject* source)
{
    QTreeWidget* treeWidget = qobject_cast<QTreeWidget*>(source);
    if (!treeWidget)
        return false;

    const QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
    return !selectedItems.isEmpty() && selectedItems.first()->childCount() == 0;
}

inline ToFormMapper* ContainerForFirstSelectedLeaf(QObject* source)
{
    if (!HasFirstSelectedLeaf(source))
        return nullptr;

    MainWindow* mainWindow = GetMainWindow();
    return mainWindow ? mainWindow->GetLogic()->GetContainer(CreateID(source)) : nullptr;
}
}

#endif // DROPWIDGETDRAGSOURCE_H
