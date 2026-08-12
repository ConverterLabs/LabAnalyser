#ifndef MAINWINDOWTREEMODEL_H
#define MAINWINDOWTREEMODEL_H

#include <QList>
#include <QPair>
#include <QStringList>

class QDockWidget;
class QTreeWidget;
class QTreeWidgetItem;
class InterfaceData;

namespace MainWindowTreeModel {

void AddElement(QTreeWidget* tree, const QStringList& parts, InterfaceData data);
void RemoveElement(const QList<QTreeWidget*>& trees, const QStringList& parts);
void Highlight(const QList<QPair<QTreeWidget*, QDockWidget*>>& trees, const QStringList& parts);

} // namespace MainWindowTreeModel

#endif // MAINWINDOWTREEMODEL_H
