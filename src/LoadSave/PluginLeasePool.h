#ifndef PLUGINLEASEPOOL_H
#define PLUGINLEASEPOOL_H

#include <QObject>

#include <memory>
#include <vector>

class QPluginLoader;

// Internal application-lifetime owner for successfully loaded plugin loaders.
// Loaders intentionally have no QObject parent; this QObject owns them solely
// through unique_ptr and never calls unload().
class PluginLeasePool : public QObject
{
public:
    static PluginLeasePool* ForCurrentApplication();

    bool Adopt(std::unique_ptr<QPluginLoader> loader);

#ifdef LABANALYSER_PLUGIN_LEASE_TESTING
    static int LeaseCountForTesting();
#endif

private:
    explicit PluginLeasePool(QObject* parent);

    std::vector<std::unique_ptr<QPluginLoader>> Loaders;
};

#endif
