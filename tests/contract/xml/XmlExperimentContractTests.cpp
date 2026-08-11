#include <QtTest>
#include <QDomDocument>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "mainwindow.h"
#include "DataManagement/UIDataManagementSetClass.h"
#include "DataManagement/DataMessengerClass.h"
#include "DropWidgets/Plots/PlotWidget.h"
#include "LoadSave/xmlexperimentreader.h"
#include "LoadSave/xmlexperimentwriter.h"

namespace
{
QString fixturePath(const QString& name)
{
    return QFINDTESTDATA("../../fixtures/xml/" + name);
}

QString legacyFixturePath(const QString& name)
{
    return QFINDTESTDATA("../../fixtures/xml/legacy/" + name);
}

QByteArray sha256(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256).toHex().toUpper();
}

void verifyLegacyFixtureHash(const QString& name, const QByteArray& expected)
{
    const QString path = legacyFixturePath(name);
    QVERIFY2(!path.isEmpty(), qPrintable("legacy fixture was not found: " + name));
    QCOMPARE(sha256(path), expected);
}

QString copyLegacyFixture(const QString& name, QTemporaryDir& directory)
{
    const QString path = directory.filePath(name);
    if (!QFile::copy(legacyFixturePath(name), path))
        return QString();
    return path;
}

QStringList errorPayloads(const QSignalSpy& errors)
{
    QStringList payloads;
    for (const QList<QVariant>& event : errors)
        payloads << event.at(1).toString();
    return payloads;
}

bool hasPayloadContaining(const QStringList& payloads, const QString& text)
{
    for (const QString& payload : payloads) {
        if (payload.contains(text))
            return true;
    }
    return false;
}

const QByteArray legacyDcHash("87E538A5B305BDFEF146C8A9261FA99731FFD96D8F9CBEC3488D7FA11F637279");
const QByteArray legacyIbsHash("DDD7FD2F6D084497D97EF5181A2E5183BB95107D8D1204FE588E6B6D31996CF7");
const QByteArray legacyIbs2CellsHash("576CB885EA2FDF4762E4B9E4CE65B412B442CF6642E5320AB0278823F152854A");
const QByteArray legacyDeviceAHash("2DA4A61EE2256C2107F486492E3EF3BE21E97A609D627F191FAD70A8BA4A545B");
const QByteArray legacyDeviceBHash("AB6B7E85972BC98D9D93556CD211BC77790F3E8F107FCAF06C5E32C177903DD3");

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

QString figureWindowXml(const QString& rows, const QString& columns,
                        const QStringList& names, bool addUnknownChild = false)
{
    QString result = QStringLiteral("<Window Rows=\"") + rows
        + QStringLiteral("\" Cols=\"") + columns
        + QStringLiteral("\" PosX=\"17\" PosY=\"29\" Width=\"321\" Height=\"123\">");
    for (int index = 0; index < names.size(); ++index) {
        if (addUnknownChild && index == 1)
            result += QStringLiteral("<UnknownFigureChild/> ");
        result += QStringLiteral("<PlotWidgetName>") + names.at(index)
            + QStringLiteral("</PlotWidgetName>");
    }
    return result + QStringLiteral("</Window>");
}

QString writeFigureExperiment(QTemporaryDir& directory, const QString& figureWindows,
                              const QString& trailingSections = QString())
{
    const QString path = directory.filePath(QStringLiteral("figure-contract.xml"));
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return QString();
    const QString document = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                            "<Experiment><FigureWindows>")
        + figureWindows + QStringLiteral("</FigureWindows>") + trailingSections
        + QStringLiteral("</Experiment>");
    if (file.write(document.toUtf8()) < 0)
        return QString();
    return path;
}

QList<QPointer<SubPlotMainWindow>> closeFigures(MainWindow& window)
{
    QList<QPointer<SubPlotMainWindow>> figures;
    const QList<SubPlotMainWindow*> liveFigures = window.findChildren<SubPlotMainWindow*>();
    for (SubPlotMainWindow* figure : liveFigures) {
        figures << figure;
        figure->close();
    }
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    return figures;
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
    void XML_FIG_001_exactPlotNameCount();
    void XML_FIG_002_fewerPlotNamesKeepGeneratedNames();
    void XML_FIG_003_noPlotNamesKeepGeneratedNames();
    void XML_FIG_004_excessPlotNamesRaiseParserErrorAfterPartialState();
    void XML_FIG_005_emptyAndNonPositiveGridsRemainSafe();
    void XML_FIG_006_unknownFigureChildIsSkipped();
    void XML_FIG_007_laterSectionsStopAfterExcessName();
    void XML_LEGACY_001_smallestRealFixtureWithMissingDependencies();
    void XML_LEGACY_002_realFixtureWithMissingDependencies();
    void XML_LEGACY_003_largestRealFixtureWithMissingDependencies();
    void XML_LEGACY_004_smallestRealFixtureReadWriteRead();
    void XML_LEGACY_005_temporaryCompatiblePluginReplacement();
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

void XmlExperimentContractTests::XML_FIG_001_exactPlotNameCount()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = writeFigureExperiment(directory,
        figureWindowXml("1", "2", {"Plot#701", "Plot#702"}));
    QVERIFY(!path.isEmpty());

    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(!reader.read(path));
    const QList<SubPlotMainWindow*> figures = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(figures.size(), 1);
    const QList<PlotWidget*> plots = figures.constFirst()->findChildren<PlotWidget*>();
    QCOMPARE(plots.size(), 2);
    QCOMPARE(plots.at(0)->objectName(), QString("Plot#701"));
    QCOMPARE(plots.at(1)->objectName(), QString("Plot#702"));
    QCOMPARE(window.GetLogic()->GetPlotByName("Plot#701"), static_cast<QObject*>(plots.at(0)));
    QCOMPARE(window.GetLogic()->GetPlotByName("Plot#702"), static_cast<QObject*>(plots.at(1)));
    QCOMPARE(window.GetLogic()->GetPlotWindowRowsCols(figures.constFirst()->objectName()).first, 1);
    QCOMPARE(window.GetLogic()->GetPlotWindowRowsCols(figures.constFirst()->objectName()).second, 2);
    const auto closed = closeFigures(window);
    for (const QPointer<SubPlotMainWindow>& figure : closed)
        QVERIFY(figure.isNull());
}

void XmlExperimentContractTests::XML_FIG_002_fewerPlotNamesKeepGeneratedNames()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = writeFigureExperiment(directory,
        figureWindowXml("1", "2", {"Plot#711"}));
    QVERIFY(!path.isEmpty());

    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(!reader.read(path));
    const QList<SubPlotMainWindow*> figures = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(figures.size(), 1);
    const QList<PlotWidget*> plots = figures.constFirst()->findChildren<PlotWidget*>();
    QCOMPARE(plots.size(), 2);
    QCOMPARE(plots.at(0)->objectName(), QString("Plot#711"));
    QVERIFY(!plots.at(1)->objectName().isEmpty());
    QVERIFY(plots.at(1)->objectName() != QString("Plot#711"));
    QCOMPARE(window.GetLogic()->GetPlotByName(plots.at(1)->objectName()), static_cast<QObject*>(plots.at(1)));
    const auto closed = closeFigures(window);
    for (const QPointer<SubPlotMainWindow>& figure : closed)
        QVERIFY(figure.isNull());
}

void XmlExperimentContractTests::XML_FIG_003_noPlotNamesKeepGeneratedNames()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = writeFigureExperiment(directory, figureWindowXml("1", "2", {}));
    QVERIFY(!path.isEmpty());

    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(!reader.read(path));
    const QList<SubPlotMainWindow*> figures = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(figures.size(), 1);
    const QList<PlotWidget*> plots = figures.constFirst()->findChildren<PlotWidget*>();
    QCOMPARE(plots.size(), 2);
    QVERIFY(!plots.at(0)->objectName().isEmpty());
    QVERIFY(!plots.at(1)->objectName().isEmpty());
    QVERIFY(plots.at(0)->objectName() != plots.at(1)->objectName());
    const auto closed = closeFigures(window);
    for (const QPointer<SubPlotMainWindow>& figure : closed)
        QVERIFY(figure.isNull());
}

void XmlExperimentContractTests::XML_FIG_004_excessPlotNamesRaiseParserErrorAfterPartialState()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = writeFigureExperiment(directory,
        figureWindowXml("1", "1", {"Plot#721", "Plot#722"}));
    QVERIFY(!path.isEmpty());

    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(reader.read(path));
    // errorString() is historically empty; read() is the observable parser-error boundary.
    QCOMPARE(reader.errorString(), QString());
    const QList<SubPlotMainWindow*> figures = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(figures.size(), 1);
    const QList<PlotWidget*> plots = figures.constFirst()->findChildren<PlotWidget*>();
    QCOMPARE(plots.size(), 1);
    QCOMPARE(plots.constFirst()->objectName(), QString("Plot#721"));
    QCOMPARE(window.GetLogic()->GetPlotByName("Plot#721"), static_cast<QObject*>(plots.constFirst()));
    const auto closed = closeFigures(window);
    for (const QPointer<SubPlotMainWindow>& figure : closed)
        QVERIFY(figure.isNull());
}

void XmlExperimentContractTests::XML_FIG_005_emptyAndNonPositiveGridsRemainSafe()
{
    const QList<QPair<QString, QString>> grids = {
        {"0", "0"}, {"0", "2"}, {"-1", "2"}, {"not-a-number", "also-not-a-number"}
    };
    for (const QPair<QString, QString>& grid : grids) {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = writeFigureExperiment(directory, figureWindowXml(grid.first, grid.second, {}));
        QVERIFY(!path.isEmpty());
        MainWindow window;
        XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
        QVERIFY(!reader.read(path));
        const QList<SubPlotMainWindow*> figures = window.findChildren<SubPlotMainWindow*>();
        QCOMPARE(figures.size(), 1);
        QCOMPARE(figures.constFirst()->findChildren<PlotWidget*>().size(), 0);
        const auto closed = closeFigures(window);
        for (const QPointer<SubPlotMainWindow>& figure : closed)
            QVERIFY(figure.isNull());
    }
}

void XmlExperimentContractTests::XML_FIG_006_unknownFigureChildIsSkipped()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = writeFigureExperiment(directory,
        figureWindowXml("1", "2", {"Plot#731", "Plot#732"}, true));
    QVERIFY(!path.isEmpty());

    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(!reader.read(path));
    const QList<SubPlotMainWindow*> figures = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(figures.size(), 1);
    const QList<PlotWidget*> plots = figures.constFirst()->findChildren<PlotWidget*>();
    QCOMPARE(plots.size(), 2);
    QCOMPARE(plots.at(0)->objectName(), QString("Plot#731"));
    QCOMPARE(plots.at(1)->objectName(), QString("Plot#732"));
    const auto closed = closeFigures(window);
    for (const QPointer<SubPlotMainWindow>& figure : closed)
        QVERIFY(figure.isNull());
}

void XmlExperimentContractTests::XML_FIG_007_laterSectionsStopAfterExcessName()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString uiPath = copyFixture(fixturePath("embedded-form.ui"), directory, "after-figure.ui");
    QVERIFY(!uiPath.isEmpty());
    const QString figures = figureWindowXml("1", "1", {"Plot#741"})
        + figureWindowXml("1", "1", {"Plot#742", "Plot#743"});
    const QString tabs = QStringLiteral("<Tabs><Form Name=\"after-figure\"><AbsPath>")
        + uiPath + QStringLiteral("</AbsPath></Form></Tabs>");
    const QString path = writeFigureExperiment(directory, figures, tabs);
    QVERIFY(!path.isEmpty());

    MainWindow window;
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QSignalSpy formLoads(&reader, &XmlExperimentReader::LoadFormFromXML);
    QVERIFY(formLoads.isValid());
    QVERIFY(reader.read(path));
    QCOMPARE(formLoads.count(), 0);
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    const QList<SubPlotMainWindow*> windows = window.findChildren<SubPlotMainWindow*>();
    QCOMPARE(windows.size(), 2);
    QCOMPARE(windows.at(0)->findChildren<PlotWidget*>().constFirst()->objectName(), QString("Plot#741"));
    QCOMPARE(windows.at(1)->findChildren<PlotWidget*>().constFirst()->objectName(), QString("Plot#742"));
    const auto closed = closeFigures(window);
    for (const QPointer<SubPlotMainWindow>& figure : closed)
        QVERIFY(figure.isNull());
}

void XmlExperimentContractTests::XML_LEGACY_001_smallestRealFixtureWithMissingDependencies()
{
    verifyLegacyFixtureHash("legacy-small.LAexp", legacyDcHash);
    verifyLegacyFixtureHash("Device_legacy-a.LAdev", legacyDeviceAHash);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString experiment = copyLegacyFixture("legacy-small.LAexp", directory);
    QVERIFY(!experiment.isEmpty());
    QVERIFY(!copyLegacyFixture("Device_legacy-a.LAdev", directory).isEmpty());

    MainWindow window;
    QSignalSpy errors(window.GetLogic()->GetMessenger(), &MessengerClass::ErrorWriter);
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(reader.read(experiment));

    const QStringList payloads = errorPayloads(errors);
    QVERIFY(hasPayloadContaining(payloads, "SpannugsquellenUI.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "debug.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "HistoricalPluginMissing.dll couldn't be loaded!"));
    QCOMPARE(errors.count(), 3);
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(window.GetLogic()->GetDevice("LegacyDeviceA"), nullptr);
    QCOMPARE(window.GetLogic()->GetContainerCount(), 0);
    verifyLegacyFixtureHash("legacy-small.LAexp", legacyDcHash);
    verifyLegacyFixtureHash("Device_legacy-a.LAdev", legacyDeviceAHash);
}

void XmlExperimentContractTests::XML_LEGACY_002_realFixtureWithMissingDependencies()
{
    verifyLegacyFixtureHash("legacy-multiform.LAexp", legacyIbsHash);
    verifyLegacyFixtureHash("Device_legacy-b.LAdev", legacyDeviceBHash);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString experimentDirectory = directory.filePath("experiments/current");
    QVERIFY(QDir().mkpath(experimentDirectory));
    const QString experiment = QDir(experimentDirectory).filePath("legacy-multiform.LAexp");
    QVERIFY(QFile::copy(legacyFixturePath("legacy-multiform.LAexp"), experiment));
    QVERIFY(!experiment.isEmpty());
    const QString deviceDirectory = directory.filePath("LegacyProjects/NeutralRoutine/LegacyExperiment");
    QVERIFY(QDir().mkpath(deviceDirectory));
    QVERIFY(QFile::copy(legacyFixturePath("Device_legacy-b.LAdev"), QDir(deviceDirectory).filePath("Device_legacy-b.LAdev")));

    MainWindow window;
    QSignalSpy errors(window.GetLogic()->GetMessenger(), &MessengerClass::ErrorWriter);
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(reader.read(experiment));

    const QStringList payloads = errorPayloads(errors);
    QVERIFY(hasPayloadContaining(payloads, "Mainboard.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "Sideboard_Error.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "Overview.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "debug.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "SpannugsquellenUI.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "HistoricalPluginMissing.dll couldn't be loaded!"));
    QCOMPARE(errors.count(), 6);
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(window.GetLogic()->GetDevice("LegacyDeviceB"), nullptr);
    QCOMPARE(window.GetLogic()->GetContainerCount(), 0);
    verifyLegacyFixtureHash("legacy-multiform.LAexp", legacyIbsHash);
    verifyLegacyFixtureHash("Device_legacy-b.LAdev", legacyDeviceBHash);
}

void XmlExperimentContractTests::XML_LEGACY_003_largestRealFixtureWithMissingDependencies()
{
    verifyLegacyFixtureHash("legacy-2cells.LAexp", legacyIbs2CellsHash);
    verifyLegacyFixtureHash("Device_legacy-a.LAdev", legacyDeviceAHash);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString experiment = copyLegacyFixture("legacy-2cells.LAexp", directory);
    QVERIFY(!experiment.isEmpty());
    QVERIFY(!copyLegacyFixture("Device_legacy-a.LAdev", directory).isEmpty());

    MainWindow window;
    QSignalSpy errors(window.GetLogic()->GetMessenger(), &MessengerClass::ErrorWriter);
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(reader.read(experiment));

    const QStringList payloads = errorPayloads(errors);
    QVERIFY(hasPayloadContaining(payloads, "Mainboard.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "Sideboard_Error.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "Overview.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "debug.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "SpannugsquellenUI.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "Sideboard_Error2.ui not found!"));
    QVERIFY(hasPayloadContaining(payloads, "HistoricalPluginMissing.dll couldn't be loaded!"));
    QCOMPARE(errors.count(), 7);
    QCOMPARE(window.GetLogic()->GetFormFileCount(), 0);
    QCOMPARE(window.GetLogic()->GetDevice("LegacyDeviceA"), nullptr);
    QCOMPARE(window.GetLogic()->GetContainerCount(), 0);
    verifyLegacyFixtureHash("legacy-2cells.LAexp", legacyIbs2CellsHash);
    verifyLegacyFixtureHash("Device_legacy-a.LAdev", legacyDeviceAHash);
}

void XmlExperimentContractTests::XML_LEGACY_004_smallestRealFixtureReadWriteRead()
{
    verifyLegacyFixtureHash("legacy-small.LAexp", legacyDcHash);
    verifyLegacyFixtureHash("Device_legacy-a.LAdev", legacyDeviceAHash);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = copyLegacyFixture("legacy-small.LAexp", directory);
    QVERIFY(!source.isEmpty());
    QVERIFY(!copyLegacyFixture("Device_legacy-a.LAdev", directory).isEmpty());

    MainWindow sourceWindow;
    XmlExperimentReader sourceReader(sourceWindow.GetLogic(), sourceWindow.GetLogic()->GetMessenger(), sourceWindow.GetLogic());
    const bool sourceResult = sourceReader.read(source);
    QVERIFY(sourceResult);
    const int sourceContainers = sourceWindow.GetLogic()->GetContainerCount();
    const QString written = directory.filePath("legacy-roundtrip.xml");
    xmlexperimentwriter writer(sourceWindow.GetLogic(), sourceWindow.GetLogic()->GetMessengerRef(), *sourceWindow.GetLogic());
    QVERIFY(!writer.write(written));

    const QDomDocument writtenDocument = readDocument(written);
    QVERIFY(!writtenDocument.isNull());
    QCOMPARE(directElementNames(writtenDocument.documentElement()), QStringList({"Tabs", "Devices", "FigureWindows", "Widgets", "Connections", "State"}));

    MainWindow roundTripWindow;
    XmlExperimentReader roundTripReader(roundTripWindow.GetLogic(), roundTripWindow.GetLogic()->GetMessenger(), roundTripWindow.GetLogic());
    QVERIFY(!roundTripReader.read(written));
    QCOMPARE(roundTripWindow.GetLogic()->GetFormFileCount(), sourceWindow.GetLogic()->GetFormFileCount());
    QCOMPARE(roundTripWindow.GetLogic()->GetContainerCount(), sourceContainers);
    verifyLegacyFixtureHash("legacy-small.LAexp", legacyDcHash);
    verifyLegacyFixtureHash("Device_legacy-a.LAdev", legacyDeviceAHash);
}

void XmlExperimentContractTests::XML_LEGACY_005_temporaryCompatiblePluginReplacement()
{
    verifyLegacyFixtureHash("legacy-small.LAexp", legacyDcHash);
    verifyLegacyFixtureHash("Device_legacy-a.LAdev", legacyDeviceAHash);
    const QString pluginRoot = qEnvironmentVariable("LABANALYSER_TEST_PLUGIN_ROOT");
    QVERIFY2(!pluginRoot.isEmpty(), "LABANALYSER_TEST_PLUGIN_ROOT must identify the runtime-built plugin fixtures.");
    const QString plugin = QDir(pluginRoot).filePath("CompatiblePlugin/release/CompatiblePlugin.dll");
    QVERIFY2(QFileInfo::exists(plugin), qPrintable("CompatiblePlugin fixture is missing: " + plugin));

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString experiment = copyLegacyFixture("legacy-small.LAexp", directory);
    const QString device = copyLegacyFixture("Device_legacy-a.LAdev", directory);
    QVERIFY(!experiment.isEmpty());
    QVERIFY(!device.isEmpty());
    QCOMPARE(sha256(experiment), legacyDcHash);

    QFile deviceFile(device);
    QVERIFY(deviceFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString deviceContents = QString::fromUtf8(deviceFile.readAll());
    deviceFile.close();
    QVERIFY(deviceContents.contains("HistoricalPluginMissing.dll"));
    deviceContents.replace("HistoricalPluginMissing.dll", QDir::toNativeSeparators(plugin));
    QVERIFY(deviceFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    deviceFile.write(deviceContents.toUtf8());
    deviceFile.close();

    MainWindow window;
    QSignalSpy errors(window.GetLogic()->GetMessenger(), &MessengerClass::ErrorWriter);
    XmlExperimentReader reader(window.GetLogic(), window.GetLogic()->GetMessenger(), window.GetLogic());
    QVERIFY(!reader.read(experiment));
    QCOMPARE(errors.count(), 2);
    QVERIFY(!hasPayloadContaining(errorPayloads(errors), "HistoricalPluginMissing.dll couldn't be loaded!"));
    QVERIFY(window.GetLogic()->GetDevice("LegacyDeviceA") != nullptr);
    QCOMPARE(window.GetLogic()->GetDevicePaths(), QStringList({device}));
    verifyLegacyFixtureHash("legacy-small.LAexp", legacyDcHash);
    verifyLegacyFixtureHash("Device_legacy-a.LAdev", legacyDeviceAHash);
}

QTEST_MAIN(XmlExperimentContractTests)

#include "XmlExperimentContractTests.moc"
