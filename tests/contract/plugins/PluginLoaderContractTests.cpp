#include <QtTest>
#include <QBuffer>
#include <QFileInfo>
#include "LoadSave/loadplugin.h"
#include "DataManagement/DataManagementSetClass.h"

class PluginLoaderContractTests : public QObject {
    Q_OBJECT
    QString root;
    QByteArray xml(const QString& dll, const QString& name) { return QString("<LEDevice DevicePlugin=\"%1\" DeviceName=\"%2\"/>").arg(dll, name).toUtf8(); }
    LoadPlugin* load(DataManagementSetClass& manager, const QByteArray& source, const QString& file) { auto* loader=new LoadPlugin(&manager, manager.GetMessenger()); QBuffer* buffer=new QBuffer(loader); buffer->setData(source); buffer->open(QIODevice::ReadOnly); if(loader->read(buffer,file)) qFatal("device XML unexpectedly failed"); return loader; }
private slots:
    void initTestCase() { root=qEnvironmentVariable("LABANALYSER_TEST_PLUGIN_ROOT"); QVERIFY2(!root.isEmpty(), "LABANALYSER_TEST_PLUGIN_ROOT is required"); }
    void PLUGIN_001_compatibleLoad();
    void PLUGIN_002_wrongIidRejected();
    void PLUGIN_003_qobjectOnlyRejected();
    void PLUGIN_004_missingDllRejected();
    void PLUGIN_005_repeatSameName();
    void PLUGIN_006_uiReturnConventionSeam();
    void PLUGIN_007_lifetimeUntilCleanup();
};
void PluginLoaderContractTests::PLUGIN_001_compatibleLoad(){ QObject parent; DataManagementSetClass manager(&parent); QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter); auto* l=load(manager,xml(root+"/CompatiblePlugin/release/CompatiblePlugin.dll","DeviceA"),"deviceA.LAdev"); QVERIFY(l->GetNewDevice()); QCOMPARE(manager.GetDevice("DeviceA"),l->GetNewDevice()); QObject* object=l->GetNewDevice()->GetObject(); QCOMPARE(object->objectName(),QString("DeviceA")); QCOMPARE(object->property("test_getInterfaceCalls").toInt(),1); QCOMPARE(object->property("test_messenger").value<QObject*>(),static_cast<QObject*>(manager.GetMessenger())); QCOMPARE(errors.count(),0); }
void PluginLoaderContractTests::PLUGIN_002_wrongIidRejected(){ QObject parent; DataManagementSetClass manager(&parent); QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter); auto* l=load(manager,xml(root+"/WrongIidPlugin/release/WrongIidPlugin.dll","Bad"),"bad.LAdev"); QVERIFY(!l->GetNewDevice()); QVERIFY(!manager.GetDevice("Bad")); QCOMPARE(errors.count(),1); QVERIFY(errors.at(0).at(1).toString().contains("incompatible with Platform_Fabric")); }
void PluginLoaderContractTests::PLUGIN_003_qobjectOnlyRejected(){ QObject parent; DataManagementSetClass manager(&parent); QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter); auto* l=load(manager,xml(root+"/QObjectOnlyPlugin/release/QObjectOnlyPlugin.dll","Plain"),"plain.LAdev"); QVERIFY(!l->GetNewDevice()); QVERIFY(!manager.GetDevice("Plain")); QCOMPARE(errors.count(),1); }
void PluginLoaderContractTests::PLUGIN_004_missingDllRejected(){ QObject parent; DataManagementSetClass manager(&parent); QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter); auto* l=load(manager,xml(root+"/missing.dll","Missing"),"missing.LAdev"); QVERIFY(!l->GetNewDevice()); QVERIFY(!manager.GetDevice("Missing")); QCOMPARE(errors.count(),1); QVERIFY(errors.at(0).at(1).toString().contains("couldn't be loaded")); }
void PluginLoaderContractTests::PLUGIN_005_repeatSameName(){ QObject parent; DataManagementSetClass manager(&parent); auto source=xml(root+"/CompatiblePlugin/release/CompatiblePlugin.dll","Plugin005Unique"); auto* first=load(manager,source,"first.LAdev"); QVERIFY(first->GetNewDevice()); QObject* object=first->GetNewDevice()->GetObject(); const int before=object->property("test_getInterfaceCalls").toInt()-1; const int afterFirst=object->property("test_getInterfaceCalls").toInt(); auto* second=load(manager,source,"second.LAdev"); const int afterSecond=object->property("test_getInterfaceCalls").toInt(); qInfo() << "PLUGIN_005 counters" << before << afterFirst << afterSecond; QCOMPARE(afterFirst,before+1); QVERIFY(!second->GetNewDevice()); QCOMPARE(manager.GetDevice("Plugin005Unique")->GetObject(),object); QCOMPARE(afterSecond,afterFirst); }
void PluginLoaderContractTests::PLUGIN_006_uiReturnConventionSeam(){ QObject parent; DataManagementSetClass manager(&parent); auto invoke=[&](const QString& dll,const QString& name){ auto* l=load(manager,xml(dll,name),name+".LAdev"); return l->GetNewDevice()?false:true; }; QVERIFY(!invoke(root+"/CompatiblePlugin/release/CompatiblePlugin.dll","UiValid")); QVERIFY(invoke(root+"/WrongIidPlugin/release/WrongIidPlugin.dll","UiBad")); QVERIFY(invoke(root+"/QObjectOnlyPlugin/release/QObjectOnlyPlugin.dll","UiPlain")); }
void PluginLoaderContractTests::PLUGIN_007_lifetimeUntilCleanup(){ QObject parent; DataManagementSetClass manager(&parent); auto* l=load(manager,xml(root+"/CompatiblePlugin/release/CompatiblePlugin.dll","Life"),"life.LAdev"); Platform_Interface* p=l->GetNewDevice(); QVERIFY(p); QObject* object=p->GetObject(); QCOMPARE(manager.GetDevice("Life"),p); QCOMPARE(manager.GetDevice("Life")->GetObject(),object); delete l; QVERIFY(manager.GetDevice("Life")); QCOMPARE(manager.GetDevice("Life")->GetObject(),object); }
QTEST_MAIN(PluginLoaderContractTests)
#include "PluginLoaderContractTests.moc"
