#include "WidgetBindingRegistry.h"

bool WidgetBindingRegistry::Contains(const QString& objectName) const
{
    return ElementsToContainerID.find(objectName) != ElementsToContainerID.end();
}

QString WidgetBindingRegistry::LookupOrInsert(const QString& objectName)
{
    return ElementsToContainerID[objectName];
}

QString WidgetBindingRegistry::Find(const QString& objectName) const
{
    auto it = ElementsToContainerID.find(objectName);
    if (it == ElementsToContainerID.end())
        return QString();
    return it->second;
}

void WidgetBindingRegistry::Set(const QString& objectName, const QString& containerId)
{
    ElementsToContainerID[objectName] = containerId;
}

bool WidgetBindingRegistry::Take(const QString& objectName, QString* containerId)
{
    auto it = ElementsToContainerID.find(objectName);
    if (it == ElementsToContainerID.end())
        return false;
    if (containerId)
        *containerId = it->second;
    ElementsToContainerID.erase(it);
    return true;
}

void WidgetBindingRegistry::Clear()
{
    ElementsToContainerID.clear();
}
