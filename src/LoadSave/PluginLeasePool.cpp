#include "PluginLeasePool.h"

#include <QCoreApplication>
#include <QPluginLoader>

namespace {
const char PoolObjectName[] = "_LabAnalyserPluginLeasePool";
}

PluginLeasePool::PluginLeasePool(QObject* parent)
    : QObject(parent)
{
    setObjectName(PoolObjectName);
}

PluginLeasePool* PluginLeasePool::ForCurrentApplication()
{
    QCoreApplication* application = QCoreApplication::instance();
    if (!application)
        return nullptr;

    for (QObject* child : application->children()) {
        if (child->objectName() == PoolObjectName)
            return static_cast<PluginLeasePool*>(child);
    }

    return new PluginLeasePool(application);
}

bool PluginLeasePool::Adopt(std::unique_ptr<QPluginLoader> loader)
{
    if (!loader || loader->parent())
        return false;

    Loaders.push_back(std::move(loader));
    return true;
}

#ifdef LABANALYSER_PLUGIN_LEASE_TESTING
int PluginLeasePool::LeaseCountForTesting()
{
    PluginLeasePool* pool = ForCurrentApplication();
    return pool ? static_cast<int>(pool->Loaders.size()) : 0;
}
#endif
