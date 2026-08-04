#include <QtTest>
#include <QFileInfo>
#include <QTemporaryDir>
#include <cmath>
#include <limits>
#include <highfive/H5File.hpp>
#include <highfive/H5Easy.hpp>
#include "Export/export2highfive.h"
#include "DataManagement/DataManagementClass.h"

namespace {
InterfaceData scalar(double v) { InterfaceData d; d.SetData(v); return d; }
template<class T> InterfaceData scalarT(T v) { InterfaceData d; d.SetData(v); return d; }
InterfaceData vectorData(const std::vector<double>& t,const std::vector<double>& d) { InterfaceData x; x.SetData(DataPair(boost::shared_ptr<std::vector<double>>(new std::vector<double>(t)),boost::shared_ptr<std::vector<double>>(new std::vector<double>(d)))); return x; }
void add(DataManagementClass& m,const QString& id,InterfaceData v){m.AddContainerElement(id,v.GetDataType(),"Data",{});m.SetData(id,v);}

// Test-only seam for UIDataManagementSetClass::Export2Hdf5. The production
// implementation requires MainWindow, widgets and generated UI headers even
// though this operation only constructs Export2HDF5, returns its value on
// success and catches all exceptions. Keeping that GUI graph out of this
// contract target makes the HDF5 contract reproducible; this class is neither
// included nor linked by LabAnalyser.pro.
bool uiExportSeam(DataManagementClass* manager, const QString& path, const QStringList& ids) {
    try { return Export2HDF5(manager).Export(path, ids); }
    catch (...) { return false; }
}
}
class Hdf5ExportContractTests: public QObject { Q_OBJECT
private slots:
 void HDF5_001_minimalFileAndTimestamp(); void HDF5_002_scalarsTypesAndSpecialValues(); void HDF5_003_nestedNamesAndVectors(); void HDF5_004_stringsUnicodeAndEmptyVectors(); void HDF5_005_overwriteUnknownAndInvalidPaths(); void HDF5_006_uiCallerErrorConvention();
};
void Hdf5ExportContractTests::HDF5_001_minimalFileAndTimestamp(){QObject o;DataManagementClass m(&o);QTemporaryDir d;QVERIFY(d.isValid());Export2HDF5 e(&m);QVERIFY(!e.Export(d.filePath("minimal.h5"),{}));HighFive::File f(d.filePath("minimal.h5").toStdString(),HighFive::File::ReadOnly);QVERIFY(f.exist("Timestamp"));QVERIFY(H5Easy::load<std::string>(f,"Timestamp").rfind("Measurement_",0)==0);}
void Hdf5ExportContractTests::HDF5_002_scalarsTypesAndSpecialValues(){QObject o;DataManagementClass m(&o);add(m,"neg",scalar(-2.5));add(m,"u8",scalarT(uint8_t(255)));add(m,"nan",scalar(std::numeric_limits<double>::quiet_NaN()));add(m,"inf",scalar(-std::numeric_limits<double>::infinity()));QTemporaryDir d;Export2HDF5 e(&m);QVERIFY(!e.Export(d.filePath("s.h5"),{"neg","u8","nan","inf"}));HighFive::File f(d.filePath("s.h5").toStdString(),HighFive::File::ReadOnly);QCOMPARE(H5Easy::load<double>(f,"neg"),-2.5);QCOMPARE(H5Easy::load<double>(f,"u8"),255.0);QVERIFY(std::isnan(H5Easy::load<double>(f,"nan")));QVERIFY(std::isinf(H5Easy::load<double>(f,"inf")));}
void Hdf5ExportContractTests::HDF5_003_nestedNamesAndVectors(){QObject o;DataManagementClass m(&o);add(m,"device::trace",vectorData({0,1,2},{10,20}));QTemporaryDir d;Export2HDF5 e(&m);QVERIFY(!e.Export(d.filePath("v.h5"),{"device::trace"}));HighFive::File f(d.filePath("v.h5").toStdString(),HighFive::File::ReadOnly);QVERIFY(f.exist("device/trace/Time"));QCOMPARE(H5Easy::load<std::vector<double>>(f,"device/trace/Time"),std::vector<double>({0,1,2}));QCOMPARE(H5Easy::load<std::vector<double>>(f,"device/trace/Data"),std::vector<double>({10,20}));}
void Hdf5ExportContractTests::HDF5_004_stringsUnicodeAndEmptyVectors(){QObject o;DataManagementClass m(&o);InterfaceData s;s.SetData(QString::fromUtf8("Größe Ω"));add(m,"text",s);add(m,"empty",vectorData({},{}));QTemporaryDir d;Export2HDF5 e(&m);QVERIFY(!e.Export(d.filePath("t.h5"),{"text","empty"}));HighFive::File f(d.filePath("t.h5").toStdString(),HighFive::File::ReadOnly);QCOMPARE(QString::fromStdString(H5Easy::load<std::string>(f,"text")),QString::fromUtf8("Größe Ω"));QCOMPARE(H5Easy::load<std::vector<double>>(f,"empty/Time").size(),size_t(0));}
void Hdf5ExportContractTests::HDF5_005_overwriteUnknownAndInvalidPaths(){QObject o;DataManagementClass m(&o);add(m,"one",scalar(1));add(m,"two",scalar(2));QTemporaryDir d;QString p=d.filePath("again.h5");Export2HDF5 e(&m);QVERIFY(!e.Export(p,{"one","missing"}));QVERIFY(!e.Export(p,{"two"}));HighFive::File f(p.toStdString(),HighFive::File::ReadOnly);QVERIFY(!f.exist("one"));QCOMPARE(H5Easy::load<double>(f,"two"),2.0);QVERIFY_EXCEPTION_THROWN(e.Export(d.filePath("none/out.h5"),{"one"}),std::exception);}
void Hdf5ExportContractTests::HDF5_006_uiCallerErrorConvention(){QObject o;DataManagementClass m(&o);add(m,"ui",scalar(4));QTemporaryDir d;QVERIFY(!uiExportSeam(&m,d.filePath("ui.h5"),{"ui"}));QVERIFY(QFileInfo::exists(d.filePath("ui.h5")));QVERIFY(!uiExportSeam(&m,d.filePath("none/ui.h5"),{"ui"}));}
QTEST_MAIN(Hdf5ExportContractTests)
#include "Hdf5ExportContractTests.moc"
