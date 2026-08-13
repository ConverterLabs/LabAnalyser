#include "MainWindowExplorerValues.h"

#include "DataManagement/DataManagementClass.h"
#include "DataManagement/mapper.h"
#include "UIFunctions/MainWindowTreePath.h"

#include <QModelIndex>
#include <QSet>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace MainWindowExplorerValues {

void ConfigureColumns(QTreeWidget& tree, int width)
{
    tree.setColumnWidth(NameColumn, width * 0.45);
    tree.setColumnWidth(ValueColumn, width * 0.25);
    tree.setColumnWidth(TypeColumn, width * 0.20);
    tree.setColumnWidth(StateColumn, width * 0.10);
}

QString FormatScalar(InterfaceData data)
{
    if (data.IsBool())
        return data.GetBool() ? QStringLiteral("true") : QStringLiteral("false");
    if (data.IsSigedNumber())
        return QString::number(data.GetSignedData());
    if (data.IsUnsigedNumber())
        return QString::number(data.GetUnsignedData());
    if (data.IsFloatingPointNumber())
        return QString::number(data.GetFloatingPointData(), 'g',
                               data.GetTypeInfo() == QStringLiteral("float") ? 7 : 12);
    return QString();
}

void RefreshVisible(QTreeWidget& tree, DataManagementClass& dataManagement)
{
    if (!tree.isVisible() || !tree.viewport()->isVisible())
        return;

    const QRect viewport = tree.viewport()->rect();
    QSet<QTreeWidgetItem*> refreshed;
    int y = viewport.top();
    while (y <= viewport.bottom()) {
        const QModelIndex index = tree.indexAt(QPoint(viewport.left(), y));
        if (!index.isValid()) {
            ++y;
            continue;
        }

        QTreeWidgetItem* item = tree.itemFromIndex(index);
        if (!item) {
            ++y;
            continue;
        }
        const QRect itemRect = tree.visualItemRect(item);
        y = qMax(y + 1, itemRect.bottom() + 1);
        if (item->childCount() != 0 || refreshed.contains(item))
            continue;
        refreshed.insert(item);

        ToFormMapper* mapper = dataManagement.GetContainer(MainWindowTreePath::IdForItem(item));
        const QString value = mapper ? FormatScalar(*mapper) : QString();
        if (item->text(ValueColumn) != value)
            item->setText(ValueColumn, value);
    }
}

} // namespace MainWindowExplorerValues
