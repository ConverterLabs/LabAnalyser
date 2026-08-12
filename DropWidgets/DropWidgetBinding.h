#ifndef DROPWIDGETBINDING_H
#define DROPWIDGETBINDING_H

#include <QtCore/qnamespace.h>

class QObject;
class DataManagementSetClass;

// Non-owning binding helpers shared by the concrete Designer widgets.  They
// deliberately retain the legacy direct Qt connection semantics, including
// repeated connections, instead of deduplicating connections or migrating
// object names.
namespace DropWidgetBinding
{
DataManagementSetClass* CurrentManager();
void ConnectRequestUpdate(QObject* source, const char* signal);
void ConnectValueChanged(QObject* source, const char* signal, DataManagementSetClass* manager,
                         Qt::ConnectionType type = Qt::AutoConnection);
}

#endif // DROPWIDGETBINDING_H
