#include "MainWindowTreeModel.h"

#include "plugins/InterfaceDataType.h"

#include <QDockWidget>
#include <QItemSelectionModel>
#include <QTreeWidget>

namespace {

class MainWindowTreeItem : public QTreeWidgetItem
{
public:
    using QTreeWidgetItem::QTreeWidgetItem;

private:
    bool operator<(const QTreeWidgetItem& other) const override
    {
        bool numeric = true;
        text(treeWidget()->sortColumn()).toInt(&numeric);
        if (numeric)
            other.text(treeWidget()->sortColumn()).toInt(&numeric);
        if (numeric)
            return text(treeWidget()->sortColumn()).toInt() < other.text(treeWidget()->sortColumn()).toInt();
        return text(treeWidget()->sortColumn()) < other.text(treeWidget()->sortColumn());
    }
};

// Slow linear path used only by Highlight and the index-free RemoveElement
// fallback (both are rare operations).
QTreeWidgetItem* FindPath(QTreeWidget* tree, const QStringList& parts, QTreeWidgetItem** parent)
{
    QTreeWidgetItem* current = nullptr;
    int found = 0;
    if (parent)
        *parent = nullptr;
    for (const QString& part : parts) {
        if (!current) {
            for (int i = 0; i < tree->topLevelItemCount(); ++i) {
                QTreeWidgetItem* item = tree->topLevelItem(i);
                if (item->text(0).compare(part) == 0) {
                    current = item;
                    ++found;
                }
            }
        } else {
            for (int i = 0; i < current->childCount(); ++i) {
                if (current->child(i)->text(0).compare(part) == 0) {
                    if (parent)
                        *parent = current;
                    current = current->child(i);
                    ++found;
                }
            }
        }
        if (!current)
            return nullptr;
    }
    return found == parts.size() ? current : nullptr;
}

} // namespace

void MainWindowTreeModel::AddElement(QTreeWidget* tree, const QStringList& parts, InterfaceData data,
                                     ItemIndex* index)
{
    if (!tree)
        return;

    const QString key = parts.join("::");

    // Fast path: leaf already exists — just update the value columns.
    if (index) {
        auto it = index->find(key);
        if (it != index->end()) {
            it.value()->setText(1, data.GetString());
            it.value()->setText(2, data.GetDataType());
            it.value()->setText(3, data.GetStateDependency());
            return;
        }
    }

    // Slow path: build any missing intermediate nodes and the leaf.
    MainWindowTreeItem* current = nullptr;
    for (int idx = 0; idx < parts.size(); ++idx) {
        const QString& part = parts.at(idx);
        if (!current) {
            for (int i = 0; i < tree->topLevelItemCount(); ++i) {
                QTreeWidgetItem* item = tree->topLevelItem(i);
                if (item->text(0).compare(part) == 0)
                    current = static_cast<MainWindowTreeItem*>(item);
            }
            if (!current) {
                current = new MainWindowTreeItem;
                current->setText(0, part);
                tree->addTopLevelItem(current);
            }
            continue;
        }

        bool childFound = false;
        for (int i = 0; i < current->childCount() && !childFound; ++i) {
            if (current->child(i)->text(0).compare(part) == 0) {
                current = static_cast<MainWindowTreeItem*>(current->child(i));
                childFound = true;
            }
        }
        if (!childFound) {
            MainWindowTreeItem* child = new MainWindowTreeItem;
            child->setText(0, part);
            current->addChild(child);
            current = child;
        }
        if (idx == parts.size() - 1) {
            current->setText(1, data.GetString());
            current->setText(2, data.GetDataType());
            current->setText(3, data.GetStateDependency());
            if (index)
                index->insert(key, current);
        }
    }
}

void MainWindowTreeModel::RemoveElement(const QList<QTreeWidget*>& trees, const QStringList& parts,
                                        const QList<ItemIndex*>& indices)
{
    const QString key = parts.join("::");
    for (int t = 0; t < trees.size(); ++t) {
        QTreeWidget* tree = trees.at(t);
        if (!tree)
            continue;
        ItemIndex* index = (t < indices.size()) ? indices.at(t) : nullptr;

        QTreeWidgetItem* current = nullptr;
        if (index) {
            current = index->value(key, nullptr);
            index->remove(key);
        }
        if (!current) {
            QTreeWidgetItem* parent = nullptr;
            current = FindPath(tree, parts, &parent);
            if (!current)
                continue;
            if (parent)
                parent->removeChild(current);
            else
                tree->removeItemWidget(current, 0);
            delete current;
            continue;
        }

        QTreeWidgetItem* parent = current->parent();
        if (parent)
            parent->removeChild(current);
        else
            tree->removeItemWidget(current, 0);
        delete current;
    }
}

void MainWindowTreeModel::Highlight(const QList<QPair<QTreeWidget*, QDockWidget*>>& trees, const QStringList& parts)
{
    for (const auto& entry : trees) {
        QTreeWidget* tree = entry.first;
        QDockWidget* dock = entry.second;
        if (!tree || !dock)
            continue;
        if (QTreeWidgetItem* current = FindPath(tree, parts, nullptr)) {
            dock->raise();
            tree->setCurrentItem(current, QItemSelectionModel::ClearAndSelect);
        }
    }
}
