#ifndef MAINWINDOWTREEMODEL_H
#define MAINWINDOWTREEMODEL_H

#include <QHash>
#include <QList>
#include <QPair>
#include <QStringList>

class QDockWidget;
class QTreeWidget;
class QTreeWidgetItem;
class InterfaceData;

namespace MainWindowTreeModel {

// Per-tree O(1) lookup index: maps a full "::" joined path to its leaf item.
// Callers that manage multiple trees should maintain one index per tree.
using ItemIndex = QHash<QString, QTreeWidgetItem*>;

// Add or update the leaf item described by |parts| in |tree|.
// |index| is kept in sync; pass nullptr to skip index maintenance.
void AddElement(QTreeWidget* tree, const QStringList& parts, InterfaceData data,
                ItemIndex* index = nullptr);

// Remove the item identified by |parts| from each tree and its matching index
// entry.  |indices| must have the same length and order as |trees|; use an
// empty (default-constructed) ItemIndex* for trees without an index.
void RemoveElement(const QList<QTreeWidget*>& trees, const QStringList& parts,
                   const QList<ItemIndex*>& indices = {});

void Highlight(const QList<QPair<QTreeWidget*, QDockWidget*>>& trees, const QStringList& parts);

} // namespace MainWindowTreeModel

#endif // MAINWINDOWTREEMODEL_H
