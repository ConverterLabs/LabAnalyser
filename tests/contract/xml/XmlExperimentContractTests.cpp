#include <QtTest>
#include <QDomDocument>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "mainwindow.h"
#include "DataManagement/UIDataManagementSetClass.h"
#include "DataManagement/DataMessengerClass.h"
#include "LoadSave/xmlexperimentreader.h"
#include "LoadSave/xmlexperimentwriter.h"

namespace
{
QString fixturePath(const QString& name)
{
    return QFINDTESTDATA("../../fixtures/xml/" + name);
}

QString copyFixture(const QString& source, QTemporaryDir& directory, const QString& target)
{
    const QString path = directory.filePath(target);
    if (!QFile::copy(source, path))
        return QString();
    return path;
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

QStringList directElementNames(const QDomElement& parent)
{
    QStringList names;
    for (QDomElement element = parent.firstChildElement(); !element.isNull(); element = element.nextSiblingElement())
        names << element.tagName();
    return names;
}
}

class XmlExperimentContractTests final : public QObject
{
    Q_OBJECT

private slots:
    void XML_001_minimalExperimentAndUnknownContent();
    void XML_002_fullLegacyFixtureLoadsFormAndDataReference();
    void XML_003_invalidTruncatedAndMissingFilesReportErrors();
    void XML_004_optionalElementsAndMissingReferences();
    void XML_005_writerEmitsOrderedUnicodeExperiment();
    void XML_006_readerWriterSemanticRoundTrip();
    void XML_007_uiDataManagementSaveLoadConventions();
    void XML_008_figureWindowStateIsPersisted();
};

void XmlExperimentContractTests::XML_001_minimalExperimentAndUnknownContent()
{
    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());

    QVERIFY(!reader.read(fixturePath("minimal-with-unknown.xml")));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(window.GetLogic()->GetContainerCount(), 0);
}

void XmlExperimentContractTests::XML_002_fullLegacyFixtureLoadsFormAndDataReference()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(!copyFixture(fixturePath("embedded-form.ui"), directory, "embedded-form.ui").isEmpty());
    const QString experiment = copyFixture(fixturePath("legacy-complete.xml"), directory, "legacy-complete.xml");
    QVERIFY(!experiment.isEmpty());

    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QSignalSpy formLoads(&reader, &XmlExperimentReader::LoadFormFromXML);
    QVERIFY(formLoads.isValid());

    const QString previousWorkingDirectory = QDir::currentPath();
    QVERIFY(!reader.read(experiment));
    QVERIFY(QDir::setCurrent(previousWorkingDirectory));
    QCOMPARE(formLoads.count(), 1);
    QCOMPARE(formLoads.at(0).at(1).toString(), QString::fromUtf8("Form Ω"));
    QCOMPARE(formLoads.at(0).at(2).toBool(), true);
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 1);
    QVERIFY(window.GetLogic()->ElementExists("legacy::temperature"));
    QCOMPARE(window.GetLogic()->MinMaxValue("legacy::temperature"), std::make_pair(1.5, 9.5));
    QCOMPARE(window.GetLogic()->GetAlias("legacy::temperature"), QString::fromUtf8("Temperatur ä"));
}

void XmlExperimentContractTests::XML_003_invalidTruncatedAndMissingFilesReportErrors()
{
    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());

    QVERIFY(reader.read(fixturePath("wrong-root.xml")));
    QVERIFY(reader.read(fixturePath("truncated.xml")));
    QVERIFY(reader.read(QDir(QDir::tempPath()).filePath("lab-analyser-no-such-experiment.xml")));
    QCOMPARE(reader.errorString(), QString());
}

void XmlExperimentContractTests::XML_004_optionalElementsAndMissingReferences()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString experiment = copyFixture(fixturePath("optional-and-missing.xml"), directory, "optional-and-missing.xml");
    QVERIFY(!experiment.isEmpty());

    MainWindow window;
    QSignalSpy errors(window.GetLogic()->GetMessenger(), &MessengerClass::ErrorWriter);
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());

    QVERIFY(!reader.read(experiment));
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(errors.count(), 1);
    QCOMPARE(errors.at(0).at(0).toString(), QString());
    QVERIFY(errors.at(0).at(1).toString().contains("missing-form.ui"));
}

void XmlExperimentContractTests::XML_005_writerEmitsOrderedUnicodeExperiment()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString form = copyFixture(fixturePath("embedded-form.ui"), directory, "form-ä.ui");
    QVERIFY(!form.isEmpty());

    MainWindow window;
    window.GetLogic()->AddFormFile({QString::fromUtf8("Form Ω"), form});
    const QString output = directory.filePath("written.xml");
    xmlexperimentwriter writer(window.GetLogic(), window.GetLogic()->GetMessengerRef(), *window.GetLogic());

    QVERIFY(!writer.write(output));
    const QDomDocument document = readDocument(output);
    QVERIFY(!document.isNull());
    const QDomElement root = document.documentElement();
    QCOMPARE(root.tagName(), QString("Experiment"));
    QCOMPARE(directElementNames(root), QStringList({"Tabs", "Devices", "FigureWindows", "Widgets", "Connections", "State"}));
    const QDomElement formElement = root.firstChildElement("Tabs").firstChildElement("Form");
    QCOMPARE(formElement.attribute("Name"), QString::fromUtf8("Form Ω"));
    QCOMPARE(formElement.firstChildElement("AbsPath").text(), form);
    QVERIFY(!formElement.firstChildElement("RelPath").text().isEmpty());
}

void XmlExperimentContractTests::XML_006_readerWriterSemanticRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString form = copyFixture(fixturePath("embedded-form.ui"), directory, "embedded-form.ui");
    QVERIFY(!form.isEmpty());
    const QString source = copyFixture(fixturePath("legacy-complete.xml"), directory, "legacy-complete.xml");
    QVERIFY(!source.isEmpty());

    MainWindow sourceWindow;
    QObject persistedReference(&sourceWindow);
    persistedReference.setObjectName("missing-widget");
    const QString previousWorkingDirectory = QDir::currentPath();
    QVERIFY(!sourceWindow.GetLogic()->LoadExperiment(source));
    QVERIFY(QDir::setCurrent(previousWorkingDirectory));
    const QString written = directory.filePath("round-trip.xml");
    QVERIFY(!sourceWindow.GetLogic()->SaveExperiment(written));

    const QDomDocument document = readDocument(written);
    QVERIFY(!document.isNull());
    QCOMPARE(document.documentElement().tagName(), QString("Experiment"));
    QCOMPARE(document.documentElement().firstChildElement("Tabs").firstChildElement("Form").attribute("Name"), QString::fromUtf8("Form Ω"));
    QVERIFY(document.documentElement().firstChildElement("Connections").firstChildElement("connect").firstChildElement("ID").text().contains("legacy::temperature"));
}

void XmlExperimentContractTests::XML_007_uiDataManagementSaveLoadConventions()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString valid = copyFixture(fixturePath("minimal-with-unknown.xml"), directory, "minimal.xml");
    QVERIFY(!valid.isEmpty());

    MainWindow window;
    const QString output = directory.filePath("ui-save.xml");
    QVERIFY(!window.GetLogic()->SaveExperiment(output));
    QVERIFY(QFileInfo::exists(output));
    QVERIFY(!window.GetLogic()->LoadExperiment(valid));
    QVERIFY(window.GetLogic()->LoadExperiment(directory.filePath("missing.xml")));
}

void XmlExperimentContractTests::XML_008_figureWindowStateIsPersisted()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    MainWindow window;
    SubPlotMainWindow* figure = window.CreateSubPlotWindow(1, 2);
    QVERIFY(figure != nullptr);
    figure->resize(321, 123);
    figure->move(17, 29);
    const QString output = directory.filePath("figure.xml");
    QVERIFY(!window.GetLogic()->SaveExperiment(output));

    const QDomElement figureElement = readDocument(output).documentElement().firstChildElement("FigureWindows").firstChildElement("Window");
    QVERIFY(!figureElement.isNull());
    QCOMPARE(figureElement.attribute("Rows"), QString("1"));
    QCOMPARE(figureElement.attribute("Cols"), QString("2"));
    QCOMPARE(figureElement.attribute("Width"), QString("321"));
    QCOMPARE(figureElement.attribute("Height"), QString("123"));
}

QTEST_MAIN(XmlExperimentContractTests)

#include "XmlExperimentContractTests.moc"
