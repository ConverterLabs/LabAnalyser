/***************************************************************************
** Internal mapper-owner used exclusively by DataManagementClass.
**
** The exposed raw map is a legacy public compatibility boundary. This store
** owns only the ToFormMapper pointers currently held by that map; ObjectStruct
** QObject pointers remain non-owning and are never deleted here.
***************************************************************************/

#ifndef CONTAINERSTORE_H
#define CONTAINERSTORE_H

#include <map>

#include "mapper.h"

class ContainerStore
{
public:
    ~ContainerStore();

    std::map<QString, ToFormMapper*>* MapAddress();
    std::map<QString, ToFormMapper*>& Map();
    const std::map<QString, ToFormMapper*>& Map() const;

    ToFormMapper* Find(QString id) const;
    ToFormMapper* LookupOrInsert(QString id);
    void AddOrReplace(QString id, QString dataType, QString type, QString stateDependency);
    int Count() const;
    void Clear();

private:
    std::map<QString, ToFormMapper*> Container;
};

#endif
