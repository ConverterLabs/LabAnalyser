#include <QtTest>
#include <QBuffer>
#include <QFileInfo>
#include <QPluginLoader>
#include <QPointer>
#include "LoadSave/loadplugin.h"
#include "LoadSave/PluginLeasePool.h"
#include "DataManagement/DataManagementSetClass.h"

class HostDeleteProbe final : public Platform_Interface
{
public:
    explicit HostDeleteProbe(int* destructions) : destructions(destructions) {}
    ~HostDeleteProbe() override { ++*destructions; }
    InterfaceData* GetSymbol(const QString&) override { return nullptr; }
    QObject* GetObject() override { return nullptr; }
    void MessageReceiver(const QString&, const QString&, InterfaceData) override {}
    void MessageSender(const QString&, const QString&, InterfaceData) override {}

private:
    int* destructions;
};

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
    void PLUGIN_008_memberOwnedModelLoads();
    void PLUGIN_009_heapOwnedModelLoads();
    void PLUGIN_010_managerDestructionDoesNotCleanDevices();
    void PLUGIN_011_loaderRootLifetimeAfterLocalLoader();
    void PLUGIN_012_successfulLoadTransfersOneLease();
    void PLUGIN_013_failedLoadsDoNotTransferLease();
    void PLUGIN_014_memberLegacyCloseDeviceRetainsInterface();
    void PLUGIN_015_heapLegacyCloseDeviceRetainsInterface();
    void PLUGIN_016_legacyRemoveAndProjectCleanupKeepPathSemantics();
    void PLUGIN_017_legacyRemovalDisconnectsOnlyItsMessengerPair();
    void PLUGIN_018_legacyPluginCanBeLoadedAgainAfterLogicalRemoval();
    void PLUGIN_019_publicAddDeviceRemainsHostDelete();
};
void PluginLoaderContractTests::PLUGIN_001_compatibleLoad(){ QObject parent; DataManagementSetClass manager(&parent); QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter); auto* l=load(manager,xml(root+"/CompatiblePlugin/release/CompatiblePlugin.dll","DeviceA"),"deviceA.LAdev"); QVERIFY(l->GetNewDevice()); QCOMPARE(manager.GetDevice("DeviceA"),l->GetNewDevice()); QObject* object=l->GetNewDevice()->GetObject(); QCOMPARE(object->objectName(),QString("DeviceA")); QCOMPARE(object->property("test_getInterfaceCalls").toInt(),1); QCOMPARE(object->property("test_messenger").value<QObject*>(),static_cast<QObject*>(manager.GetMessenger())); QCOMPARE(errors.count(),0); }
void PluginLoaderContractTests::PLUGIN_002_wrongIidRejected(){ QObject parent; DataManagementSetClass manager(&parent); QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter); auto* l=load(manager,xml(root+"/WrongIidPlugin/release/WrongIidPlugin.dll","Bad"),"bad.LAdev"); QVERIFY(!l->GetNewDevice()); QVERIFY(!manager.GetDevice("Bad")); QCOMPARE(errors.count(),1); QVERIFY(errors.at(0).at(1).toString().contains("incompatible with Platform_Fabric")); }
void PluginLoaderContractTests::PLUGIN_003_qobjectOnlyRejected(){ QObject parent; DataManagementSetClass manager(&parent); QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter); auto* l=load(manager,xml(root+"/QObjectOnlyPlugin/release/QObjectOnlyPlugin.dll","Plain"),"plain.LAdev"); QVERIFY(!l->GetNewDevice()); QVERIFY(!manager.GetDevice("Plain")); QCOMPARE(errors.count(),1); }
void PluginLoaderContractTests::PLUGIN_004_missingDllRejected(){ QObject parent; DataManagementSetClass manager(&parent); QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter); auto* l=load(manager,xml(root+"/missing.dll","Missing"),"missing.LAdev"); QVERIFY(!l->GetNewDevice()); QVERIFY(!manager.GetDevice("Missing")); QCOMPARE(errors.count(),1); QVERIFY(errors.at(0).at(1).toString().contains("couldn't be loaded")); }
void PluginLoaderContractTests::PLUGIN_005_repeatSameName(){ QObject parent; DataManagementSetClass manager(&parent); auto source=xml(root+"/CompatiblePlugin/release/CompatiblePlugin.dll","Plugin005Unique"); auto* first=load(manager,source,"first.LAdev"); QVERIFY(first->GetNewDevice()); QObject* object=first->GetNewDevice()->GetObject(); const int before=object->property("test_getInterfaceCalls").toInt()-1; const int afterFirst=object->property("test_getInterfaceCalls").toInt(); auto* second=load(manager,source,"second.LAdev"); const int afterSecond=object->property("test_getInterfaceCalls").toInt(); qInfo() << "PLUGIN_005 counters" << before << afterFirst << afterSecond; QCOMPARE(afterFirst,before+1); QVERIFY(!second->GetNewDevice()); QCOMPARE(manager.GetDevice("Plugin005Unique")->GetObject(),object); QCOMPARE(afterSecond,afterFirst); }
void PluginLoaderContractTests::PLUGIN_006_uiReturnConventionSeam(){ QObject parent; DataManagementSetClass manager(&parent); auto invoke=[&](const QString& dll,const QString& name){ auto* l=load(manager,xml(dll,name),name+".LAdev"); return l->GetNewDevice()?false:true; }; QVERIFY(!invoke(root+"/CompatiblePlugin/release/CompatiblePlugin.dll","UiValid")); QVERIFY(invoke(root+"/WrongIidPlugin/release/WrongIidPlugin.dll","UiBad")); QVERIFY(invoke(root+"/QObjectOnlyPlugin/release/QObjectOnlyPlugin.dll","UiPlain")); }
void PluginLoaderContractTests::PLUGIN_007_lifetimeUntilCleanup(){ QObject parent; DataManagementSetClass manager(&parent); auto* l=load(manager,xml(root+"/CompatiblePlugin/release/CompatiblePlugin.dll","Life"),"life.LAdev"); Platform_Interface* p=l->GetNewDevice(); QVERIFY(p); QObject* object=p->GetObject(); QCOMPARE(manager.GetDevice("Life"),p); QCOMPARE(manager.GetDevice("Life")->GetObject(),object); delete l; QVERIFY(manager.GetDevice("Life")); QCOMPARE(manager.GetDevice("Life")->GetObject(),object); }

void PluginLoaderContractTests::PLUGIN_008_memberOwnedModelLoads()
{
    QPluginLoader loader(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll");
    QObject* pluginRoot = loader.instance();
    QVERIFY2(pluginRoot, qPrintable(loader.errorString()));
    auto* fabric = qobject_cast<Platform_Fabric*>(pluginRoot);
    QVERIFY(fabric);
    Platform_Interface* first = fabric->GetInterface(nullptr);
    Platform_Interface* second = fabric->GetInterface(nullptr);
    QCOMPARE(first, second);
    QPointer<QObject> device(first->GetObject());
    QVERIFY(device);
    QCOMPARE(device->objectName(), QString("MemberOwnedInterfaceDevice"));
    QCOMPARE(pluginRoot->property("test_getInterfaceCalls").toInt(), 2);
    QCOMPARE(pluginRoot->property("test_interfaceDestructions").toInt(), 0);
    QVERIFY(device);
}

void PluginLoaderContractTests::PLUGIN_009_heapOwnedModelLoads()
{
    QPluginLoader loader(root + "/HeapOwnedInterfacePlugin/release/HeapOwnedInterfacePlugin.dll");
    QObject* pluginRoot = loader.instance();
    QVERIFY2(pluginRoot, qPrintable(loader.errorString()));
    auto* fabric = qobject_cast<Platform_Fabric*>(pluginRoot);
    QVERIFY(fabric);
    Platform_Interface* first = fabric->GetInterface(nullptr);
    Platform_Interface* second = fabric->GetInterface(nullptr);
    QCOMPARE(first, second);
    QPointer<QObject> device(first->GetObject());
    QVERIFY(device);
    QCOMPARE(device->objectName(), QString("HeapOwnedInterfaceDevice"));
    QCOMPARE(pluginRoot->property("test_getInterfaceCalls").toInt(), 2);
    QCOMPARE(pluginRoot->property("test_interfaceDestructions").toInt(), 0);
    QVERIFY(device);
}

void PluginLoaderContractTests::PLUGIN_010_managerDestructionDoesNotCleanDevices()
{
    Platform_Interface* member = nullptr;
    QPointer<QObject> memberObject;
    {
        QObject parent;
        DataManagementSetClass manager(&parent);
        auto* loader = load(manager, xml(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll", "MemberRegistry"), "member.LAdev");
        member = loader->GetNewDevice();
        QVERIFY(member);
        memberObject = member->GetObject();
        QCOMPARE(manager.GetDevice("MemberRegistry"), member);
    }
    QVERIFY(memberObject);

    Platform_Interface* heap = nullptr;
    QPointer<QObject> heapObject;
    QObject* heapPluginRoot = nullptr;
    {
        QObject parent;
        DataManagementSetClass manager(&parent);
        auto* loader = load(manager, xml(root + "/HeapOwnedInterfacePlugin/release/HeapOwnedInterfacePlugin.dll", "HeapRegistry"), "heap.LAdev");
        heap = loader->GetNewDevice();
        QVERIFY(heap);
        heapObject = heap->GetObject();
        heapPluginRoot = heapObject->property("test_pluginRoot").value<QObject*>();
        QCOMPARE(manager.GetDevice("HeapRegistry"), heap);
    }
    QVERIFY(heapObject);
    QVERIFY(heapPluginRoot);
    QCOMPARE(heapPluginRoot->property("test_interfaceDestructions").toInt(), 0);
    delete heap; // Controlled fixture-only cleanup after proving manager destruction does not delete it.
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(!heapObject);
    QCOMPARE(heapPluginRoot->property("test_interfaceDestructions").toInt(), 1);
    Q_UNUSED(member); // Never delete the member-owned interface.
}

void PluginLoaderContractTests::PLUGIN_011_loaderRootLifetimeAfterLocalLoader()
{
    QPointer<QObject> rootPointer;
    QPointer<QObject> devicePointer;
    Platform_Interface* interfacePointer = nullptr;
    {
        QPluginLoader loader(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll");
        QObject* pluginRoot = loader.instance();
        QVERIFY2(pluginRoot, qPrintable(loader.errorString()));
        rootPointer = pluginRoot;
        auto* fabric = qobject_cast<Platform_Fabric*>(pluginRoot);
        QVERIFY(fabric);
        interfacePointer = fabric->GetInterface(nullptr);
        QVERIFY(interfacePointer);
        devicePointer = interfacePointer->GetObject();
    }
    QVERIFY(rootPointer);
    QVERIFY(devicePointer);
    QCOMPARE(interfacePointer->GetObject(), devicePointer.data());
}

void PluginLoaderContractTests::PLUGIN_012_successfulLoadTransfersOneLease()
{
    const int before = PluginLeasePool::LeaseCountForTesting();
    QObject parent;
    DataManagementSetClass manager(&parent);
    auto* loader = load(manager,
                        xml(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll", "Lease012"),
                        "lease012.LAdev");
    Platform_Interface* interfacePointer = loader->GetNewDevice();
    QVERIFY(interfacePointer);
    QPointer<QObject> object(interfacePointer->GetObject());
    QVERIFY(object);
    QCOMPARE(PluginLeasePool::LeaseCountForTesting(), before + 1);
    delete loader;
    QVERIFY(object);
    QCOMPARE(manager.GetDevice("Lease012"), interfacePointer);

    auto* duplicate = load(manager,
                           xml(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll", "Lease012"),
                           "lease012-duplicate.LAdev");
    QVERIFY(!duplicate->GetNewDevice());
    QCOMPARE(PluginLeasePool::LeaseCountForTesting(), before + 1);
}

void PluginLoaderContractTests::PLUGIN_013_failedLoadsDoNotTransferLease()
{
    const int before = PluginLeasePool::LeaseCountForTesting();
    QObject parent;
    DataManagementSetClass manager(&parent);
    auto* wrong = load(manager, xml(root + "/WrongIidPlugin/release/WrongIidPlugin.dll", "LeaseBadIid"), "lease-bad-iid.LAdev");
    QVERIFY(!wrong->GetNewDevice());
    QCOMPARE(PluginLeasePool::LeaseCountForTesting(), before);
    auto* plain = load(manager, xml(root + "/QObjectOnlyPlugin/release/QObjectOnlyPlugin.dll", "LeasePlain"), "lease-plain.LAdev");
    QVERIFY(!plain->GetNewDevice());
    QCOMPARE(PluginLeasePool::LeaseCountForTesting(), before);
    auto* missing = load(manager, xml(root + "/missing-lease.dll", "LeaseMissing"), "lease-missing.LAdev");
    QVERIFY(!missing->GetNewDevice());
    QCOMPARE(PluginLeasePool::LeaseCountForTesting(), before);
}

void PluginLoaderContractTests::PLUGIN_014_memberLegacyCloseDeviceRetainsInterface()
{
    QObject parent;
    DataManagementSetClass manager(&parent);
    auto* loader = load(manager,
                        xml(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll", "MemberLegacyClose"),
                        "member-legacy-close.LAdev");
    Platform_Interface* interfacePointer = loader->GetNewDevice();
    QVERIFY(interfacePointer);
    QPointer<QObject> device(interfacePointer->GetObject());
    QPointer<QObject> pluginRoot(device->property("test_pluginRoot").value<QObject*>());
    const int beforeDestructions = pluginRoot->property("test_interfaceDestructions").toInt();

    manager.CloseDevice("MemberLegacyClose");

    QVERIFY(!manager.GetDevice("MemberLegacyClose"));
    QVERIFY(!manager.GetDevices().contains("MemberLegacyClose"));
    QVERIFY(!manager.GetDevicePaths().contains("member-legacy-close.LAdev"));
    QCOMPARE(pluginRoot->property("test_interfaceDestructions").toInt(), beforeDestructions);
    QVERIFY(device);
    QVERIFY(pluginRoot);
}

void PluginLoaderContractTests::PLUGIN_015_heapLegacyCloseDeviceRetainsInterface()
{
    QObject parent;
    DataManagementSetClass manager(&parent);
    auto* loader = load(manager,
                        xml(root + "/HeapOwnedInterfacePlugin/release/HeapOwnedInterfacePlugin.dll", "HeapLegacyClose"),
                        "heap-legacy-close.LAdev");
    Platform_Interface* interfacePointer = loader->GetNewDevice();
    QVERIFY(interfacePointer);
    QPointer<QObject> device(interfacePointer->GetObject());
    QPointer<QObject> pluginRoot(device->property("test_pluginRoot").value<QObject*>());
    const int beforeDestructions = pluginRoot->property("test_interfaceDestructions").toInt();

    manager.CloseDevice("HeapLegacyClose");

    QVERIFY(!manager.GetDevice("HeapLegacyClose"));
    QVERIFY(!manager.GetDevices().contains("HeapLegacyClose"));
    QVERIFY(!manager.GetDevicePaths().contains("heap-legacy-close.LAdev"));
    QCOMPARE(pluginRoot->property("test_interfaceDestructions").toInt(), beforeDestructions);
    QVERIFY(device);
    QVERIFY(pluginRoot);
}

void PluginLoaderContractTests::PLUGIN_016_legacyRemoveAndProjectCleanupKeepPathSemantics()
{
    {
        QObject parent;
        DataManagementSetClass manager(&parent);
        auto* member = load(manager,
                            xml(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll", "MemberLegacyRemove"),
                            "member-legacy-remove.LAdev");
        auto* heap = load(manager,
                          xml(root + "/HeapOwnedInterfacePlugin/release/HeapOwnedInterfacePlugin.dll", "HeapLegacyRemove"),
                          "heap-legacy-remove.LAdev");
        QVERIFY(member->GetNewDevice());
        QVERIFY(heap->GetNewDevice());
        QPointer<QObject> memberObject(member->GetNewDevice()->GetObject());
        QPointer<QObject> heapObject(heap->GetNewDevice()->GetObject());
        manager.RemoveDevices();
        QVERIFY(!manager.GetDevice("MemberLegacyRemove"));
        QVERIFY(!manager.GetDevice("HeapLegacyRemove"));
        QVERIFY(manager.GetDevicePaths().contains("member-legacy-remove.LAdev"));
        QVERIFY(manager.GetDevicePaths().contains("heap-legacy-remove.LAdev"));
        QVERIFY(memberObject);
        QVERIFY(heapObject);
    }
    {
        QObject parent;
        DataManagementSetClass manager(&parent);
        auto* member = load(manager,
                            xml(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll", "MemberLegacyProject"),
                            "member-legacy-project.LAdev");
        QVERIFY(member->GetNewDevice());
        QPointer<QObject> memberObject(member->GetNewDevice()->GetObject());
        manager.CloseProjectLogic();
        QVERIFY(!manager.GetDevice("MemberLegacyProject"));
        QVERIFY(!manager.GetDevicePaths().contains("member-legacy-project.LAdev"));
        QVERIFY(memberObject);
    }
}

void PluginLoaderContractTests::PLUGIN_017_legacyRemovalDisconnectsOnlyItsMessengerPair()
{
    QObject parent;
    DataManagementSetClass manager(&parent);
    auto* member = load(manager,
                        xml(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll", "MemberLegacySignals"),
                        "member-legacy-signals.LAdev");
    auto* heap = load(manager,
                      xml(root + "/HeapOwnedInterfacePlugin/release/HeapOwnedInterfacePlugin.dll", "HeapLegacySignals"),
                      "heap-legacy-signals.LAdev");
    QVERIFY(member->GetNewDevice());
    QVERIFY(heap->GetNewDevice());
    QObject* memberObject = member->GetNewDevice()->GetObject();
    QObject* heapObject = heap->GetNewDevice()->GetObject();
    manager.GetMessenger()->NewDeviceRegistration(memberObject);
    manager.GetMessenger()->NewDeviceRegistration(heapObject);
    const int memberBefore = memberObject->property("test_messageReceives").toInt();
    const int heapBefore = heapObject->property("test_messageReceives").toInt();
    InterfaceData payload;
    payload.SetData(QString("ownership-probe"));
    manager.GetMessenger()->MessageTransmitter("info", "ownership", payload);
    QCOMPARE(memberObject->property("test_messageReceives").toInt(), memberBefore + 1);
    QCOMPARE(heapObject->property("test_messageReceives").toInt(), heapBefore + 1);

    manager.CloseDevice("MemberLegacySignals");
    manager.GetMessenger()->MessageTransmitter("info", "ownership", payload);
    QCOMPARE(memberObject->property("test_messageReceives").toInt(), memberBefore + 1);
    QCOMPARE(heapObject->property("test_messageReceives").toInt(), heapBefore + 2);
}

void PluginLoaderContractTests::PLUGIN_018_legacyPluginCanBeLoadedAgainAfterLogicalRemoval()
{
    QObject parent;
    DataManagementSetClass manager(&parent);
    const QByteArray source = xml(root + "/MemberOwnedInterfacePlugin/release/MemberOwnedInterfacePlugin.dll", "LegacyReload");
    auto* first = load(manager, source, "legacy-reload-first.LAdev");
    QVERIFY(first->GetNewDevice());
    manager.CloseDevice("LegacyReload");
    QVERIFY(!manager.GetDevice("LegacyReload"));

    auto* second = load(manager, source, "legacy-reload-second.LAdev");
    QVERIFY(second->GetNewDevice());
    QCOMPARE(manager.GetDevice("LegacyReload"), second->GetNewDevice());
    manager.CloseDevice("LegacyReload");
    QVERIFY(!manager.GetDevice("LegacyReload"));
}

void PluginLoaderContractTests::PLUGIN_019_publicAddDeviceRemainsHostDelete()
{
    QObject parent;
    DataManagementSetClass manager(&parent);
    int destructions = 0;
    manager.AddDevice("HostDelete", "host-delete.LAdev", new HostDeleteProbe(&destructions));
    manager.CloseDevice("HostDelete");
    QCOMPARE(destructions, 1);
    QVERIFY(!manager.GetDevice("HostDelete"));
}
QTEST_MAIN(PluginLoaderContractTests)
#include "PluginLoaderContractTests.moc"
