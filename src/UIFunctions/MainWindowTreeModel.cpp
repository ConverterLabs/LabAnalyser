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

void MainWindowTreeModel::AddElement(QTreeWidget* tree, const QStringList& parts, InterfaceData data)
{
    if (!tree)
        return;
    MainWindowTreeItem* current = nullptr;
    for (int index = 0; index < parts.size(); ++index) {
        const QString& part = parts.at(index);
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
        if (index == parts.size() - 1) {
            current->setText(1, data.GetString());
            current->setText(2, data.GetDataType());
            current->setText(3, data.GetStateDependency());
        }
    }
}

void MainWindowTreeModel::RemoveElement(const QList<QTreeWidget*>& trees, const QStringList& parts)
{
    for (QTreeWidget* tree : trees) {
        if (!tree)
            continue;
        QTreeWidgetItem* parent = nullptr;
        QTreeWidgetItem* current = FindPath(tree, parts, &parent);
        if (!current)
            continue;
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
