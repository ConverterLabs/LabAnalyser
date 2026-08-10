#include <QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "mainwindow.h"
#include "DataManagement/UIDataManagementSetClass.h"
#include "DataManagement/DataMessengerClass.h"

namespace
{
QString fixturePath(const QString& name)
{
    return QFINDTESTDATA("../../fixtures/xml/" + name);
}

QString copyFixture(const QString& source, QTemporaryDir& directory, const QString& target)
{
    const QString path = directory.filePath(target);
    return QFile::copy(source, path) ? path : QString();
}

class ProcessStateGuard final
{
public:
    ProcessStateGuard() : workingDirectory(QDir::currentPath()), locale(QLocale()) {}
    ~ProcessStateGuard()
    {
        QDir::setCurrent(workingDirectory);
        QLocale::setDefault(locale);
    }

    QString workingDirectory;
    QLocale locale;
};

class MessageOrderRecorder final : public QObject
{
    Q_OBJECT
public:
    QStringList events;

public slots:
    void info(const QString&, const QString&) { events << "info"; }
    void error(const QString&, const QString&) { events << "error"; }
};

InterfaceData number(double value)
{
    InterfaceData data;
    data.SetData(value);
    return data;
}

void addData(DataManagementClass& manager, const QString& id, const QString& type, InterfaceData value)
{
    manager.AddContainerElement(id, value.GetDataType(), type, QString());
    manager.SetData(id, value);
}

QString deviceXml(const QString& libraryPath, const QString& name)
{
    return QString("<LEDevice DevicePlugin=\"%1\" DeviceName=\"%2\"/>").arg(libraryPath, name);
}

QString writeTextFile(QTemporaryDir& directory, const QString& name, const QString& text)
{
    const QString path = directory.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return QString();
    file.write(text.toUtf8());
    return path;
}
}

class ProjectIoFacadeContractTests final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void UIIO_001_publicInitialStateDoesNotExposeFacadePrivatePaths();
    void UIIO_002_experimentSaveLoadAndFormRouting();
    void UIIO_003_invalidAndPartialExperimentLoadsPreserveProcessState();
    void UIIO_004_parameterXmlFacadeRoutingAndConventions();
    void UIIO_005_matAndHdf5FacadeConventions();
    void UIIO_006_pluginFacadeRoutingAndConventions();

private:
    QString pluginRoot;
};

void ProjectIoFacadeContractTests::initTestCase()
{
    qRegisterMetaType<InterfaceData>("InterfaceData");
    pluginRoot = qEnvironmentVariable("LABANALYSER_TEST_PLUGIN_ROOT");
    QVERIFY2(!pluginRoot.isEmpty(), "LABANALYSER_TEST_PLUGIN_ROOT is required");
}

void ProjectIoFacadeContractTests::UIIO_001_publicInitialStateDoesNotExposeFacadePrivatePaths()
{
    ProcessStateGuard guard;
    MainWindow window;
    UIDataManagementSetClass* logic = window.GetLogic();
    QVERIFY(logic != nullptr);
    QCOMPARE(logic->metaObject()->indexOfProperty("LoadPath"), -1);
    QCOMPARE(logic->metaObject()->indexOfProperty("StdSavePath"), -1);
    QCOMPARE(logic->metaObject()->indexOfProperty("ChangeDetected"), -1);
    QCOMPARE(logic->metaObject()->indexOfMethod("LoadForms()"), -1);
    QCOMPARE(QDir::currentPath(), guard.workingDirectory);
    QCOMPARE(QLocale().name(), guard.locale.name());
}

void ProjectIoFacadeContractTests::UIIO_002_experimentSaveLoadAndFormRouting()
{
    QTemporaryDir directory;
    ProcessStateGuard guard;
    QVERIFY(directory.isValid());
    QVERIFY(!copyFixture(fixturePath("embedded-form.ui"), directory, "embedded-form.ui").isEmpty());
    const QString experiment = copyFixture(fixturePath("legacy-complete.xml"), directory, "legacy.xml");
    QVERIFY(!experiment.isEmpty());

    MainWindow window;
    UIDataManagementSetClass* logic = window.GetLogic();
    QSignalSpy facadeLoads(logic, &UIDataManagementSetClass::LoadFormFromXML);
    QVERIFY(facadeLoads.isValid());
    const QString saved = directory.filePath("saved.xml");

    QVERIFY(!logic->SaveExperiment(saved));
    QVERIFY(QFileInfo::exists(saved));
    QVERIFY(!logic->LoadExperiment(experiment));
    QCOMPARE(logic->GetFormFileCount(), 1);
    QVERIFY(logic->ElementExists("legacy::temperature"));
    // The reader routes directly to MainWindow. The public facade signal is
    // declared and connected, but is not emitted by LoadExperiment.
    QCOMPARE(facadeLoads.count(), 0);
    QVERIFY(!logic->LoadExperiment(experiment));
    QCOMPARE(logic->GetFormFileCount(), 2);
    // XmlExperimentReader changes the process working directory to the
    // experiment directory and does not restore it.
    QCOMPARE(QDir::cleanPath(QDir::currentPath()), QDir::cleanPath(directory.path()));
    QCOMPARE(QLocale().name(), guard.locale.name());
}

void ProjectIoFacadeContractTests::UIIO_003_invalidAndPartialExperimentLoadsPreserveProcessState()
{
    QTemporaryDir directory;
    ProcessStateGuard guard;
    QVERIFY(directory.isValid());
    const QString form = copyFixture(fixturePath("embedded-form.ui"), directory, "partial.ui");
    QVERIFY(!form.isEmpty());
    const QString partial = writeTextFile(directory, "partial.xml",
        "<Experiment><Tabs><Form Name=\"first\"><AbsPath>" + form +
        "</AbsPath></Form><Form Name=\"missing\"><RelPath>no.ui</RelPath></Form></Tabs>"
        "<Devices/><FigureWindows/><Widgets/><Connections/><State/></Experiment>");
    QVERIFY(!partial.isEmpty());

    MainWindow window;
    UIDataManagementSetClass* logic = window.GetLogic();
    MessageOrderRecorder recorder;
    connect(logic->GetMessenger(), &MessengerClass::InfoWriter, &recorder, &MessageOrderRecorder::info);
    connect(logic->GetMessenger(), &MessengerClass::ErrorWriter, &recorder, &MessageOrderRecorder::error);
    QVERIFY(!logic->LoadExperiment(partial));
    QCOMPARE(logic->GetFormFileCount(), 1);
    QVERIFY(recorder.events.indexOf("info") >= 0);
    QVERIFY(recorder.events.indexOf("error") > recorder.events.indexOf("info"));
    QVERIFY(logic->LoadExperiment(directory.filePath("missing.xml")));
    QVERIFY(logic->LoadExperiment(fixturePath("wrong-root.xml")));
    QCOMPARE(QDir::cleanPath(QDir::currentPath()), QDir::cleanPath(directory.path()));
    QCOMPARE(QLocale().name(), guard.locale.name());
}

void ProjectIoFacadeContractTests::UIIO_004_parameterXmlFacadeRoutingAndConventions()
{
    ProcessStateGuard guard;
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString input = copyFixture(QFINDTESTDATA("../../fixtures/parameters/minimal.xml"), directory, "parameters.xml");
    QVERIFY(!input.isEmpty());

    MainWindow window;
    UIDataManagementSetClass* logic = window.GetLogic();
    addData(*logic, "gain", "Parameter", number(0.0));
    const QString output = directory.filePath("export.xml");
    QVERIFY(!logic->Export2Xml(output, {"gain"}));
    QVERIFY(QFileInfo::exists(output));
    QVERIFY(!logic->ImportFromXml(input));
    QCOMPARE(logic->GetContainer("gain")->GetDouble(), 2.5);
    QVERIFY(logic->Export2Xml(directory.filePath("missing/export.xml"), {"gain"}));
    QVERIFY(logic->ImportFromXml(directory.filePath("missing.xml")));
    QCOMPARE(QDir::currentPath(), guard.workingDirectory);
    QCOMPARE(QLocale().name(), guard.locale.name());
}

void ProjectIoFacadeContractTests::UIIO_005_matAndHdf5FacadeConventions()
{
    ProcessStateGuard guard;
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    MainWindow window;
    UIDataManagementSetClass* logic = window.GetLogic();
    addData(*logic, "value", "Data", number(4.5));

    const QString mat = directory.filePath("value.mat");
    const QString hdf5 = directory.filePath("value.h5");
    QVERIFY(!logic->Export2Mat(mat, {"value"}));
    QVERIFY(QFileInfo::exists(mat));
    QVERIFY(logic->Export2Mat(directory.filePath("missing/value.mat"), {"value"}));
    QVERIFY(!logic->Export2Hdf5(hdf5, {"value"}));
    QVERIFY(QFileInfo::exists(hdf5));

    QSignalSpy errors(logic, &DataManagementClass::Error);
    QVERIFY(!logic->Export2Hdf5(directory.filePath("missing/value.h5"), {"value"}));
    QCOMPARE(errors.count(), 1);
    QVERIFY(errors.at(0).at(0).toString().contains("Export failed"));
    QCOMPARE(QDir::currentPath(), guard.workingDirectory);
    QCOMPARE(QLocale().name(), guard.locale.name());
}

void ProjectIoFacadeContractTests::UIIO_006_pluginFacadeRoutingAndConventions()
{
    ProcessStateGuard guard;
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString compatible = pluginRoot + "/CompatiblePlugin/release/CompatiblePlugin.dll";
    const QString wrongIid = pluginRoot + "/WrongIidPlugin/release/WrongIidPlugin.dll";
    const QString qobjectOnly = pluginRoot + "/QObjectOnlyPlugin/release/QObjectOnlyPlugin.dll";
    QVERIFY(QFileInfo::exists(compatible));
    QVERIFY(QFileInfo::exists(wrongIid));
    QVERIFY(QFileInfo::exists(qobjectOnly));

    MainWindow window;
    UIDataManagementSetClass* logic = window.GetLogic();
    QSignalSpy errors(logic->GetMessenger(), &MessengerClass::ErrorWriter);
    const QString validXml = writeTextFile(directory, "valid.LAdev", deviceXml(compatible, "UIIO_Valid"));
    const QString wrongXml = writeTextFile(directory, "wrong.LAdev", deviceXml(wrongIid, "UIIO_Wrong"));
    const QString plainXml = writeTextFile(directory, "plain.LAdev", deviceXml(qobjectOnly, "UIIO_Plain"));
    QVERIFY(!validXml.isEmpty());
    QVERIFY(!wrongXml.isEmpty());
    QVERIFY(!plainXml.isEmpty());

    QVERIFY(!logic->LoadPlugin(validXml));
    QVERIFY(logic->GetDevice("UIIO_Valid") != nullptr);
    QCOMPARE(errors.count(), 0);
    QVERIFY(logic->LoadPlugin(wrongXml));
    QVERIFY(logic->LoadPlugin(plainXml));
    QVERIFY(logic->LoadPlugin(directory.filePath("missing.LAdev")));
    QCOMPARE(errors.count(), 3);
    QVERIFY(errors.at(0).at(1).toString().contains("incompatible with Platform_Fabric"));
    QVERIFY(errors.at(1).at(1).toString().contains("incompatible with Platform_Fabric"));
    QVERIFY(errors.at(2).at(1).toString().contains("Cannot read file"));
    QCOMPARE(QDir::currentPath(), guard.workingDirectory);
    QCOMPARE(QLocale().name(), guard.locale.name());
}

QTEST_MAIN(ProjectIoFacadeContractTests)

#include "ProjectIoFacadeContractTests.moc"
