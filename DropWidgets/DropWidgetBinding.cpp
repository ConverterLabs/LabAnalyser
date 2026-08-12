#include "DropWidgetBinding.h"

#include "CreateID.h"
#include "../DataManagement/DataManagementSetClass.h"

namespace DropWidgetBinding
{
DataManagementSetClass* CurrentManager()
{
    return GetMainWindow()->GetLogic();
}

void ConnectRequestUpdate(QObject* source, const char* signal)
{
    QObject::connect(source, signal, CurrentManager(), SLOT(UpdateRequest()));
}

void ConnectValueChanged(QObject* source, const char* signal, DataManagementSetClass* manager, Qt::ConnectionType type)
{
    QObject::connect(source, signal, manager, SLOT(SendNewValue()), type);
}
}
