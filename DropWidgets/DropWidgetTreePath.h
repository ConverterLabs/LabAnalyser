#ifndef DROPWIDGETTREEPATH_H
#define DROPWIDGETTREEPATH_H

#include <QtCore/qstring.h>
#include <QtWidgets/qtreewidget.h>

// Shared legacy tree-item to container-ID mapping for adapters that allow
// multiple selected leaves.  CreateID(QObject*) keeps the existing
// first-selection contract; this helper deliberately operates on one item.
namespace DropWidgetTreePath
{
inline QString IdForItem(const QTreeWidgetItem* item, int column = 0)
{
    QString id;
    for (const QTreeWidgetItem* current = item; current; current = current->parent())
    {
        id.prepend(current->text(column));
        if (current->parent())
            id.prepend("::");
    }
    return id;
}
}

#endif // DROPWIDGETTREEPATH_H
