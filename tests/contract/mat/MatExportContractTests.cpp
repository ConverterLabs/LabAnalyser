#include <QtTest>
#include <QFileInfo>
#include <QTemporaryDir>
#include <cmath>
#include <limits>

#include <matio.h>
#include "Export/Export2Mat.h"
#include "Import/MatDataImport.h"
#include "DataManagement/DataManagementSetClass.h"
#include "DataManagement/UIDataManagementSetClass.h"
#include "mainwindow.h"

namespace {
InterfaceData numeric(double value) { InterfaceData d; d.SetData(value); return d; }
template <typename T> InterfaceData numericT(T value) { InterfaceData d; d.SetData(value); return d; }
InterfaceData stringValue(const QString& value) { InterfaceData d; d.SetData(value); return d; }
InterfaceData vectorValue(const std::vector<double>& time, const std::vector<double>& data) {
    InterfaceData value;
    value.SetData(DataPair(boost::shared_ptr<std::vector<double>>(new std::vector<double>(time)),
                           boost::shared_ptr<std::vector<double>>(new std::vector<double>(data))));
    return value;
}
void add(DataManagementClass& manager, const QString& id, InterfaceData value) {
    manager.AddContainerElement(id, value.GetDataType(), "Data", QString());
    manager.SetData(id, value);
}
matvar_t* field(matvar_t* channels, const char* name, size_t index) {
    return Mat_VarGetStructFieldByName(channels, name, index);
}
QString text(matvar_t* variable) {
    if (!variable || !variable->data) return QString();
    const size_t characters = variable->rank == 2 ? variable->dims[0] * variable->dims[1] : Mat_VarGetSize(variable);
    QString result = QString::fromUtf8(static_cast<const char*>(variable->data), int(characters));
    while (result.endsWith(QChar::Null)) result.chop(1);
    return result;
}
void verifyDouble(matvar_t* variable, const std::vector<double>& expected) {
    QVERIFY(variable);
    QCOMPARE(variable->class_type, MAT_C_DOUBLE);
    QCOMPARE(variable->data_type, MAT_T_DOUBLE);
    QCOMPARE(variable->rank, 2);
    QCOMPARE(variable->dims[0], expected.size());
    QCOMPARE(variable->dims[1], size_t(1));
    const auto* data = static_cast<const double*>(variable->data);
    for (size_t i = 0; i < expected.size(); ++i) {
        if (std::isnan(expected[i])) QVERIFY(std::isnan(data[i]));
        else if (std::isinf(expected[i])) QVERIFY(std::isinf(data[i]) && std::signbit(data[i]) == std::signbit(expected[i]));
        else QCOMPARE(data[i], expected[i]);
    }
}
struct MatFile { mat_t* value = nullptr; ~MatFile(){ if(value) Mat_Close(value); } };
}

class MatExportContractTests : public QObject {
    Q_OBJECT
private slots:
    void MAT_001_nullManagerFailsWithoutFile();
    void MAT_002_minimalFileHasMat5TimestampAndEmptyStruct();
    void MAT_003_allNumericTypesBecomeDoubleScalars();
    void MAT_004_vectorsPreserveColumnDimensionsAndUnequalLengths();
    void MAT_005_stringsUnicodeAndSelectionAreUtf8Characters();
    void MAT_006_repeatedExportsOverwriteAndKeepRequestedOrder();
    void MAT_007_invalidPathAndUnknownIdBehavior();
    void MAT_008_uiCallerUsesSameConvention();
    void MAT_009_uninitializedContainerExportsAsEmptyFields();
    void MAT_010_exportedMatImportsAsIndependentDataSet();
    void MAT_011_largeVectorExportPreservesDataWithoutExtraExporterCopy();
};

void MatExportContractTests::MAT_001_nullManagerFailsWithoutFile() {
    QTemporaryDir dir; QVERIFY(dir.isValid());
    MatExporter exporter(nullptr);
    QVERIFY(exporter.Export2Mat(dir.filePath("none.mat"), {}));
    QVERIFY(!QFileInfo::exists(dir.filePath("none.mat")));
}
void MatExportContractTests::MAT_002_minimalFileHasMat5TimestampAndEmptyStruct() {
    QObject owner; DataManagementSetClass manager(&owner); QTemporaryDir dir; QVERIFY(dir.isValid());
    const QString path=dir.filePath("empty.mat"); MatExporter exporter(&manager);
    QVERIFY(!exporter.Export2Mat(path, {}));
    MatFile file{Mat_Open(path.toUtf8().constData(), MAT_ACC_RDONLY)}; QVERIFY(file.value);
    QCOMPARE(Mat_GetVersion(file.value), MAT_FT_MAT5);
    matvar_t* timestamp=Mat_VarRead(file.value,"Timestamp"); QVERIFY(timestamp);
    QCOMPARE(timestamp->class_type, MAT_C_CHAR); QCOMPARE(timestamp->data_type, MAT_T_UTF8); QCOMPARE(timestamp->rank,2); QVERIFY(text(timestamp).startsWith("Measurement_")); Mat_VarFree(timestamp);
    matvar_t* channels=Mat_VarRead(file.value,"ExportedChannels"); QVERIFY(channels); QCOMPARE(channels->class_type,MAT_C_STRUCT); QCOMPARE(channels->rank,2); QCOMPARE(channels->dims[0],size_t(0)); QCOMPARE(channels->dims[1],size_t(1)); QCOMPARE(Mat_VarGetNumberOfFields(channels),unsigned(3)); Mat_VarFree(channels);
}
void MatExportContractTests::MAT_003_allNumericTypesBecomeDoubleScalars() {
    QObject owner; DataManagementSetClass manager(&owner);
    add(manager,"i8",numericT(int8_t(-8))); add(manager,"i16",numericT(int16_t(-16))); add(manager,"i32",numericT(int32_t(-32))); add(manager,"i64",numericT(int64_t(-64)));
    add(manager,"u8",numericT(uint8_t(8))); add(manager,"u16",numericT(uint16_t(16))); add(manager,"u32",numericT(uint32_t(32))); add(manager,"u64",numericT(uint64_t(64)));
    add(manager,"float",numericT(float(-1.5f))); add(manager,"bool",numericT(true)); add(manager,"nan",numeric(std::numeric_limits<double>::quiet_NaN())); add(manager,"inf",numeric(-std::numeric_limits<double>::infinity()));
    QStringList ids={"i8","i16","i32","i64","u8","u16","u32","u64","float","bool","nan","inf"}; QTemporaryDir dir; MatExporter exporter(&manager); QVERIFY(!exporter.Export2Mat(dir.filePath("numbers.mat"),ids));
    MatFile file{Mat_Open(dir.filePath("numbers.mat").toUtf8().constData(),MAT_ACC_RDONLY)}; QVERIFY(file.value); matvar_t* channels=Mat_VarRead(file.value,"ExportedChannels"); QVERIFY(channels); QCOMPARE(channels->dims[0],size_t(ids.size()));
    const std::vector<double> values={-8,-16,-32,-64,8,16,32,64,-1.5,1,std::numeric_limits<double>::quiet_NaN(),-std::numeric_limits<double>::infinity()};
    for(size_t i=0;i<values.size();++i){ QCOMPARE(text(field(channels,"ID",i)),ids.at(int(i))); QCOMPARE(text(field(channels,"Time",i)),QString()); verifyDouble(field(channels,"Data",i),{values[i]}); } Mat_VarFree(channels);
}
void MatExportContractTests::MAT_004_vectorsPreserveColumnDimensionsAndUnequalLengths() {
    QObject owner; DataManagementSetClass manager(&owner); add(manager,"trace",vectorValue({0,1,2},{10,20})); add(manager,"empty",vectorValue({},{})); QTemporaryDir dir; MatExporter exporter(&manager); QVERIFY(!exporter.Export2Mat(dir.filePath("vectors.mat"),{"trace","empty"}));
    MatFile file{Mat_Open(dir.filePath("vectors.mat").toUtf8().constData(),MAT_ACC_RDONLY)}; QVERIFY(file.value); matvar_t* channels=Mat_VarRead(file.value,"ExportedChannels"); QVERIFY(channels); verifyDouble(field(channels,"Time",0),{0,1,2}); verifyDouble(field(channels,"Data",0),{10,20}); QVERIFY(field(channels,"Time",1)); QCOMPARE(field(channels,"Time",1)->dims[0],size_t(0)); QCOMPARE(field(channels,"Data",1)->dims[0],size_t(0)); Mat_VarFree(channels);
}
void MatExportContractTests::MAT_005_stringsUnicodeAndSelectionAreUtf8Characters() {
    QObject owner; DataManagementSetClass manager(&owner); add(manager,"text",stringValue(QString::fromUtf8("Größe Ω & <x>"))); InterfaceData list; list.SetData(QStringList({QString::fromUtf8("erste Ω"),"second"})); add(manager,"list",list); InterfaceData selection; selection.SetData(GuiSelection(QString::fromUtf8("Auswahl Ω"),QStringList({"a"}))); add(manager,"selection",selection); QTemporaryDir dir; MatExporter exporter(&manager); QVERIFY(!exporter.Export2Mat(dir.filePath("strings.mat"),{"text","list","selection"}));
    MatFile file{Mat_Open(dir.filePath("strings.mat").toUtf8().constData(),MAT_ACC_RDONLY)}; QVERIFY(file.value); matvar_t* channels=Mat_VarRead(file.value,"ExportedChannels"); QVERIFY(channels); QCOMPARE(text(field(channels,"Data",0)),QString::fromUtf8("Größe Ω & <x>")); QCOMPARE(text(field(channels,"Data",1)),QString::fromUtf8("erste Ω")); QCOMPARE(text(field(channels,"Data",2)),QString::fromUtf8("Auswahl Ω")); Mat_VarFree(channels);
}
void MatExportContractTests::MAT_006_repeatedExportsOverwriteAndKeepRequestedOrder() {
    QObject owner; DataManagementSetClass manager(&owner); add(manager,"one",numeric(1)); add(manager,"two",numeric(2)); QTemporaryDir dir; QString path=dir.filePath("again.mat"); MatExporter exporter(&manager); QVERIFY(!exporter.Export2Mat(path,{"one","two"})); QVERIFY(!exporter.Export2Mat(path,{"two"})); MatFile file{Mat_Open(path.toUtf8().constData(),MAT_ACC_RDONLY)}; QVERIFY(file.value); matvar_t* channels=Mat_VarRead(file.value,"ExportedChannels"); QVERIFY(channels); QCOMPARE(channels->dims[0],size_t(1)); QCOMPARE(text(field(channels,"ID",0)),QString("two")); verifyDouble(field(channels,"Data",0),{2}); Mat_VarFree(channels);
}
void MatExportContractTests::MAT_007_invalidPathAndUnknownIdBehavior() {
    QObject owner; DataManagementSetClass manager(&owner); add(manager,"ok",numeric(1)); QTemporaryDir dir; MatExporter exporter(&manager); QVERIFY(exporter.Export2Mat(dir.filePath("missing/out.mat"),{"ok"})); QVERIFY(!exporter.Export2Mat(dir.filePath("unknown.mat"),{"missing"})); MatFile file{Mat_Open(dir.filePath("unknown.mat").toUtf8().constData(),MAT_ACC_RDONLY)}; QVERIFY(file.value); matvar_t* channels=Mat_VarRead(file.value,"ExportedChannels"); QVERIFY(channels); QCOMPARE(channels->dims[0],size_t(1)); QVERIFY(field(channels,"ID",0)); QVERIFY(field(channels,"Time",0)); QVERIFY(field(channels,"Data",0)); QCOMPARE(field(channels,"ID",0)->class_type, MAT_C_DOUBLE); QCOMPARE(field(channels,"ID",0)->dims[0],size_t(0)); QCOMPARE(field(channels,"Time",0)->dims[0],size_t(0)); QCOMPARE(field(channels,"Data",0)->dims[0],size_t(0)); Mat_VarFree(channels);
}
void MatExportContractTests::MAT_008_uiCallerUsesSameConvention() {
    MainWindow window; add(*window.GetLogic(),"ui",numeric(4.5)); QTemporaryDir dir; QVERIFY(!window.GetLogic()->Export2Mat(dir.filePath("ui.mat"),{"ui"})); QVERIFY(QFileInfo::exists(dir.filePath("ui.mat"))); QVERIFY(window.GetLogic()->Export2Mat(dir.filePath("none/ui.mat"),{"ui"}));
}
void MatExportContractTests::MAT_009_uninitializedContainerExportsAsEmptyFields() {
    QObject owner;
    DataManagementSetClass manager(&owner);
    manager.AddContainerElement("pending", "double", "Data", QString());
    QTemporaryDir dir;
    MatExporter exporter(&manager);
    QVERIFY(!exporter.Export2Mat(dir.filePath("pending.mat"), {"pending"}));

    MatFile file{Mat_Open(dir.filePath("pending.mat").toUtf8().constData(), MAT_ACC_RDONLY)};
    QVERIFY(file.value);
    matvar_t* channels = Mat_VarRead(file.value, "ExportedChannels");
    QVERIFY(channels);
    QCOMPARE(text(field(channels, "ID", 0)), QString("pending"));
    verifyDouble(field(channels, "Time", 0), {});
    verifyDouble(field(channels, "Data", 0), {});
    Mat_VarFree(channels);
}
void MatExportContractTests::MAT_010_exportedMatImportsAsIndependentDataSet() {
    QObject sourceOwner;
    DataManagementSetClass source(&sourceOwner);
    add(source, "trace", vectorValue({42.5, 43.5, 44.5}, {10, 20, 30}));
    add(source, "scalar", numeric(4.5));
    QTemporaryDir dir;
    const QString path = dir.filePath("source.mat");
    MatExporter exporter(&source);
    QVERIFY(!exporter.Export2Mat(path, {"trace", "scalar"}));

    QObject targetOwner;
    DataManagementSetClass target(&targetOwner);
    QSignalSpy addToExplorer(target.GetMessenger(), SIGNAL(AddElementToWidget(QString,InterfaceData)));
    QString root;
    QString error;
    QVERIFY2(MatDataImport::Import(target, path, &root, &error), qPrintable(error));
    QVERIFY(root.startsWith("Export_source_"));
    QCOMPARE(addToExplorer.count(), 2);
    for (const QList<QVariant>& emission : addToExplorer) {
        QCOMPARE(emission.at(1).value<InterfaceData>().GetType(), QString("Data"));
    }
    ToFormMapper* trace = target.GetContainer(root + "::trace");
    QVERIFY(trace);
    QVERIFY(trace->IsPairOfVectorOfDoubles());
    QCOMPARE(*trace->GetPointerPair().first, std::vector<double>({42.5, 43.5, 44.5}));
    QCOMPARE(*trace->GetPointerPair().second, std::vector<double>({10, 20, 30}));
    QVERIFY(trace->GetPointerPair().third);
    QCOMPARE(*trace->GetPointerPair().third, 42.5);
    ToFormMapper* scalar = target.GetContainer(root + "::scalar");
    QVERIFY(scalar);
    QCOMPARE(scalar->GetAsDouble(), 4.5);
}
void MatExportContractTests::MAT_011_largeVectorExportPreservesDataWithoutExtraExporterCopy() {
    constexpr size_t sampleCount = 262144;
    std::vector<double> time(sampleCount);
    std::vector<double> data(sampleCount);
    for (size_t index = 0; index < sampleCount; ++index) {
        time[index] = 1000.0 + static_cast<double>(index) * 0.001;
        data[index] = static_cast<double>(index) * 0.25;
    }

    QObject owner;
    DataManagementSetClass manager(&owner);
    add(manager, "large", vectorValue(time, data));
    QTemporaryDir dir;
    MatExporter exporter(&manager);
    const QString path = dir.filePath("large.mat");
    QVERIFY2(!exporter.Export2Mat(path, {"large"}), "large vector export must succeed");

    MatFile file{Mat_Open(path.toUtf8().constData(), MAT_ACC_RDONLY)};
    QVERIFY(file.value);
    matvar_t* channels = Mat_VarRead(file.value, "ExportedChannels");
    QVERIFY(channels);
    matvar_t* exportedTime = field(channels, "Time", 0);
    matvar_t* exportedData = field(channels, "Data", 0);
    QVERIFY(exportedTime && exportedData);
    QCOMPARE(exportedTime->dims[0], sampleCount);
    QCOMPARE(exportedData->dims[0], sampleCount);
    const auto* writtenTime = static_cast<const double*>(exportedTime->data);
    const auto* writtenData = static_cast<const double*>(exportedData->data);
    QCOMPARE(writtenTime[0], time.front());
    QCOMPARE(writtenTime[sampleCount - 1], time.back());
    QCOMPARE(writtenData[0], data.front());
    QCOMPARE(writtenData[sampleCount - 1], data.back());
    Mat_VarFree(channels);
}
QTEST_MAIN(MatExportContractTests)
#include "MatExportContractTests.moc"
