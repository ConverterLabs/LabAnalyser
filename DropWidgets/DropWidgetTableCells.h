#ifndef DROPWIDGETTABLECELLS_H
#define DROPWIDGETTABLECELLS_H

#include <QtCore/qstring.h>

class DataManagementSetClass;
class QTableWidget;
class QWidget;

// Internal table-cell construction boundary.  QTableWidgeD still owns row
// ordering and removal; this helper preserves the historical per-cell widget,
// object-name and manager-binding choices.
namespace DropWidgetTableCells
{
QWidget* CreateBoundCell(QTableWidget* table, DataManagementSetClass* manager,
                         const QString& id, int column);
}

#endif // DROPWIDGETTABLECELLS_H
