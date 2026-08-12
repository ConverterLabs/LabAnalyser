#include "DropWidgetBinding.h"

#include "CreateID.h"
#include "../DataManagement/DataManagementSetClass.h"

namespace DropWidgetBinding
{
DataManagementSetClass* CurrentManager()
{
    MainWindow* mainWindow = GetMainWindow();
    return mainWindow ? mainWindow->GetLogic() : nullptr;
}

void ConnectRequestUpdate(QObject* source, const char* signal)
{
    if (DataManagementSetClass* manager = CurrentManager())
        QObject::connect(source, signal, manager, SLOT(UpdateRequest()));
}

void ConnectValueChanged(QObject* source, const char* signal, DataManagementSetClass* manager, Qt::ConnectionType type)
{
    if (manager)
        QObject::connect(source, signal, manager, SLOT(SendNewValue()), type);
}
}
