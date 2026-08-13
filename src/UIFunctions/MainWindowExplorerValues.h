#ifndef MAINWINDOWEXPLORERVALUES_H
#define MAINWINDOWEXPLORERVALUES_H

#include <QString>

class DataManagementClass;
class InterfaceData;
class QTreeWidget;

namespace MainWindowExplorerValues {

enum Column {
    NameColumn = 0,
    ValueColumn = 1,
    TypeColumn = 2,
    StateColumn = 3
};

void ConfigureColumns(QTreeWidget& tree, int width);
QString FormatScalar(InterfaceData data);
void RefreshVisible(QTreeWidget& tree, DataManagementClass& dataManagement);

} // namespace MainWindowExplorerValues

#endif // MAINWINDOWEXPLORERVALUES_H
