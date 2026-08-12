#include "MainWindowTreePath.h"

#include <QTreeWidgetItem>

namespace MainWindowTreePath
{
QString IdForItem(const QTreeWidgetItem* item)
{
    QString id;
    for (const QTreeWidgetItem* current = item; current; current = current->parent()) {
        if (!id.isEmpty())
            id.prepend("::");
        id.prepend(current->text(0));
    }
    return id;
}
}
