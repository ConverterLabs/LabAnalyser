#ifndef PLOTWIDGETDROPBINDING_H
#define PLOTWIDGETDROPBINDING_H

#include "../../DataManagement/DataManagementSetClass.h"
#include "../DropWidgetTreePath.h"

namespace PlotWidgetDropBinding
{
inline bool SupportsVectorData(ToFormMapper* container)
{
    return container && container->GetType().compare("Data") == 0
           && container->GetDataType().compare("vector<double>") == 0;
}

inline QString BufferedId(const QString& id)
{
    const QStringList parts = id.split("::");
    if (parts.isEmpty())
        return id;

    QStringList buffered = parts;
    buffered.insert(1, "Buffered");
    return buffered.join("::");
}

inline bool ResolveSupportedItem(DataManagementSetClass* manager,
                                 const QTreeWidgetItem* item,
                                 QString* resolvedId)
{
    if (!manager || !item || item->childCount() != 0 || !resolvedId)
        return false;

    const QString id = DropWidgetTreePath::IdForItem(item);
    if (SupportsVectorData(manager->GetContainer(id)))
    {
        *resolvedId = id;
        return true;
    }

    const QString bufferedId = BufferedId(id);
    if (!SupportsVectorData(manager->GetContainer(bufferedId)))
        return false;

    *resolvedId = bufferedId;
    return true;
}
}

#endif // PLOTWIDGETDROPBINDING_H
