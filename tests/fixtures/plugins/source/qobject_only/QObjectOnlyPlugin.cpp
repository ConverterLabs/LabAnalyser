#include <QObject>
class QObjectOnlyPlugin : public QObject {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.EchoInterface")
};
#include "QObjectOnlyPlugin.moc"
