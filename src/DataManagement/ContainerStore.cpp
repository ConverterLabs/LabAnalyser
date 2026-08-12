#include "ContainerStore.h"

ContainerStore::~ContainerStore()
{
    Clear();
}

std::map<QString, ToFormMapper*>* ContainerStore::MapAddress()
{
    return &Container;
}

std::map<QString, ToFormMapper*>& ContainerStore::Map()
{
    return Container;
}

const std::map<QString, ToFormMapper*>& ContainerStore::Map() const
{
    return Container;
}

ToFormMapper* ContainerStore::Find(QString id) const
{
    if (!Container.size())
        return nullptr;
    auto it = Container.find(id);
    if (it == Container.end())
        return nullptr;
    return it->second;
}

ToFormMapper* ContainerStore::LookupOrInsert(QString id)
{
    return Container[id];
}

void ContainerStore::AddOrReplace(QString id, QString dataType, QString type, QString stateDependency)
{
    ToFormMapper* previous = Find(id);
    ToFormMapper* replacement = new ToFormMapper(dataType, type);
    replacement->SetStateDependency(stateDependency);
    if (previous) {
        replacement->Objects = previous->Objects;
        replacement->MaxValue = previous->MaxValue;
        replacement->MinValue = previous->MinValue;
    }
    Container[id] = replacement;
    if (previous)
        delete previous;
}

int ContainerStore::Count() const
{
    return static_cast<int>(Container.size());
}

void ContainerStore::Clear()
{
    for (auto entry : Container)
        if (entry.second)
            delete entry.second;
    Container.clear();
}
