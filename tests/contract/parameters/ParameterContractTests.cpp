#include <QtTest>
#include <QDomDocument>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "mainwindow.h"
#include "DataManagement/UIDataManagementSetClass.h"
#include "DataManagement/DataManagementSetClass.h"
#include "DataManagement/DataMessengerClass.h"
#include "Import/parameterloader.h"
#include "Export/exportinputs2xml.h"

Q_DECLARE_METATYPE(InterfaceData)

namespace
{
QString fixturePath(const QString& name)
{
    return QFINDTESTDATA("../../fixtures/parameters/" + name);
}

QString copyFixture(const QString& source, QTemporaryDir& directory, const QString& target)
{
    const QString path = directory.filePath(target);
    return QFile::copy(source, path) ? path : QString();
}

QDomDocument readDocument(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QDomDocument();
    QDomDocument document;
    document.setContent(&file);
    return document;
}

void addParameter(DataManagementSetClass& manager, const QString& id, InterfaceData value,
                  const QString& alias = QString(), double minimum = -100.0, double maximum = 100.0)
{
    manager.AddContainerElement(id, value.GetDataType(), "Parameter", "state-meta");
    static_cast<DataManagementClass&>(manager).SetData(id, value);
    manager.SetAlias(id, alias);
    manager.SetMinMaxValue(id, minimum, maximum);
}

InterfaceData integer(qint32 value) { InterfaceData data; data.SetData(static_cast<int32_t>(value)); return data; }
InterfaceData floating(double value) { InterfaceData data; data.SetData(value); return data; }
InterfaceData boolean(bool value) { InterfaceData data; data.SetData(value); return data; }
InterfaceData text(const QString& value) { InterfaceData data; data.SetData(value); return data; }
}

class ParameterContractTests final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void PARAM_001_minimalKnownParameterLoadsAndSignals();
    void PARAM_002_multipleTypesUnicodeAndMetadata();
    void PARAM_003_optionalUnknownAndDuplicateEntries();
    void PARAM_004_invalidNumbersRangesAndTypeConversions();
    void PARAM_005_missingEmptyTruncatedAndInvalidFiles();
    void PARAM_006_repeatedLoadsRetainUnmentionedState();
    void PARAM_007_writerOrderDuplicatesEscapingAndPathErrors();
    void PARAM_008_exportImportSemanticRoundTrip();
    void PARAM_009_uiCallersPreserveErrorConvention();
};

void ParameterContractTests::initTestCase()
{
    qRegisterMetaType<InterfaceData>("InterfaceData");
}

void ParameterContractTests::PARAM_001_minimalKnownParameterLoadsAndSignals()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    addParameter(manager, "gain", floating(1.0));
    QSignalSpy set(manager.GetMessenger(), &MessengerClass::SetData);
    QSignalSpy sent(manager.GetMessenger(), &MessengerClass::MessageSender);
    ParameterLoader loader(&manager);

    QVERIFY(!loader.Load(fixturePath("minimal.xml")));
    QCOMPARE(manager.GetContainer("gain")->GetDouble(), 2.5);
    QCOMPARE(set.count(), 1);
    QCOMPARE(sent.count(), 1);
    QCOMPARE(set.at(0).at(0).toString(), QString("gain"));
    QCOMPARE(qvariant_cast<InterfaceData>(set.at(0).at(1)).GetDouble(), 2.5);
    QCOMPARE(sent.at(0).at(0).toString(), QString("set"));
}

void ParameterContractTests::PARAM_002_multipleTypesUnicodeAndMetadata()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    addParameter(manager, "count", integer(0), QString::fromUtf8("Zähler ä"), 1.0, 9.0);
    addParameter(manager, "enabled", boolean(false), "Enabled", 0.0, 1.0);
    addParameter(manager, "label", text("before"), QString::fromUtf8("Beschriftung Ω"));
    ParameterLoader loader(&manager);

    QVERIFY(!loader.Load(fixturePath("multiple-unicode.xml")));
    QCOMPARE(manager.GetContainer("count")->GetInt32_tData(), 42);
    QVERIFY(manager.GetContainer("enabled")->GetBool());
    QCOMPARE(manager.GetContainer("label")->GetString(), QString::fromUtf8("Größe Ω & <Test>"));
    QCOMPARE(manager.GetAlias("count"), QString::fromUtf8("Zähler ä"));
    QCOMPARE(manager.MinMaxValue("count"), std::make_pair(1.0, 9.0));
    QCOMPARE(manager.GetContainer("count")->GetStateDependency(), QString("state-meta"));
}

void ParameterContractTests::PARAM_003_optionalUnknownAndDuplicateEntries()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    addParameter(manager, "repeat", text("seed"));
    addParameter(manager, "empty", text("seed"));
    QSignalSpy info(manager.GetMessenger(), &MessengerClass::InfoWriter);
    ParameterLoader loader(&manager);

    QVERIFY(!loader.Load(fixturePath("optional-unknown-duplicate.xml")));
    QCOMPARE(manager.GetContainer("repeat")->GetString(), QString("second"));
    QCOMPARE(manager.GetContainer("empty")->GetString(), QString());
    QCOMPARE(info.count(), 1);
    QVERIFY(info.at(0).at(1).toString().contains("missing"));
}

void ParameterContractTests::PARAM_004_invalidNumbersRangesAndTypeConversions()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    addParameter(manager, "double", floating(3.0), QString(), 0.0, 1.0);
    InterfaceData unsignedByte;
    unsignedByte.SetData(static_cast<uint8_t>(7));
    addParameter(manager, "u8", unsignedByte);
    addParameter(manager, "flag", boolean(false));
    ParameterLoader loader(&manager);

    QVERIFY(!loader.Load(fixturePath("invalid-numbers.xml")));
    QCOMPARE(manager.GetContainer("double")->GetDouble(), 0.0);
    QCOMPARE(manager.GetContainer("u8")->GetUInt8_tData(), static_cast<uint8_t>(63));
    QVERIFY(manager.GetContainer("flag")->GetBool());
    QCOMPARE(manager.MinMaxValue("double"), std::make_pair(0.0, 1.0));
}

void ParameterContractTests::PARAM_005_missingEmptyTruncatedAndInvalidFiles()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    QSignalSpy errors(manager.GetMessenger(), &MessengerClass::ErrorWriter);
    ParameterLoader loader(&manager);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString empty = directory.filePath("empty.xml");
    QFile emptyFile(empty);
    QVERIFY(emptyFile.open(QIODevice::WriteOnly));
    emptyFile.close();

    QVERIFY(loader.Load(directory.filePath("missing.xml")));
    QVERIFY(loader.Load(empty));
    QVERIFY(loader.Load(fixturePath("wrong-root.xml")));
    QVERIFY(loader.Load(fixturePath("truncated.xml")));
    QVERIFY(loader.Load(fixturePath("invalid-syntax.xml")));
    QVERIFY(errors.count() >= 4);
}

void ParameterContractTests::PARAM_006_repeatedLoadsRetainUnmentionedState()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    addParameter(manager, "gain", floating(0.0));
    addParameter(manager, "other", text("initial"));
    ParameterLoader loader(&manager);

    QVERIFY(!loader.Load(fixturePath("minimal.xml")));
    QCOMPARE(manager.GetContainer("gain")->GetDouble(), 2.5);
    QVERIFY(!loader.Load(fixturePath("minimal.xml")));
    QCOMPARE(manager.GetContainer("gain")->GetDouble(), 2.5);
    QCOMPARE(manager.GetContainer("other")->GetString(), QString("initial"));
}

void ParameterContractTests::PARAM_007_writerOrderDuplicatesEscapingAndPathErrors()
{
    QObject owner;
    owner.setObjectName("LabAnalyser");
    DataManagementSetClass manager(&owner);
    addParameter(manager, "first", text(QString::fromUtf8("A & <Ω>")));
    addParameter(manager, "second", integer(7));
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString output = directory.filePath("parameters.xml");
    ExportInputs2Xml exporter(manager);

    QVERIFY(!exporter.Export2XML(output, {"second", "missing", "first", "second"}));
    const QDomDocument document = readDocument(output);
    QVERIFY(!document.isNull());
    QCOMPARE(document.documentElement().tagName(), QString("ParameterSet"));
    QDomElement entry = document.documentElement().firstChildElement("entry");
    QStringList names;
    QStringList values;
    while (!entry.isNull()) {
        names << entry.firstChildElement("Parameter").text();
        values << entry.firstChildElement("Value").text();
        entry = entry.nextSiblingElement("entry");
    }
    QCOMPARE(names, QStringList({"second", "first", "second"}));
    QCOMPARE(values, QStringList({"7", QString::fromUtf8("A & <Ω>"), "7"}));
    QVERIFY(exporter.Export2XML(directory.filePath("missing-parent/out.xml"), {"first"}));
}

void ParameterContractTests::PARAM_008_exportImportSemanticRoundTrip()
{
    QObject sourceOwner;
    sourceOwner.setObjectName("LabAnalyser");
    DataManagementSetClass source(&sourceOwner);
    addParameter(source, "number", floating(12.75));
    addParameter(source, "enabled", boolean(true));
    addParameter(source, "text", text(QString::fromUtf8("Wert Ω")));
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString output = directory.filePath("roundtrip.xml");
    ExportInputs2Xml exporter(source);
    QVERIFY(!exporter.Export2XML(output, {"number", "enabled", "text"}));

    QObject targetOwner;
    targetOwner.setObjectName("LabAnalyser");
    DataManagementSetClass target(&targetOwner);
    addParameter(target, "number", floating(0.0), "target alias", -1.0, 1.0);
    addParameter(target, "enabled", boolean(false));
    addParameter(target, "text", text(QString()));
    ParameterLoader loader(&target);
    QVERIFY(!loader.Load(output));
    QCOMPARE(target.GetContainer("number")->GetDouble(), 12.75);
    QVERIFY(target.GetContainer("enabled")->GetBool());
    QCOMPARE(target.GetContainer("text")->GetString(), QString::fromUtf8("Wert Ω"));
    QCOMPARE(target.GetAlias("number"), QString("target alias"));
    QCOMPARE(target.MinMaxValue("number"), std::make_pair(-1.0, 1.0));
}

void ParameterContractTests::PARAM_009_uiCallersPreserveErrorConvention()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString input = copyFixture(fixturePath("minimal.xml"), directory, "minimal.xml");
    QVERIFY(!input.isEmpty());
    MainWindow window;
    addParameter(*window.GetLogic(), "gain", floating(0.0));
    const QString output = directory.filePath("ui-export.xml");

    QVERIFY(!window.GetLogic()->Export2Xml(output, {"gain"}));
    QVERIFY(QFileInfo::exists(output));
    QVERIFY(!window.GetLogic()->ImportFromXml(input));
    QCOMPARE(window.GetLogic()->GetContainer("gain")->GetDouble(), 2.5);
    QVERIFY(window.GetLogic()->Export2Xml(directory.filePath("missing/out.xml"), {"gain"}));
    QVERIFY(window.GetLogic()->ImportFromXml(directory.filePath("missing.xml")));
}

QTEST_MAIN(ParameterContractTests)

#include "ParameterContractTests.moc"
