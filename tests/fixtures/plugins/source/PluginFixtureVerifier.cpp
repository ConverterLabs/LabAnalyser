#include <QCoreApplication>
#include <QPluginLoader>
#include <QJsonObject>
#include <QDebug>
#include "plugins/platforminterface.h"

static bool verify(const QString& path, const QString& iid, bool fabric) {
    QPluginLoader loader(path);
    const QString actualIid = loader.metaData().value("IID").toString();
    QObject* instance = loader.instance();
    const bool cast = qobject_cast<Platform_Fabric*>(instance) != nullptr;
    qInfo().noquote() << path << "IID=" << actualIid << "instance=" << (instance != nullptr) << "fabric=" << cast << "error=" << loader.errorString();
    const bool unloaded = loader.unload();
    qInfo().noquote() << path << "unload=" << unloaded << "error=" << loader.errorString();
    return instance && actualIid == iid && cast == fabric && unloaded;
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    if (argc != 4) return 64;
    const bool compatible = verify(QString::fromLocal8Bit(argv[1]), "org.qt-project.Qt.Examples.EchoInterface", true);
    const bool wrong = verify(QString::fromLocal8Bit(argv[2]), "org.example.tests.WrongIid", false);
    const bool objectOnly = verify(QString::fromLocal8Bit(argv[3]), "org.qt-project.Qt.Examples.EchoInterface", false);
    return compatible && wrong && objectOnly ? 0 : 1;
}
