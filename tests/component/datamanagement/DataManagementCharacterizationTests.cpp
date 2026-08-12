#include <QtTest>
#include <QPointer>
#include <QSignalSpy>

#include "DataManagement/DataManagementClass.h"
#include "DataManagement/DataManagementSetClass.h"
#include "DataManagement/DataMessengerClass.h"
#include "DropWidgets/DropWidgets.h"

Q_DECLARE_METATYPE(InterfaceData)

namespace
{
class SignalOrderRecorder final : public QObject
{
    Q_OBJECT
public:
    QStringList events;

public slots:
    void added(const QString&, const QString&, const QString&, const QString&) { events << "added"; }
    void set(const QString&, InterfaceData) { events << "set"; }
    void widget(const QString&, InterfaceData) { events << "widget"; }
    void received(const QString&) { events << "received"; }
    void sent(const QString&, const QString&, InterfaceData) { events << "sent"; }
    void error(const QString&, const QString&) { events << "error"; }
    void info(const QString&, const QString&) { events << "info"; }
    void notification(const QString&, const QString&) { events << "notification"; }
    void closeProject() { events << "close"; }
    void publishStart() { events << "start"; }
    void publishFinished() { events << "finished"; }
};

class ProbeDropWidget final : public QObject, public VariantDropWidget
{
    Q_OBJECT
public:
    int setCalls = 0;
    int getCalls = 0;
    ToFormMapper lastReceived{"", ""};
    InterfaceData nextValue;

    void SetVariantData(ToFormMapper data) override
    {
        ++setCalls;
        lastReceived = data;
    }

    void GetVariantData(ToFormMapper* data) override
    {
        ++getCalls;
        data->SetDataRaw(nextValue.GetData());
    }

    bool LoadFromXML(const std::vector<std::pair<QString, QString>>&, const QString&) override { return false; }
    bool SaveToXML(std::vector<std::pair<QString, QString>>&, QString&) override { return false; }
    void ConnectToID(DataManagementSetClass*, QString) override {}

signals:
    void changed();
    void requestUpdate();
};

class DeviceProbe final : public QObject
{
    Q_OBJECT
public:
    int receivedCount = 0;
    QString lastCommand;
    QString lastId;

public slots:
    void MessageReceiver(const QString& command, const QString& id, InterfaceData)
    {
        ++receivedCount;
        lastCommand = command;
        lastId = id;
    }

signals:
    void MessageSender(const QString& command, const QString& id, InterfaceData data);
};

class PlatformProbe final : public Platform_Interface
{
public:
    explicit PlatformProbe(int* destroyed) : destroyed_(destroyed) {}
    ~PlatformProbe() override { ++*destroyed_; }

    InterfaceData* GetSymbol(const QString&) override { return nullptr; }
    QObject* GetObject() override { return nullptr; }
    void MessageReceiver(const QString&, const QString&, InterfaceData) override {}
    void MessageSender(const QString&, const QString&, InterfaceData) override {}

private:
    int* destroyed_;
};

class DeviceLifecycleProbe final : public Platform_Interface
{
public:
    DeviceLifecycleProbe(const QString& label, QStringList* lifecycle, QObject* object = nullptr)
        : label_(label), lifecycle_(lifecycle), object_(object) {}

    ~DeviceLifecycleProbe() override
    {
        lifecycle_->append(label_ + ":destroyed");
    }

    InterfaceData* GetSymbol(const QString&) override { return nullptr; }
    QObject* GetObject() override { return object_; }
    void MessageReceiver(const QString&, const QString&, InterfaceData) override {}
    void MessageSender(const QString&, const QString&, InterfaceData) override {}

private:
    QString label_;
    QStringList* lifecycle_;
    QObject* object_;
};

InterfaceData number(double value)
{
    InterfaceData data;
    data.SetData(value);
    return data;
}
}

class DataManagementCharacterizationTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void DM_001_emptyStateAndSafeLookup();
    void DM_002_plotAndFormRegistries();
    void DM_003_containerValuesAliasesAndMappings();
    void DM_004_repeatedMappingsAndCleanup();
    void DM_005_deviceLifetimeAndProjectCleanup();
    void DM_006_messengerPublishSignalOrderAndValues();
    void DM_007_messengerForwardingAndSpecialCommands();
    void DM_008_messengerDeviceRegistrationAndOwnership();
    void DM_009_setDataWidgetPropagationAndRequests();
    void DM_010_setOwnershipAndRepeatedDataFlow();
    void DM_SAFE_001_senderless_and_unknown_setter_slots_are_noops();
    void DM_SAFE_002_messenger_without_optional_parent_boundaries_is_safe();
    void DM_SAFE_003_null_widget_boundaries_do_not_create_or_dereference_bindings();
    void DM_SAFE_004_registry_boundary_inputs_are_safe();
    void DM_SAFE_005_missing_minmax_container_is_safe();
    void DM_REG_001_formFiles_preserve_order_duplicates_and_first_removal();
    void DM_REG_002_skipFormFlags_override_and_project_cleanup();
    void DM_REG_003_aliases_accept_unknown_empty_and_unicode_keys();
    void DM_REG_004_plot_and_window_numbers_keep_duplicate_history();
    void DM_REG_005_registry_state_is_instance_local_and_recreated_empty();
    void DM_CONT_001_containerPointerAndLookupSideEffects();
    void DM_CONT_002_replacementPreservesBindingsAndMinMaxButInvalidatesMapper();
    void DM_CONT_003_multipleContainersUseMapOrderAndIndependentPointers();
    void DM_CONT_004_projectCleanupInvalidatesMappersButNotForeignWidgets();
    void DM_CONT_005_managerDestructionAndContainerStateAreIsolated();
    void DM_BIND_001_bindingIsNameKeyedAndPreservesBoundQObjectPointer();
    void DM_BIND_002_rebindingAndRemovalKeepCurrentLegacySideEffects();
    void DM_BIND_003_destroyedBoundQObjectIsNotAutoCleanedButProjectCleanupIs();
    void DM_BIND_004_boundWidgetsRouteMessagesAndExposePlotDuplicatePropagation();
    void DM_BIND_005_bindingStateAndForeignQObjectOwnershipAreInstanceLocal();
    void DM_BIND_006_nameCollisionsRenamesEmptyNamesAndRoutingUseCurrentName();
    void DM_MSG_001_messageReceiver_commandMatrix();
    void DM_MSG_002_messageTransmitter_commandMatrix();
    void DM_MSG_003_parentHierarchy_emptyInputs_and_mixedSequence();
    void DM_DEV_001_registration_lookup_and_order_are_facade_observable();
    void DM_DEV_002_duplicate_names_do_not_take_the_rejected_pointer();
    void DM_DEV_003_close_remove_and_reregistration_keep_legacy_path_state();
    void DM_DEV_004_project_cleanup_order_qobject_lifetime_and_instance_isolation();
};

void DataManagementCharacterizationTests::initTestCase()
{
    qRegisterMetaType<InterfaceData>("InterfaceData");
}

void DataManagementCharacterizationTests::DM_001_emptyStateAndSafeLookup()
{
    QObject owner;
    DataManagementClass manager(&owner);

    QCOMPARE(manager.PlotCount(), 0);
    QCOMPARE(manager.GetUniquePlotNumber(), 0);
    QCOMPARE(manager.GetPlotByName("missing"), nullptr);
    QCOMPARE(manager.GetContainerCount(), 0);
    QVERIFY(!manager.ElementExists("missing"));
    QCOMPARE(manager.GetContainer("missing"), nullptr);
    QCOMPARE(manager.GetContainerElementForms(0).first, QString());
    QCOMPARE(manager.GetContainerElementForms(0).second.size(), size_t(0));
    QCOMPARE(manager.GetAlias("missing"), QString("missing"));
    QVERIFY(!manager.GetSkipFormFile("missing"));

    QObject unknown;
    unknown.setObjectName("unknown");
    QCOMPARE(manager.GetContainerID(&unknown), QString());
    QCOMPARE(manager.GetContainer(&unknown), nullptr);
    // GetContainer(QObject*) routes through the QString overload, whose map
    // operator inserts an empty mapping for an unknown object name.
    QVERIFY(manager.IsObjectLinked(&unknown));
    manager.SetData("missing", number(1.0));
    manager.DeletePlotPointer("missing");
    manager.DeletePlotWindow("missing");
    manager.RemoveFormFile("missing");
    manager.CloseDevice("missing");
}

void DataManagementCharacterizationTests::DM_002_plotAndFormRegistries()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject first;
    QObject second;
    first.setObjectName("visible-first");
    second.setObjectName("visible-second");

    manager.AddPlotPointer("first", &first, 0);
    manager.AddPlotPointer("second", &second, 2);
    QCOMPARE(manager.PlotCount(), 2);
    QCOMPARE(manager.GetPlotByName("visible-first"), &first);
    QCOMPARE(manager.GetUniquePlotNumber(), 1);
    manager.DeletePlotPointer("first");
    QCOMPARE(manager.GetUniquePlotNumber(), 0);
    manager.RenamePlotPointer("second", "renamed#2");
    QCOMPARE(manager.GetPlotByName("visible-second"), &second);
    manager.DeletePlotPointer("renamed#2");
    QCOMPARE(manager.PlotCount(), 0);

    manager.AddPlotWindow("window-a", 2, 3, 0);
    manager.AddPlotWindow("window-b", 4, 5, 2);
    QCOMPARE(manager.GetPlotWindowRowsCols("window-a"), std::make_pair(2, 3));
    QCOMPARE(manager.GetPlotWindowsIncrementer(), 1);
    manager.DeletePlotWindow("window-a");
    QCOMPARE(manager.GetPlotWindowsIncrementer(), 0);
    manager.DeletePlotWindow("window-a");

    manager.AddFormFile({"form-a", "a.ui"});
    manager.AddFormFile({"form-b", "b.ui"});
    QCOMPARE(manager.GetFormFileCount(), 2);
    QCOMPARE(manager.GetFormFileEntry(1), std::make_pair(QString("form-b"), QString("b.ui")));
    manager.AddSkipFormFile("form-a", true);
    QVERIFY(manager.GetSkipFormFile("form-a"));
    manager.AddSkipFormFile("form-a", false);
    QVERIFY(!manager.GetSkipFormFile("form-a"));
    manager.RemoveFormFile("form-a");
    manager.RemoveFormFile("form-a");
    QCOMPARE(manager.GetFormFileCount(), 1);
}

void DataManagementCharacterizationTests::DM_003_containerValuesAliasesAndMappings()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject widget;
    widget.setObjectName("widget-a");

    manager.AddContainerElement("parameter", "double", "Parameter", "state-a");
    QVERIFY(manager.ElementExists("parameter"));
    QCOMPARE(manager.GetContainer("parameter")->GetDataType(), QString());
    QCOMPARE(manager.GetContainer("parameter")->GetType(), QString("Parameter"));
    QCOMPARE(manager.GetContainer("parameter")->GetStateDependency(), QString("state-a"));
    manager.SetMinMaxValue("parameter", -1.0, 2.0);
    QCOMPARE(manager.MinMaxValue("parameter"), std::make_pair(-1.0, 2.0));
    manager.SetAlias("parameter", "alias");
    QCOMPARE(manager.GetAlias("parameter"), QString("alias"));

    manager.AddElementToContainerEntry("widget-a", "parameter", "QWidget", &widget);
    QVERIFY(manager.IsObjectLinked(&widget));
    QCOMPARE(manager.GetContainerID("widget-a"), QString("parameter"));
    QCOMPARE(manager.GetContainerID(&widget), QString("parameter"));
    QCOMPARE(manager.GetObjectType(&widget), QString("QWidget"));

    manager.SetData("parameter", number(12.5));
    QCOMPARE(manager.GetContainer("parameter")->GetDataType(), QString("double"));
    InterfaceData data = manager.GetInterfaceData(&widget);
    QCOMPARE(data.GetDataType(), QString("double"));
    QCOMPARE(data.GetType(), QString("Parameter"));
    QCOMPARE(data.GetDouble(), 12.5);
    const auto entry = manager.GetContainerElementForms(0);
    QCOMPARE(entry.first, QString("parameter"));
    QCOMPARE(entry.second, std::vector<QString>{"widget-a"});
}

void DataManagementCharacterizationTests::DM_004_repeatedMappingsAndCleanup()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject widget;
    widget.setObjectName("widget");
    manager.AddContainerElement("one", "double", "Parameter", "");
    manager.AddContainerElement("two", "double", "Parameter", "");

    manager.AddElementToContainerEntry("widget", "one", "QWidget", &widget);
    manager.AddElementToContainerEntry("widget", "one", "QWidget", &widget);
    QCOMPARE(manager.GetContainerElementForms(0).second.size(), size_t(1));

    manager.AddElementToContainerEntry("widget", "two", "QWidget", &widget);
    QCOMPARE(manager.GetContainerID(&widget), QString("two"));
    QCOMPARE(manager.GetContainer("one")->Objects.size(), size_t(0));
    QCOMPARE(manager.GetContainer("two")->Objects.size(), size_t(1));
    manager.DeleteEntryOfObject(&widget);
    QVERIFY(!manager.IsObjectLinked(&widget));
    QCOMPARE(manager.GetContainer("two")->Objects.size(), size_t(0));

    manager.GetContainerID("ghost");
    QObject ghost;
    ghost.setObjectName("ghost");
    QVERIFY(manager.IsObjectLinked(&ghost));
    QCOMPARE(manager.GetContainer(&ghost), nullptr);
}

void DataManagementCharacterizationTests::DM_005_deviceLifetimeAndProjectCleanup()
{
    QObject owner;
    DataManagementClass manager(&owner);
    int destroyed = 0;
    manager.AddDevice("device", "device.xml", new PlatformProbe(&destroyed));
    QVERIFY(manager.GetDevice("device") != nullptr);
    QCOMPARE(manager.GetDevices(), QList<QString>{"device"});
    QCOMPARE(manager.GetDevicePaths(), QList<QString>{"device.xml"});
    manager.CloseDevice("device");
    QCOMPARE(destroyed, 1);
    QCOMPARE(manager.GetDevice("device"), nullptr);

    manager.AddDevice("cleanup", "cleanup.xml", new PlatformProbe(&destroyed));
    manager.AddContainerElement("cleanup-id", "double", "Parameter", "");
    manager.AddPlotPointer("plot", &owner, 0);
    manager.AddPlotWindow("window", 1, 1, 0);
    manager.AddFormFile({"form", "form.ui"});
    manager.CloseProjectLogic();
    QCOMPARE(destroyed, 2);
    QCOMPARE(manager.GetContainerCount(), 0);
    QCOMPARE(manager.PlotCount(), 0);
    QCOMPARE(manager.GetFormFileCount(), 0);
    QCOMPARE(manager.GetDevices().size(), 0);
    QCOMPARE(manager.GetPlotWindowsIncrementer(), 0);
}

void DataManagementCharacterizationTests::DM_DEV_001_registration_lookup_and_order_are_facade_observable()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QStringList lifecycle;
    auto* beta = new DeviceLifecycleProbe("beta", &lifecycle);
    auto* alpha = new DeviceLifecycleProbe("alpha", &lifecycle);

    manager.AddDevice("beta", "beta.xml", beta);
    manager.AddDevice("alpha", "alpha.xml", alpha);

    QCOMPARE(manager.GetDevice("alpha"), alpha);
    QCOMPARE(manager.GetDevice("alpha"), alpha);
    QCOMPARE(manager.GetDevice("beta"), beta);
    QCOMPARE(manager.GetDevice("unknown"), nullptr);
    QCOMPARE(manager.GetDevices(), QList<QString>({"alpha", "beta"}));
    QCOMPARE(manager.GetDevicePaths(), QList<QString>({"alpha.xml", "beta.xml"}));
    QCOMPARE(lifecycle, QStringList());

    manager.CloseProjectLogic();
    QCOMPARE(lifecycle, QStringList({"alpha:destroyed", "beta:destroyed"}));
}

void DataManagementCharacterizationTests::DM_DEV_002_duplicate_names_do_not_take_the_rejected_pointer()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QStringList lifecycle;
    auto* accepted = new DeviceLifecycleProbe("accepted", &lifecycle);
    auto* rejected = new DeviceLifecycleProbe("rejected", &lifecycle);

    manager.AddDevice("device", "first.xml", accepted);
    manager.AddDevice("device", "same-pointer.xml", accepted);
    manager.AddDevice("device", "rejected-pointer.xml", rejected);

    QCOMPARE(manager.GetDevice("device"), accepted);
    QCOMPARE(manager.GetDevices(), QList<QString>({"device"}));
    QCOMPARE(manager.GetDevicePaths(), QList<QString>({"first.xml"}));
    QCOMPARE(lifecycle, QStringList());

    manager.RemoveDevices();
    QCOMPARE(lifecycle, QStringList({"accepted:destroyed"}));
    QCOMPARE(manager.GetDevice("device"), nullptr);
    // RemoveDevices deliberately retains the legacy path map.
    QCOMPARE(manager.GetDevices(), QList<QString>({"device"}));
    QCOMPARE(manager.GetDevicePaths(), QList<QString>({"first.xml"}));

    // The rejected duplicate was never adopted by the manager; its creator
    // remains responsible for deleting it.
    delete rejected;
    QCOMPARE(lifecycle, QStringList({"accepted:destroyed", "rejected:destroyed"}));
    manager.CloseProjectLogic();
}

void DataManagementCharacterizationTests::DM_DEV_003_close_remove_and_reregistration_keep_legacy_path_state()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QStringList lifecycle;
    auto* alpha = new DeviceLifecycleProbe("alpha", &lifecycle);
    auto* beta = new DeviceLifecycleProbe("beta", &lifecycle);

    manager.AddDevice("alpha", "alpha.xml", alpha);
    manager.AddDevice("beta", "beta.xml", beta);
    manager.CloseDevice("unknown");
    QCOMPARE(lifecycle, QStringList());

    manager.CloseDevice("alpha");
    QCOMPARE(lifecycle, QStringList({"alpha:destroyed"}));
    QCOMPARE(manager.GetDevice("alpha"), nullptr);
    QCOMPARE(manager.GetDevices(), QList<QString>({"beta"}));
    manager.CloseDevice("alpha");
    QCOMPARE(lifecycle, QStringList({"alpha:destroyed"}));

    manager.RemoveDevices();
    QCOMPARE(lifecycle, QStringList({"alpha:destroyed", "beta:destroyed"}));
    QCOMPARE(manager.GetDevice("beta"), nullptr);
    QCOMPARE(manager.GetDevices(), QList<QString>({"beta"}));
    QCOMPARE(manager.GetDevicePaths(), QList<QString>({"beta.xml"}));

    auto* replacement = new DeviceLifecycleProbe("replacement", &lifecycle);
    manager.AddDevice("beta", "replacement.xml", replacement);
    QCOMPARE(manager.GetDevice("beta"), replacement);
    QCOMPARE(manager.GetDevicePaths(), QList<QString>({"replacement.xml"}));
    manager.CloseDevice("beta");
    QCOMPARE(lifecycle, QStringList({"alpha:destroyed", "beta:destroyed", "replacement:destroyed"}));
    QVERIFY(manager.GetDevices().isEmpty());
    QVERIFY(manager.GetDevicePaths().isEmpty());
}

void DataManagementCharacterizationTests::DM_DEV_004_project_cleanup_order_qobject_lifetime_and_instance_isolation()
{
    QObject firstOwner;
    QObject secondOwner;
    DataManagementClass first(&firstOwner);
    DataManagementClass second(&secondOwner);
    QStringList lifecycle;
    auto* providedObject = new QObject;
    QPointer<QObject> objectGuard(providedObject);

    first.AddDevice("zeta", "zeta.xml", new DeviceLifecycleProbe("first-zeta", &lifecycle));
    first.AddDevice("alpha", "alpha.xml", new DeviceLifecycleProbe("first-alpha", &lifecycle, providedObject));
    second.AddDevice("other", "other.xml", new DeviceLifecycleProbe("second-other", &lifecycle));
    QVERIFY(first.GetDevice("alpha")->GetObject() == providedObject);

    first.CloseProjectLogic();
    QCOMPARE(lifecycle, QStringList({"first-alpha:destroyed", "first-zeta:destroyed"}));
    QVERIFY(objectGuard);
    QVERIFY(first.GetDevices().isEmpty());
    QVERIFY(first.GetDevicePaths().isEmpty());
    QVERIFY(second.GetDevice("other") != nullptr);
    QCOMPARE(second.GetDevices(), QList<QString>({"other"}));

    delete providedObject;
    QVERIFY(objectGuard.isNull());
    second.CloseProjectLogic();
    QCOMPARE(lifecycle, QStringList({"first-alpha:destroyed", "first-zeta:destroyed", "second-other:destroyed"}));
}

void DataManagementCharacterizationTests::DM_006_messengerPublishSignalOrderAndValues()
{
    QObject owner;
    owner.setObjectName("owner");
    MessengerClass messenger(&owner);
    QSignalSpy added(&messenger, &MessengerClass::AddContainerElement);
    QSignalSpy set(&messenger, &MessengerClass::SetData);
    QSignalSpy widget(&messenger, &MessengerClass::AddElementToWidget);
    QSignalSpy received(&messenger, &MessengerClass::NewDataReceived);
    SignalOrderRecorder recorder;
    connect(&messenger, &MessengerClass::AddContainerElement, &recorder, &SignalOrderRecorder::added);
    connect(&messenger, &MessengerClass::SetData, &recorder, &SignalOrderRecorder::set);
    connect(&messenger, &MessengerClass::AddElementToWidget, &recorder, &SignalOrderRecorder::widget);
    connect(&messenger, &MessengerClass::NewDataReceived, &recorder, &SignalOrderRecorder::received);

    InterfaceData published = number(4.5);
    published.SetType("Parameter");
    published.SetStateDependency("state");
    messenger.MessageReceiver("publish", "id", published);

    QCOMPARE(added.count(), 1);
    QCOMPARE(set.count(), 2);
    QCOMPARE(widget.count(), 1);
    QCOMPARE(received.count(), 1);
    QCOMPARE(recorder.events, QStringList({"added", "set", "widget", "set", "received"}));
    QCOMPARE(added.at(0).at(0).toString(), QString("id"));
    QCOMPARE(added.at(0).at(1).toString(), QString("double"));
    QCOMPARE(added.at(0).at(2).toString(), QString("Parameter"));
    QCOMPARE(added.at(0).at(3).toString(), QString("state"));
    QCOMPARE(set.at(0).at(0).toString(), QString("id"));
    QCOMPARE(qvariant_cast<InterfaceData>(set.at(0).at(1)).GetDouble(), 4.5);
}

void DataManagementCharacterizationTests::DM_007_messengerForwardingAndSpecialCommands()
{
    QObject owner;
    owner.setObjectName("owner");
    MessengerClass messenger(&owner);
    QSignalSpy set(&messenger, &MessengerClass::SetData);
    QSignalSpy received(&messenger, &MessengerClass::NewDataReceived);
    QSignalSpy sent(&messenger, &MessengerClass::MessageSender);
    QSignalSpy errors(&messenger, &MessengerClass::ErrorWriter);
    QSignalSpy infos(&messenger, &MessengerClass::InfoWriter);
    QSignalSpy notifications(&messenger, &MessengerClass::NotificationWriter);
    QSignalSpy start(&messenger, &MessengerClass::PublishStart);
    QSignalSpy finished(&messenger, &MessengerClass::PublishFinished);
    SignalOrderRecorder recorder;
    connect(&messenger, &MessengerClass::SetData, &recorder, &SignalOrderRecorder::set);
    connect(&messenger, &MessengerClass::NewDataReceived, &recorder, &SignalOrderRecorder::received);
    connect(&messenger, &MessengerClass::MessageSender, &recorder, &SignalOrderRecorder::sent);

    messenger.MessageTransmitter("set", "id", number(3.0));
    QCOMPARE(recorder.events, QStringList({"set", "received", "sent"}));
    QCOMPARE(set.count(), 1);
    QCOMPARE(received.count(), 1);
    QCOMPARE(sent.count(), 1);
    QCOMPARE(sent.at(0).at(0).toString(), QString("set"));
    QCOMPARE(qvariant_cast<InterfaceData>(sent.at(0).at(2)).GetDouble(), 3.0);

    InterfaceData text;
    text.SetData(QString("message"));
    messenger.MessageReceiver("error", "origin", text);
    messenger.MessageReceiver("info", "origin", text);
    messenger.MessageReceiver("notification", "origin", text);
    messenger.MessageReceiver("publish_start", "", InterfaceData());
    messenger.MessageReceiver("publish_finished", "", InterfaceData());
    messenger.SendInfo("local-info");
    messenger.SendError("local-error");
    QCOMPARE(errors.count(), 2);
    QCOMPARE(infos.count(), 2);
    QCOMPARE(notifications.count(), 1);
    QCOMPARE(start.count(), 1);
    QCOMPARE(finished.count(), 1);
    QCOMPARE(errors.at(0).at(0).toString(), QString("origin"));
    QCOMPARE(errors.at(1).at(0).toString(), QString("owner"));
    QCOMPARE(infos.at(1).at(1).toString(), QString("local-info"));
    messenger.MessageReceiver("unknown", "id", InterfaceData());
    QCOMPARE(sent.count(), 1);
}

void DataManagementCharacterizationTests::DM_008_messengerDeviceRegistrationAndOwnership()
{
    auto* owner = new QObject;
    owner->setObjectName("owner");
    auto* messenger = new MessengerClass(owner);
    QPointer<MessengerClass> guard(messenger);
    DeviceProbe device;
    messenger->NewDeviceRegistration(&device);
    emit device.MessageSender("set", "from-device", number(8.0));
    QCOMPARE(device.receivedCount, 0);
    QSignalSpy sent(messenger, &MessengerClass::MessageSender);
    messenger->MessageTransmitter("get", "to-device", InterfaceData());
    QCOMPARE(device.receivedCount, 1);
    QCOMPARE(device.lastCommand, QString("get"));
    QCOMPARE(device.lastId, QString("to-device"));
    QCOMPARE(sent.count(), 1);

    delete owner;
    QVERIFY(guard.isNull());
}

void DataManagementCharacterizationTests::DM_009_setDataWidgetPropagationAndRequests()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    ProbeDropWidget widget;
    widget.setObjectName("widget");
    manager.AddContainerElement("parameter", "double", "Parameter", "");
    manager.AddElementToContainerEntry("widget", "parameter", "ProbeDropWidget", &widget);
    static_cast<DataManagementClass&>(manager).SetData("parameter", number(6.0));
    manager.SetData(QString("parameter"));
    QCOMPARE(widget.setCalls, 1);
    QCOMPARE(widget.lastReceived.GetDouble(), 6.0);
    manager.SetData(QString("missing"));
    QCOMPARE(widget.setCalls, 1);

    QSignalSpy requests(&manager, &DataManagementClass::MessageSender);
    connect(&widget, &ProbeDropWidget::requestUpdate, &manager, qOverload<>(&DataManagementSetClass::UpdateRequest));
    emit widget.requestUpdate();
    QCOMPARE(requests.count(), 1);
    QCOMPARE(requests.at(0).at(0).toString(), QString("get"));
    QCOMPARE(requests.at(0).at(1).toString(), QString("parameter"));
    manager.UpdateRequest(QString("explicit"));
    QCOMPARE(requests.count(), 2);
    QCOMPARE(requests.at(1).at(1).toString(), QString("explicit"));
}

void DataManagementCharacterizationTests::DM_010_setOwnershipAndRepeatedDataFlow()
{
    auto* owner = new QObject;
    owner->setObjectName("LabAnalyser");
    auto* manager = new DataManagementSetClass(owner);
    QPointer<MessengerClass> messenger(manager->GetMessenger());
    QVERIFY(messenger);
    QVERIFY(messenger->parent() == manager);

    ProbeDropWidget parameterWidget;
    parameterWidget.setObjectName("parameter-widget");
    parameterWidget.nextValue = number(9.0);
    manager->AddContainerElement("parameter", "double", "Parameter", "");
    manager->AddElementToContainerEntry("parameter-widget", "parameter", "ProbeDropWidget", &parameterWidget);
    QSignalSpy messages(manager, &DataManagementClass::MessageSender);
    connect(&parameterWidget, &ProbeDropWidget::changed, manager, &DataManagementSetClass::SendNewValue);
    emit parameterWidget.changed();
    emit parameterWidget.changed();
    QCOMPARE(parameterWidget.getCalls, 2);
    QCOMPARE(messages.count(), 2);
    QCOMPARE(messages.at(0).at(0).toString(), QString("set"));
    QCOMPARE(qvariant_cast<InterfaceData>(messages.at(1).at(2)).GetDouble(), 9.0);

    ProbeDropWidget dataWidget;
    dataWidget.setObjectName("data-widget");
    manager->AddContainerElement("data", "double", "Data", "");
    manager->AddElementToContainerEntry("data-widget", "data", "ProbeDropWidget", &dataWidget);
    connect(&dataWidget, &ProbeDropWidget::changed, manager, &DataManagementSetClass::SendNewValue);
    emit dataWidget.changed();
    QCOMPARE(dataWidget.getCalls, 0);
    QCOMPARE(messages.count(), 3);
    QCOMPARE(messages.at(2).at(0).toString(), QString("get"));
    QCOMPARE(messages.at(2).at(1).toString(), QString("data"));

    delete owner;
    QVERIFY(messenger.isNull());
}

void DataManagementCharacterizationTests::DM_SAFE_001_senderless_and_unknown_setter_slots_are_noops()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    QSignalSpy messages(&manager, &DataManagementClass::MessageSender);

    // The former direct calls dereferenced QObject::sender(); do not execute
    // that unsafe baseline in-process. The approved contract is a no-op.
    manager.SendNewValue();
    manager.UpdateRequest();
    QCOMPARE(messages.count(), 0);

    ProbeDropWidget unknown;
    unknown.setObjectName("unknown-widget");
    QObject::connect(&unknown, &ProbeDropWidget::changed,
                     &manager, &DataManagementSetClass::SendNewValue);
    QObject::connect(&unknown, &ProbeDropWidget::requestUpdate,
                     &manager, qOverload<>(&DataManagementSetClass::UpdateRequest));
    emit unknown.changed();
    emit unknown.requestUpdate();
    QCOMPARE(unknown.getCalls, 0);
    QCOMPARE(messages.count(), 0);

    manager.SetData(QString());
    manager.SetData(QString("missing"));
    QCOMPARE(messages.count(), 0);
}

void DataManagementCharacterizationTests::DM_SAFE_002_messenger_without_optional_parent_boundaries_is_safe()
{
    MessengerClass parentless(nullptr, nullptr);
    QSignalSpy parentlessInfo(&parentless, &MessengerClass::InfoWriter);
    QSignalSpy parentlessError(&parentless, &MessengerClass::ErrorWriter);
    QSignalSpy parentlessNotification(&parentless, &MessengerClass::NotificationWriter);
    QSignalSpy parentlessClose(&parentless, &MessengerClass::CloseProject);
    parentless.SendInfo(QString("ignored"));
    parentless.SendError(QString("ignored"));
    parentless.NewDeviceRegistration(nullptr);
    parentless.MessageReceiver("CloseProject", "manual", InterfaceData());
    QCOMPARE(parentlessInfo.count(), 0);
    QCOMPARE(parentlessError.count(), 0);
    QCOMPARE(parentlessNotification.count(), 0);
    QCOMPARE(parentlessClose.count(), 1);

    DataManagementSetClass parentlessManager(nullptr);
    QVERIFY(parentlessManager.GetMessenger());
    QSignalSpy parentlessManagerClose(parentlessManager.GetMessenger(), &MessengerClass::CloseProject);
    parentlessManager.GetMessenger()->MessageReceiver("CloseProject", "manual", InterfaceData());
    QCOMPARE(parentlessManagerClose.count(), 1);

    QObject root;
    root.setObjectName("root");
    MessengerClass direct(&root, nullptr);
    QSignalSpy directNotification(&direct, &MessengerClass::NotificationWriter);
    QSignalSpy directClose(&direct, &MessengerClass::CloseProject);
    direct.MessageReceiver("CloseProject", "manual", InterfaceData());
    QCOMPARE(directNotification.count(), 0);
    QCOMPARE(directClose.count(), 1);

    QObject application;
    application.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&application);
    QSignalSpy notifications(manager.GetMessenger(), &MessengerClass::NotificationWriter);
    QSignalSpy closed(manager.GetMessenger(), &MessengerClass::CloseProject);
    manager.GetMessenger()->MessageReceiver("CloseProject", "manual", InterfaceData());
    QCOMPARE(notifications.count(), 1);
    QCOMPARE(notifications.at(0).at(0).toString(), QString("LabAnalyser"));
    QCOMPARE(closed.count(), 1);
}

void DataManagementCharacterizationTests::DM_SAFE_003_null_widget_boundaries_do_not_create_or_dereference_bindings()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    manager.AddContainerElement("parameter", "double", "Parameter", "");
    const int beforeContainers = manager.GetContainerCount();

    // Null widget pointers were previously dereferenced by facade helpers.
    // Do not exercise that unsafe baseline; null input now has no binding side effect.
    manager.AddElementToContainerEntry("null-widget", "parameter", "ProbeDropWidget", nullptr);
    QCOMPARE(manager.GetContainerCount(), beforeContainers);
    QCOMPARE(manager.GetContainerID(static_cast<QObject*>(nullptr)), QString());
    QCOMPARE(manager.GetContainer(static_cast<QObject*>(nullptr)), nullptr);
    QCOMPARE(manager.GetObjectType(nullptr), QString());
    QVERIFY(!manager.IsObjectLinked(nullptr));
    manager.DeleteEntryOfObject(QString("parameter"), nullptr);
    manager.DeleteEntryOfObject(static_cast<QObject*>(nullptr));
    QVERIFY(manager.GetContainer("parameter") != nullptr);
    QCOMPARE(manager.GetContainer("parameter")->Objects.size(), size_t(0));

    manager.SetData(QString("parameter"));
    QCOMPARE(manager.GetContainerCount(), beforeContainers);
}

void DataManagementCharacterizationTests::DM_SAFE_004_registry_boundary_inputs_are_safe()
{
    QObject owner;
    DataManagementClass manager(&owner);
    const std::pair<QString, QString> emptyEntry;

    QCOMPARE(manager.GetFormFileEntry(-1), emptyEntry);
    QCOMPARE(manager.GetFormFileEntry(0), emptyEntry);
    manager.AddFormFile({"form", "form.ui"});
    QCOMPARE(manager.GetFormFileEntry(1), emptyEntry);

    QObject plot;
    plot.setObjectName("plot-object");
    manager.AddPlotPointer("plot#8", &plot, 7);
    manager.RenamePlotPointer("plot#8", "renamed-plot");
    QCOMPARE(manager.GetPlotByName("plot-object"), &plot);
    manager.DeletePlotPointer("renamed-plot");
    QCOMPARE(manager.PlotCount(), 0);
}

void DataManagementCharacterizationTests::DM_SAFE_005_missing_minmax_container_is_safe()
{
    QObject owner;
    DataManagementClass manager(&owner);
    const int beforeCount = manager.GetContainerCount();

    manager.SetMinMaxValue("missing", -3.0, 9.0);
    QCOMPARE(manager.MinMaxValue("missing"), std::make_pair(0.0, 0.0));
    QCOMPARE(manager.GetContainerCount(), beforeCount);

    manager.AddContainerElement("present", "double", "Parameter", "");
    manager.SetMinMaxValue("present", 9.0, -3.0);
    QCOMPARE(manager.MinMaxValue("present"), std::make_pair(9.0, -3.0));
}

void DataManagementCharacterizationTests::DM_REG_001_formFiles_preserve_order_duplicates_and_first_removal()
{
    QObject owner;
    DataManagementClass manager(&owner);

    manager.AddFormFile({"alpha", "alpha.ui"});
    manager.AddFormFile({"beta", "beta.ui"});
    manager.AddFormFile({"alpha", "alpha-second.ui"});
    QCOMPARE(manager.GetFormFileCount(), 3);
    QCOMPARE(manager.GetFormFileEntry(0), std::make_pair(QString("alpha"), QString("alpha.ui")));
    QCOMPARE(manager.GetFormFileEntry(1), std::make_pair(QString("beta"), QString("beta.ui")));
    QCOMPARE(manager.GetFormFileEntry(2), std::make_pair(QString("alpha"), QString("alpha-second.ui")));

    manager.RemoveFormFile("missing");
    QCOMPARE(manager.GetFormFileCount(), 3);
    manager.RemoveFormFile("alpha");
    QCOMPARE(manager.GetFormFileCount(), 2);
    QCOMPARE(manager.GetFormFileEntry(0), std::make_pair(QString("beta"), QString("beta.ui")));
    QCOMPARE(manager.GetFormFileEntry(1), std::make_pair(QString("alpha"), QString("alpha-second.ui")));
    manager.RemoveFormFile("alpha");
    QCOMPARE(manager.GetFormFileCount(), 1);
    QCOMPARE(manager.GetFormFileEntry(0), std::make_pair(QString("beta"), QString("beta.ui")));
}

void DataManagementCharacterizationTests::DM_REG_002_skipFormFlags_override_and_project_cleanup()
{
    QObject owner;
    DataManagementClass manager(&owner);

    QVERIFY(!manager.GetSkipFormFile("missing.ui"));
    manager.AddSkipFormFile("alpha.ui", true);
    QVERIFY(manager.GetSkipFormFile("alpha.ui"));
    manager.AddSkipFormFile("alpha.ui", false);
    QVERIFY(!manager.GetSkipFormFile("alpha.ui"));
    const QString unicodeName = QString::fromUtf8("Mäßwert Ω.ui");
    manager.AddSkipFormFile(unicodeName, true);
    QVERIFY(manager.GetSkipFormFile(unicodeName));
    manager.CloseProjectLogic();
    QVERIFY(!manager.GetSkipFormFile("alpha.ui"));
    QVERIFY(!manager.GetSkipFormFile(unicodeName));
}

void DataManagementCharacterizationTests::DM_REG_003_aliases_accept_unknown_empty_and_unicode_keys()
{
    QObject owner;
    DataManagementClass manager(&owner);

    QCOMPARE(manager.GetAlias("missing"), QString("missing"));
    manager.SetAlias("missing", "first");
    QCOMPARE(manager.GetAlias("missing"), QString("first"));
    manager.SetAlias("missing", "second");
    QCOMPARE(manager.GetAlias("missing"), QString("second"));
    manager.SetAlias("empty", "");
    QCOMPARE(manager.GetAlias("empty"), QString());
    const QString unicodeId = QString::fromUtf8("Gerät::Kanal Ω");
    const QString unicodeAlias = QString::fromUtf8("Mäßwert Ω");
    manager.SetAlias(unicodeId, unicodeAlias);
    QCOMPARE(manager.GetAlias(unicodeId), unicodeAlias);
    manager.CloseProjectLogic();
    QCOMPARE(manager.GetAlias("missing"), QString("missing"));
    QCOMPARE(manager.GetAlias("empty"), QString("empty"));
    QCOMPARE(manager.GetAlias(unicodeId), unicodeId);
}

void DataManagementCharacterizationTests::DM_REG_004_plot_and_window_numbers_keep_duplicate_history()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject first;
    QObject replacement;
    first.setObjectName("first");
    replacement.setObjectName("replacement");

    manager.AddPlotPointer("plot#1", &first, 0);
    manager.AddPlotPointer("plot#1", &replacement, 0);
    QCOMPARE(manager.PlotCount(), 1);
    QCOMPARE(manager.GetPlotByName("replacement"), &replacement);
    manager.DeletePlotPointer("plot#1");
    QCOMPARE(manager.PlotCount(), 0);
    QCOMPARE(manager.GetUniquePlotNumber(), 1);

    manager.AddPlotWindow("figure#0", 2, 3, 0);
    manager.AddPlotWindow("figure#0", 4, 5, 0);
    QCOMPARE(manager.GetPlotWindowRowsCols("figure#0"), std::make_pair(4, 5));
    manager.DeletePlotWindow("figure#0");
    QCOMPARE(manager.GetPlotWindowsIncrementer(), 1);
    QCOMPARE(manager.GetPlotWindowRowsCols("unknown"), std::make_pair(0, 0));
}

void DataManagementCharacterizationTests::DM_REG_005_registry_state_is_instance_local_and_recreated_empty()
{
    QObject owner;
    DataManagementClass first(&owner);
    DataManagementClass second(&owner);
    QObject plot;
    plot.setObjectName("first-plot");

    first.AddFormFile({"first", "first.ui"});
    first.AddSkipFormFile("first.ui", true);
    first.SetAlias("first-id", "first-alias");
    first.AddPlotPointer("plot#0", &plot, 0);
    QCOMPARE(second.GetFormFileCount(), 0);
    QVERIFY(!second.GetSkipFormFile("first.ui"));
    QCOMPARE(second.GetAlias("first-id"), QString("first-id"));
    QCOMPARE(second.PlotCount(), 0);

    first.CloseProjectLogic();
    QCOMPARE(second.GetFormFileCount(), 0);
    { auto* temporary = new DataManagementClass(&owner); temporary->AddFormFile({"temporary", "temporary.ui"}); temporary->SetAlias("temporary-id", "temporary-alias"); delete temporary; }
    DataManagementClass recreated(&owner);
    QCOMPARE(recreated.GetFormFileCount(), 0);
    QCOMPARE(recreated.GetAlias("temporary-id"), QString("temporary-id"));
    QCOMPARE(recreated.PlotCount(), 0);
}

void DataManagementCharacterizationTests::DM_CONT_001_containerPointerAndLookupSideEffects()
{
    QObject owner;
    DataManagementClass manager(&owner);

    auto* firstMap = manager.GetContainerPointer();
    auto* secondMap = manager.GetContainerPointer();
    QCOMPARE(firstMap, secondMap);
    QCOMPARE(firstMap->size(), size_t(0));
    QCOMPARE(manager.GetContainer("missing"), nullptr);
    QCOMPARE(firstMap->size(), size_t(0));

    manager.AddContainerElement("known", "double", "Parameter", "state");
    ToFormMapper* firstLookup = manager.GetContainer("known");
    ToFormMapper* secondLookup = manager.GetContainer("known");
    QVERIFY(firstLookup != nullptr);
    QCOMPARE(firstLookup, secondLookup);
    QCOMPARE(firstLookup, firstMap->at("known"));

    QObject missingWidget;
    missingWidget.setObjectName("missing-widget");
    QCOMPARE(manager.GetContainer(&missingWidget), nullptr);
    QVERIFY(firstMap->find(QString()) != firstMap->end());
    QCOMPARE(firstMap->at(QString()), nullptr);
}

void DataManagementCharacterizationTests::DM_CONT_002_replacementPreservesBindingsAndMinMaxButInvalidatesMapper()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject boundWidget;
    boundWidget.setObjectName("bound-widget");

    manager.AddContainerElement("id", "double", "Parameter", "old-state");
    manager.SetMinMaxValue("id", -2.0, 4.0);
    manager.SetData("id", number(3.5));
    manager.AddElementToContainerEntry("bound-widget", "id", "QWidget", &boundWidget);
    ToFormMapper* beforeReplacement = manager.GetContainer("id");
    QVERIFY(beforeReplacement != nullptr);

    manager.AddContainerElement("id", "int", "Data", "new-state");
    ToFormMapper* afterReplacement = manager.GetContainer("id");
    QVERIFY(afterReplacement != nullptr);
    QVERIFY(afterReplacement != beforeReplacement);
    QCOMPARE(manager.GetContainerPointer()->at("id"), afterReplacement);
    QCOMPARE(afterReplacement->GetType(), QString("Data"));
    QCOMPARE(afterReplacement->GetStateDependency(), QString("new-state"));
    QCOMPARE(manager.MinMaxValue("id"), std::make_pair(-2.0, 4.0));
    QCOMPARE(afterReplacement->Objects.size(), size_t(1));
    QCOMPARE(afterReplacement->Objects.at(0).FormP, &boundWidget);
    QCOMPARE(afterReplacement->Objects.at(0).FormName, QString("bound-widget"));
    // beforeReplacement was deleted by the current facade and must not be dereferenced.
}

void DataManagementCharacterizationTests::DM_CONT_003_multipleContainersUseMapOrderAndIndependentPointers()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject alphaWidget;
    QObject zetaWidget;
    alphaWidget.setObjectName("alpha-widget");
    zetaWidget.setObjectName("zeta-widget");

    manager.AddContainerElement("zeta", "double", "Data", "");
    manager.AddContainerElement("alpha", "double", "Parameter", "");
    manager.AddContainerElement("middle", "double", "State", "");
    manager.AddElementToContainerEntry("alpha-widget", "alpha", "QWidget", &alphaWidget);
    manager.AddElementToContainerEntry("zeta-widget", "zeta", "QWidget", &zetaWidget);

    QCOMPARE(manager.GetContainerCount(), 3);
    QCOMPARE(manager.GetContainer("alpha"), manager.GetContainerPointer()->at("alpha"));
    QVERIFY(manager.GetContainer("alpha") != manager.GetContainer("middle"));
    QVERIFY(manager.GetContainer("middle") != manager.GetContainer("zeta"));
    QCOMPARE(manager.GetContainerElementForms(0).first, QString("alpha"));
    QCOMPARE(manager.GetContainerElementForms(0).second, std::vector<QString>{"alpha-widget"});
    QCOMPARE(manager.GetContainerElementForms(1).first, QString("middle"));
    QCOMPARE(manager.GetContainerElementForms(1).second, std::vector<QString>{});
    QCOMPARE(manager.GetContainerElementForms(2).first, QString("zeta"));
    QCOMPARE(manager.GetContainerElementForms(2).second, std::vector<QString>{"zeta-widget"});
}

void DataManagementCharacterizationTests::DM_CONT_004_projectCleanupInvalidatesMappersButNotForeignWidgets()
{
    QObject owner;
    DataManagementClass manager(&owner);
    auto* foreignWidget = new QObject(&owner);
    QPointer<QObject> widgetGuard(foreignWidget);
    foreignWidget->setObjectName("foreign-widget");

    manager.AddContainerElement("id", "double", "Parameter", "");
    manager.AddElementToContainerEntry("foreign-widget", "id", "QWidget", foreignWidget);
    ToFormMapper* releasedMapper = manager.GetContainer("id");
    QVERIFY(releasedMapper != nullptr);
    manager.CloseProjectLogic();

    QCOMPARE(manager.GetContainerCount(), 0);
    QCOMPARE(manager.GetContainer("id"), nullptr);
    QVERIFY(widgetGuard);
    QCOMPARE(widgetGuard->parent(), &owner);
    // releasedMapper has been released by CloseProjectLogic and is intentionally not dereferenced.
}

void DataManagementCharacterizationTests::DM_CONT_005_managerDestructionAndContainerStateAreIsolated()
{
    QObject owner;
    auto* foreignWidget = new QObject(&owner);
    QPointer<QObject> widgetGuard(foreignWidget);
    foreignWidget->setObjectName("surviving-widget");

    auto* first = new DataManagementClass;
    QPointer<DataManagementClass> firstGuard(first);
    DataManagementClass second(&owner);
    first->AddContainerElement("shared-id", "double", "Parameter", "");
    first->AddElementToContainerEntry("surviving-widget", "shared-id", "QWidget", foreignWidget);
    second.AddContainerElement("shared-id", "double", "Data", "");
    ToFormMapper* releasedMapper = first->GetContainer("shared-id");
    QVERIFY(releasedMapper != nullptr);
    QCOMPARE(second.GetContainer("shared-id")->GetType(), QString("Data"));

    delete first;
    QVERIFY(firstGuard.isNull());
    QVERIFY(widgetGuard);
    QCOMPARE(widgetGuard->parent(), &owner);
    QCOMPARE(second.GetContainerCount(), 1);
    QCOMPARE(second.GetContainer("shared-id")->GetType(), QString("Data"));
    // releasedMapper belonged to first and is intentionally not dereferenced after destruction.
}

void DataManagementCharacterizationTests::DM_BIND_001_bindingIsNameKeyedAndPreservesBoundQObjectPointer()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject bound;
    QObject sameNameDifferentPointer;
    QObject unknown;
    bound.setObjectName("bound-name");
    sameNameDifferentPointer.setObjectName("bound-name");
    unknown.setObjectName("unknown-name");

    manager.AddContainerElement("id", "double", "Parameter", "");
    QVERIFY(!manager.IsObjectLinked(&bound));
    QCOMPARE(manager.GetContainerID(&bound), QString());
    manager.AddElementToContainerEntry("bound-name", "id", "QWidget", &bound);

    QVERIFY(manager.IsObjectLinked(&bound));
    QCOMPARE(manager.GetContainerID(&bound), QString("id"));
    QCOMPARE(manager.GetContainer(&bound), manager.GetContainer("id"));
    QCOMPARE(manager.GetContainer("id")->Objects.size(), size_t(1));
    QCOMPARE(manager.GetContainer("id")->Objects.at(0).FormP, &bound);
    QCOMPARE(manager.GetContainer("id")->Objects.at(0).FormName, QString("bound-name"));

    // Binding lookup is keyed by objectName, not by QObject pointer identity.
    QVERIFY(manager.IsObjectLinked(&sameNameDifferentPointer));
    QCOMPARE(manager.GetContainerID(&sameNameDifferentPointer), QString("id"));
    QCOMPARE(manager.GetContainer(&sameNameDifferentPointer), manager.GetContainer("id"));

    QCOMPARE(manager.GetContainerID(&unknown), QString());
    QCOMPARE(manager.GetContainer(&unknown), nullptr);
    QVERIFY(manager.IsObjectLinked(&unknown));
    QVERIFY(manager.GetContainerPointer()->find(QString()) != manager.GetContainerPointer()->end());
    QCOMPARE(manager.GetContainerPointer()->at(QString()), nullptr);
}

void DataManagementCharacterizationTests::DM_BIND_002_rebindingAndRemovalKeepCurrentLegacySideEffects()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject widget;
    widget.setObjectName("widget");
    manager.AddContainerElement("one", "double", "Parameter", "");
    manager.AddContainerElement("two", "double", "Parameter", "");

    manager.AddElementToContainerEntry("widget", "one", "QWidget", &widget);
    manager.AddElementToContainerEntry("widget", "one", "QWidget", &widget);
    QCOMPARE(manager.GetContainer("one")->Objects.size(), size_t(1));
    QCOMPARE(manager.GetContainer("one")->Objects.at(0).FormP, &widget);

    manager.AddElementToContainerEntry("widget", "two", "QWidget", &widget);
    QCOMPARE(manager.GetContainerID(&widget), QString("two"));
    QCOMPARE(manager.GetContainer("one")->Objects.size(), size_t(0));
    QCOMPARE(manager.GetContainer("two")->Objects.size(), size_t(1));
    QCOMPARE(manager.GetContainer("two")->Objects.at(0).FormP, &widget);

    // The ID-specific removal changes the mapper list only; it leaves the
    // name-to-ID lookup in place until the QObject overload removes it.
    manager.DeleteEntryOfObject("two", &widget);
    QCOMPARE(manager.GetContainer("two")->Objects.size(), size_t(0));
    QVERIFY(manager.IsObjectLinked(&widget));
    QCOMPARE(manager.GetContainerID(&widget), QString("two"));
    manager.DeleteEntryOfObject(&widget);
    QVERIFY(!manager.IsObjectLinked(&widget));
    QCOMPARE(manager.GetContainer("two")->Objects.size(), size_t(0));
}

void DataManagementCharacterizationTests::DM_BIND_003_destroyedBoundQObjectIsNotAutoCleanedButProjectCleanupIs()
{
    QObject owner;
    DataManagementClass manager(&owner);
    auto* bound = new QObject(&owner);
    QPointer<QObject> boundGuard(bound);
    bound->setObjectName("destroyed-widget");
    manager.AddContainerElement("id", "double", "Parameter", "");
    manager.AddElementToContainerEntry("destroyed-widget", "id", "QWidget", bound);
    QCOMPARE(manager.GetContainer("id")->Objects.size(), size_t(1));

    delete bound;
    QVERIFY(boundGuard.isNull());
    // Do not dereference the stale ObjectStruct pointer. A fresh object with
    // the same name demonstrates that no destroyed(QObject*) cleanup exists.
    QObject replacement;
    replacement.setObjectName("destroyed-widget");
    QVERIFY(manager.IsObjectLinked(&replacement));
    QCOMPARE(manager.GetContainerID(&replacement), QString("id"));
    QCOMPARE(manager.GetContainerElementForms(0).second, std::vector<QString>{"destroyed-widget"});

    manager.CloseProjectLogic();
    QCOMPARE(manager.GetContainerCount(), 0);
    QVERIFY(!manager.IsObjectLinked(&replacement));
}

void DataManagementCharacterizationTests::DM_BIND_004_boundWidgetsRouteMessagesAndExposePlotDuplicatePropagation()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    ProbeDropWidget parameterWidget;
    parameterWidget.setObjectName("parameter-widget");
    parameterWidget.nextValue = number(7.0);
    manager.AddContainerElement("parameter", "double", "Parameter", "");
    manager.AddElementToContainerEntry("parameter-widget", "parameter", "ProbeDropWidget", &parameterWidget);
    manager.AddElementToContainerEntry("parameter-widget", "parameter", "ProbeDropWidget", &parameterWidget);
    QCOMPARE(manager.GetContainer("parameter")->Objects.size(), size_t(1));

    QSignalSpy messages(&manager, &DataManagementClass::MessageSender);
    connect(&parameterWidget, &ProbeDropWidget::changed, &manager, &DataManagementSetClass::SendNewValue);
    emit parameterWidget.changed();
    QCOMPARE(parameterWidget.getCalls, 1);
    QCOMPARE(messages.count(), 1);
    QCOMPARE(messages.at(0).at(0).toString(), QString("set"));
    QCOMPARE(messages.at(0).at(1).toString(), QString("parameter"));
    QCOMPARE(qvariant_cast<InterfaceData>(messages.at(0).at(2)).GetDouble(), 7.0);
    // The existing Manager -> Messenger route feeds a successful "set" back
    // into the bound widget once before the test performs an explicit update.
    QCOMPARE(parameterWidget.setCalls, 1);

    static_cast<DataManagementClass&>(manager).SetData("parameter", number(3.0));
    manager.SetData(QString("parameter"));
    QCOMPARE(parameterWidget.setCalls, 2);

    ProbeDropWidget plotWidget;
    plotWidget.setObjectName("plot-widget");
    manager.AddContainerElement("plot", "double", "Parameter", "");
    // The current PlotWidget class-name exception does not detach the prior
    // entry, so repeated registration propagates manager updates twice.
    manager.AddElementToContainerEntry("plot-widget", "plot", "PlotWidget", &plotWidget);
    manager.AddElementToContainerEntry("plot-widget", "plot", "PlotWidget", &plotWidget);
    QCOMPARE(manager.GetContainer("plot")->Objects.size(), size_t(2));
    static_cast<DataManagementClass&>(manager).SetData("plot", number(4.0));
    manager.SetData(QString("plot"));
    QCOMPARE(plotWidget.setCalls, 2);

    connect(&plotWidget, &ProbeDropWidget::changed, &manager, &DataManagementSetClass::SendNewValue);
    emit plotWidget.changed();
    QCOMPARE(plotWidget.getCalls, 1);
    QCOMPARE(messages.count(), 2);
    QCOMPARE(messages.at(1).at(0).toString(), QString("set"));
    QCOMPARE(messages.at(1).at(1).toString(), QString("plot"));
}

void DataManagementCharacterizationTests::DM_BIND_005_bindingStateAndForeignQObjectOwnershipAreInstanceLocal()
{
    QObject owner;
    auto* foreign = new QObject(&owner);
    QPointer<QObject> foreignGuard(foreign);
    foreign->setObjectName("foreign-binding");
    auto* first = new DataManagementClass;
    DataManagementClass second(&owner);
    first->AddContainerElement("id", "double", "Parameter", "");
    first->AddElementToContainerEntry("foreign-binding", "id", "QWidget", foreign);

    QVERIFY(first->IsObjectLinked(foreign));
    QVERIFY(!second.IsObjectLinked(foreign));
    delete first;
    QVERIFY(foreignGuard);
    QCOMPARE(foreignGuard->parent(), &owner);
    QVERIFY(!second.IsObjectLinked(foreign));
    second.AddContainerElement("id", "double", "Parameter", "");
    second.AddElementToContainerEntry("foreign-binding", "id", "QWidget", foreign);
    QVERIFY(second.IsObjectLinked(foreign));
}

void DataManagementCharacterizationTests::DM_BIND_006_nameCollisionsRenamesEmptyNamesAndRoutingUseCurrentName()
{
    QObject owner;
    DataManagementClass manager(&owner);
    QObject first;
    QObject second;
    QObject emptyName;
    first.setObjectName("shared-name");
    second.setObjectName("shared-name");

    manager.AddContainerElement("first-id", "double", "Parameter", "");
    manager.AddContainerElement("second-id", "double", "Parameter", "");
    manager.AddContainerElement("empty-id", "double", "Parameter", "");
    manager.AddElementToContainerEntry("shared-name", "first-id", "QWidget", &first);
    QCOMPARE(manager.GetContainer("first-id")->Objects.size(), size_t(1));
    QCOMPARE(manager.GetContainer("first-id")->Objects.at(0).FormP, &first);

    // A second live QObject with the same name replaces the name binding and
    // removes the first mapper entry by matching its stored FormName.
    manager.AddElementToContainerEntry("shared-name", "second-id", "QWidget", &second);
    QCOMPARE(manager.GetContainer("first-id")->Objects.size(), size_t(0));
    QCOMPARE(manager.GetContainer("second-id")->Objects.size(), size_t(1));
    QCOMPARE(manager.GetContainer("second-id")->Objects.at(0).FormP, &second);
    QCOMPARE(manager.GetContainerID(&first), QString("second-id"));
    QCOMPARE(manager.GetContainerID(&second), QString("second-id"));
    QCOMPARE(manager.GetContainer(&first), manager.GetContainer("second-id"));

    second.setObjectName("renamed");
    QVERIFY(!manager.IsObjectLinked(&second));
    QCOMPARE(manager.GetContainerID(&second), QString());
    QCOMPARE(manager.GetContainer(&second), nullptr);
    QVERIFY(manager.IsObjectLinked(&second));
    QCOMPARE(manager.GetContainerPointer()->at(QString()), nullptr);
    // The old mapping remains addressable by another QObject with its old name.
    QCOMPARE(manager.GetContainerID(&first), QString("second-id"));
    QCOMPARE(manager.GetContainer("second-id")->Objects.at(0).FormP, &second);

    // The newly inserted renamed-name mapping is empty, so removal deletes it
    // only; it cannot remove the mapper object stored under the old name.
    manager.DeleteEntryOfObject(&second);
    QVERIFY(!manager.IsObjectLinked(&second));
    QCOMPARE(manager.GetContainer("second-id")->Objects.size(), size_t(1));
    second.setObjectName("shared-name");
    QVERIFY(manager.IsObjectLinked(&second));
    QCOMPARE(manager.GetContainerID(&second), QString("second-id"));
    manager.DeleteEntryOfObject(&second);
    QVERIFY(!manager.IsObjectLinked(&second));
    QCOMPARE(manager.GetContainer("second-id")->Objects.size(), size_t(0));

    // Empty object names are legitimate map keys and have the same ordinary
    // bind/lookup/remove behavior as non-empty names.
    QVERIFY(emptyName.objectName().isEmpty());
    manager.AddElementToContainerEntry(QString(), "empty-id", "QWidget", &emptyName);
    QVERIFY(manager.IsObjectLinked(&emptyName));
    QCOMPARE(manager.GetContainerID(&emptyName), QString("empty-id"));
    QCOMPARE(manager.GetContainer(&emptyName), manager.GetContainer("empty-id"));
    QCOMPARE(manager.GetContainer("empty-id")->Objects.at(0).FormP, &emptyName);
    manager.DeleteEntryOfObject(&emptyName);
    QVERIFY(!manager.IsObjectLinked(&emptyName));
    QCOMPARE(manager.GetContainer("empty-id")->Objects.size(), size_t(0));

    QObject routeOwner;
    routeOwner.setObjectName("LabAnalyser");
    DataManagementSetClass routeManager(&routeOwner);
    ProbeDropWidget routeWidget;
    routeWidget.setObjectName("route-original");
    routeWidget.nextValue = number(8.0);
    routeManager.AddContainerElement("route-id", "double", "Parameter", "");
    routeManager.AddElementToContainerEntry("route-original", "route-id", "ProbeDropWidget", &routeWidget);
    QSignalSpy routed(&routeManager, &DataManagementClass::MessageSender);
    connect(&routeWidget, &ProbeDropWidget::changed, &routeManager, &DataManagementSetClass::SendNewValue);

    routeWidget.setObjectName("route-renamed");
    QVERIFY(!routeManager.IsObjectLinked(&routeWidget));
    emit routeWidget.changed();
    QCOMPARE(routeWidget.getCalls, 0);
    QCOMPARE(routed.count(), 0);
    routeWidget.setObjectName("route-original");
    QVERIFY(routeManager.IsObjectLinked(&routeWidget));
    emit routeWidget.changed();
    QCOMPARE(routeWidget.getCalls, 1);
    QCOMPARE(routed.count(), 1);
    QCOMPARE(routed.at(0).at(0).toString(), QString("set"));
    QCOMPARE(routed.at(0).at(1).toString(), QString("route-id"));

    // Name-collision and rename state do not leak across manager instances.
    DataManagementClass isolated(&owner);
    QCOMPARE(isolated.GetContainerID(&first), QString());
    QVERIFY(!isolated.IsObjectLinked(&first));
    QCOMPARE(isolated.GetContainer(&first), nullptr);
    QVERIFY(isolated.IsObjectLinked(&first));
    QCOMPARE(isolated.GetContainerPointer()->at(QString()), nullptr);
}

void DataManagementCharacterizationTests::DM_MSG_001_messageReceiver_commandMatrix()
{
    struct CommandCase {
        QString command;
        QStringList events;
        bool createsContainer = false;
        bool requiresExistingContainer = false;
    };
    const std::vector<CommandCase> cases = {
        {"publish", {"added", "set", "widget", "set", "received"}, true, false},
        {"set", {"set", "received"}, false, true},
        {"get", {}, false, false},
        {"error", {"error"}, false, false},
        {"info", {"info"}, false, false},
        {"notification", {"notification"}, false, false},
        {"CloseProject", {"notification", "close"}, false, false},
        {"publish_start", {"start"}, false, false},
        {"publish_finished", {"finished"}, false, false},
    };

    for (const CommandCase& commandCase : cases) {
        QObject application;
        application.setObjectName("LabAnalyser");
        DataManagementSetClass manager(&application);
        MessengerClass* messenger = manager.GetMessenger();
        QVERIFY(messenger);
        QCOMPARE(messenger->parent(), static_cast<QObject*>(&manager));
        QCOMPARE(manager.parent(), &application);

        const QString id = QString("message::%1").arg(commandCase.command);
        InterfaceData payload = number(4.5);
        payload.SetType("Parameter");
        payload.SetStateDependency("state");

        if (commandCase.requiresExistingContainer)
            manager.AddContainerElement(id, "double", "Parameter", "state");

        QSignalSpy added(messenger, &MessengerClass::AddContainerElement);
        QSignalSpy set(messenger, &MessengerClass::SetData);
        QSignalSpy widget(messenger, &MessengerClass::AddElementToWidget);
        QSignalSpy received(messenger, &MessengerClass::NewDataReceived);
        QSignalSpy errors(messenger, &MessengerClass::ErrorWriter);
        QSignalSpy infos(messenger, &MessengerClass::InfoWriter);
        QSignalSpy notifications(messenger, &MessengerClass::NotificationWriter);
        QSignalSpy closed(messenger, &MessengerClass::CloseProject);
        QSignalSpy started(messenger, &MessengerClass::PublishStart);
        QSignalSpy finished(messenger, &MessengerClass::PublishFinished);
        SignalOrderRecorder recorder;
        connect(messenger, &MessengerClass::AddContainerElement, &recorder, &SignalOrderRecorder::added);
        connect(messenger, &MessengerClass::SetData, &recorder, &SignalOrderRecorder::set);
        connect(messenger, &MessengerClass::AddElementToWidget, &recorder, &SignalOrderRecorder::widget);
        connect(messenger, &MessengerClass::NewDataReceived, &recorder, &SignalOrderRecorder::received);
        connect(messenger, &MessengerClass::ErrorWriter, &recorder, &SignalOrderRecorder::error);
        connect(messenger, &MessengerClass::InfoWriter, &recorder, &SignalOrderRecorder::info);
        connect(messenger, &MessengerClass::NotificationWriter, &recorder, &SignalOrderRecorder::notification);
        connect(messenger, &MessengerClass::CloseProject, &recorder, &SignalOrderRecorder::closeProject);
        connect(messenger, &MessengerClass::PublishStart, &recorder, &SignalOrderRecorder::publishStart);
        connect(messenger, &MessengerClass::PublishFinished, &recorder, &SignalOrderRecorder::publishFinished);

        messenger->MessageReceiver(commandCase.command, id, payload);
        QCOMPARE(recorder.events, commandCase.events);

        QCOMPARE(added.count(), commandCase.events.count("added"));
        QCOMPARE(set.count(), commandCase.events.count("set"));
        QCOMPARE(widget.count(), commandCase.events.count("widget"));
        QCOMPARE(received.count(), commandCase.events.count("received"));
        QCOMPARE(errors.count(), commandCase.events.count("error"));
        QCOMPARE(infos.count(), commandCase.events.count("info"));
        QCOMPARE(notifications.count(), commandCase.events.count("notification"));
        QCOMPARE(closed.count(), commandCase.events.count("close"));
        QCOMPARE(started.count(), commandCase.events.count("start"));
        QCOMPARE(finished.count(), commandCase.events.count("finished"));

        if (commandCase.command == "publish") {
            QCOMPARE(added.at(0).at(0).toString(), id);
            QCOMPARE(added.at(0).at(1).toString(), QString("double"));
            QCOMPARE(added.at(0).at(2).toString(), QString("Parameter"));
            QCOMPARE(added.at(0).at(3).toString(), QString("state"));
            QCOMPARE(qvariant_cast<InterfaceData>(set.at(0).at(1)).GetDouble(), 4.5);
            QCOMPARE(manager.GetContainer(id)->GetDouble(), 4.5);
        } else if (commandCase.command == "set") {
            QCOMPARE(set.at(0).at(0).toString(), id);
            QCOMPARE(qvariant_cast<InterfaceData>(set.at(0).at(1)).GetDouble(), 4.5);
            QCOMPARE(manager.GetContainer(id)->GetDouble(), 4.5);
        } else if (commandCase.command == "error" || commandCase.command == "info") {
            const QList<QVariant> signal = commandCase.command == "error" ? errors.at(0) : infos.at(0);
            QCOMPARE(signal.at(0).toString(), id);
            QCOMPARE(signal.at(1).toString(), QString("4.5"));
        } else if (commandCase.command == "notification") {
            QCOMPARE(notifications.at(0).at(0).toString(), id);
            QCOMPARE(notifications.at(0).at(1).toString(), QString("4.5"));
        } else if (commandCase.command == "CloseProject") {
            QCOMPARE(notifications.at(0).at(0).toString(), QString("LabAnalyser"));
            QCOMPARE(notifications.at(0).at(1).toString(), QString("Closing forced by: %1").arg(id));
        } else {
            QVERIFY(!manager.ElementExists(id));
        }

        messenger->MessageReceiver(commandCase.command, id, payload);
        QCOMPARE(recorder.events, commandCase.events + commandCase.events);
    }
}

void DataManagementCharacterizationTests::DM_MSG_002_messageTransmitter_commandMatrix()
{
    struct CommandCase { QString command; QStringList receiverEvents; };
    const std::vector<CommandCase> cases = {
        {"publish", {"added", "set", "widget", "set", "received"}},
        {"set", {"set", "received"}}, {"get", {}}, {"error", {"error"}},
        {"info", {"info"}}, {"notification", {"notification"}},
        {"CloseProject", {"notification", "close"}}, {"publish_start", {"start"}},
        {"publish_finished", {"finished"}},
    };

    for (const CommandCase& commandCase : cases) {
        QObject application;
        application.setObjectName("LabAnalyser");
        DataManagementSetClass manager(&application);
        MessengerClass* messenger = manager.GetMessenger();
        const QString id = QString("transmit::%1").arg(commandCase.command);
        InterfaceData payload;
        payload.SetData(QString("payload \\u03bc"));
        if (commandCase.command == "set")
            manager.AddContainerElement(id, "String", "Parameter", "");

        QSignalSpy sent(messenger, &MessengerClass::MessageSender);
        SignalOrderRecorder recorder;
        connect(messenger, &MessengerClass::AddContainerElement, &recorder, &SignalOrderRecorder::added);
        connect(messenger, &MessengerClass::SetData, &recorder, &SignalOrderRecorder::set);
        connect(messenger, &MessengerClass::AddElementToWidget, &recorder, &SignalOrderRecorder::widget);
        connect(messenger, &MessengerClass::NewDataReceived, &recorder, &SignalOrderRecorder::received);
        connect(messenger, &MessengerClass::ErrorWriter, &recorder, &SignalOrderRecorder::error);
        connect(messenger, &MessengerClass::InfoWriter, &recorder, &SignalOrderRecorder::info);
        connect(messenger, &MessengerClass::NotificationWriter, &recorder, &SignalOrderRecorder::notification);
        connect(messenger, &MessengerClass::CloseProject, &recorder, &SignalOrderRecorder::closeProject);
        connect(messenger, &MessengerClass::PublishStart, &recorder, &SignalOrderRecorder::publishStart);
        connect(messenger, &MessengerClass::PublishFinished, &recorder, &SignalOrderRecorder::publishFinished);
        connect(messenger, &MessengerClass::MessageSender, &recorder, &SignalOrderRecorder::sent);

        messenger->MessageTransmitter(commandCase.command, id, payload);
        QStringList expected = commandCase.receiverEvents;
        expected << "sent";
        QCOMPARE(recorder.events, expected);
        QCOMPARE(sent.count(), 1);
        QCOMPARE(sent.at(0).at(0).toString(), commandCase.command);
        QCOMPARE(sent.at(0).at(1).toString(), id);
        QCOMPARE(qvariant_cast<InterfaceData>(sent.at(0).at(2)).GetDataType(), QString("QString"));
        QCOMPARE(qvariant_cast<InterfaceData>(sent.at(0).at(2)).GetString(), QString("payload \\u03bc"));

        messenger->MessageTransmitter(commandCase.command, id, payload);
        QCOMPARE(sent.count(), 2);
        QCOMPARE(recorder.events, expected + expected);
    }
}

void DataManagementCharacterizationTests::DM_MSG_003_parentHierarchy_emptyInputs_and_mixedSequence()
{
    QObject application;
    application.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&application);
    MessengerClass* messenger = manager.GetMessenger();
    QVERIFY(messenger);
    QCOMPARE(messenger->parent()->parent(), static_cast<QObject*>(&application));

    QSignalSpy added(messenger, &MessengerClass::AddContainerElement);
    QSignalSpy set(messenger, &MessengerClass::SetData);
    QSignalSpy sent(messenger, &MessengerClass::MessageSender);
    QSignalSpy notifications(messenger, &MessengerClass::NotificationWriter);
    SignalOrderRecorder recorder;
    connect(messenger, &MessengerClass::AddContainerElement, &recorder, &SignalOrderRecorder::added);
    connect(messenger, &MessengerClass::SetData, &recorder, &SignalOrderRecorder::set);
    connect(messenger, &MessengerClass::MessageSender, &recorder, &SignalOrderRecorder::sent);
    connect(messenger, &MessengerClass::NotificationWriter, &recorder, &SignalOrderRecorder::notification);

    InterfaceData emptyPayload;
    messenger->MessageReceiver("", "", emptyPayload);
    messenger->MessageReceiver("unknown", "", emptyPayload);
    QCOMPARE(recorder.events, QStringList());
    QCOMPARE(manager.GetContainerCount(), 0);

    InterfaceData text;
    text.SetData(QString("safe text"));
    messenger->MessageReceiver("publish", "", text);
    messenger->MessageReceiver("set", "", number(9.0));
    messenger->MessageTransmitter("get", "", emptyPayload);
    InterfaceData textList;
    textList.SetData(QStringList({"first", "\u03bc"}));
    messenger->MessageTransmitter("notification", "list", textList);
    messenger->MessageReceiver("CloseProject", "manual", emptyPayload);

    QCOMPARE(recorder.events, QStringList({"added", "set", "set", "set", "sent", "notification", "sent", "notification"}));
    QCOMPARE(added.count(), 1);
    QCOMPARE(set.count(), 3);
    QCOMPARE(sent.count(), 2);
    QCOMPARE(notifications.count(), 2);
    QVERIFY(manager.ElementExists(QString()));
    QCOMPARE(manager.GetContainer(QString())->GetDouble(), 9.0);
    QCOMPARE(notifications.at(0).at(0).toString(), QString("list"));
    QCOMPARE(qvariant_cast<InterfaceData>(sent.at(1).at(2)).GetDataType(), QString("QStringList"));
    QCOMPARE(qvariant_cast<InterfaceData>(sent.at(1).at(2)).GetStringList(), QStringList({"first", "\u03bc"}));
    QCOMPARE(notifications.at(1).at(0).toString(), QString("LabAnalyser"));
    QCOMPARE(notifications.at(1).at(1).toString(), QString("Closing forced by: manual"));
}

QTEST_MAIN(DataManagementCharacterizationTests)

#include "DataManagementCharacterizationTests.moc"
