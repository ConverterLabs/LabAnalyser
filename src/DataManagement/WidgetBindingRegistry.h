/***************************************************************************
** Internal name-to-container binding registry used only by
** DataManagementClass.
**
** It deliberately stores QObject::objectName() values, not QObject pointers.
** The registry neither observes QObject destruction nor owns QObjects.
***************************************************************************/

#ifndef WIDGETBINDINGREGISTRY_H
#define WIDGETBINDINGREGISTRY_H

#include <map>

#include <QString>

class WidgetBindingRegistry
{
public:
    bool Contains(const QString& objectName) const;
    QString LookupOrInsert(const QString& objectName);
    QString Find(const QString& objectName) const;
    void Set(const QString& objectName, const QString& containerId);
    bool Take(const QString& objectName, QString* containerId);
    void Clear();

private:
    std::map<QString, QString> ElementsToContainerID;
};

#endif
