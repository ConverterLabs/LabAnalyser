#include <QObject>
class WrongIidPlugin : public QObject {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.example.tests.WrongIid")
};
#include "WrongIidPlugin.moc"
