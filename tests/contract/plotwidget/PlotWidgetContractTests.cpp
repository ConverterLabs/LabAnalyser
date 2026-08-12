#include <QtTest>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <cmath>

#include "DataManagement/DataMessengerClass.h"
#include "DropWidgets/Plots/FFTPlotWidget.h"
#include "DropWidgets/Plots/PlotWidget.h"
#include "mainwindow.h"

namespace PlotWidgetTestHooks
{
void setFftwAllocationFailure(bool enabled);
void setFftwPlanFailure(bool enabled);
}

class PlotWidgetContractTests final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void PLOT_001_construction_parenting_and_default_state();
    void PLOT_002_single_graph_alias_and_messenger_update();
    void PLOT_003_multiple_repeated_graphs_visibility_and_removal();
    void PLOT_004_data_shapes_and_non_finite_values();
    void PLOT_005_ranges_xml_and_navigation_state();
    void PLOT_006_connect_to_id_and_safe_cleanup();
    void FFT_001_fft_widget_construction_and_destruction();
    void FFT_002_uniform_sine_frequency_bins_and_mode();
    void FFT_003_dc_and_sine_amplitude_scaling();
    void FFT_004_multiple_graphs_keep_separate_fft_vectors();
    void FFT_005_return_to_time_domain_restores_state();
    void FFT_006_messenger_update_recalculates_without_new_graph();
    void FFT_007_non_uniform_samples_use_mean_delta_t();
    void FFT_008_empty_and_mismatched_vectors_return_safely();
    void FFT_009_zero_and_one_sample_do_not_create_fft_data();
    void FFT_010_fftw_allocation_and_plan_failures_leave_plot_usable();

private:
    static InterfaceData data(const std::vector<double>& time,
                              const std::vector<double>& values,
                              double offset = 0.0);
    static void publish(MainWindow& window, const QString& id, const InterfaceData& value);
    static QCPGraph* graph(PlotWidget& plot, int index);
    static void toggleFrequency(PlotWidget& plot);
    QString originalWorkingDirectory;
};

InterfaceData PlotWidgetContractTests::data(const std::vector<double>& time,
                                            const std::vector<double>& values,
                                            double offset)
{
    InterfaceData result("vector<double>", "Data");
    result.SetData(DataPair(boost::shared_ptr<std::vector<double>>(new std::vector<double>(time)),
                            boost::shared_ptr<std::vector<double>>(new std::vector<double>(values)),
                            offset));
    return result;
}

void PlotWidgetContractTests::publish(MainWindow& window, const QString& id, const InterfaceData& value)
{
    window.GetLogic()->GetMessenger()->MessageReceiver("publish", id, value);
    QVERIFY2(window.GetLogic()->GetContainer(id) != nullptr, qPrintable(id));
}

QCPGraph* PlotWidgetContractTests::graph(PlotWidget& plot, int index)
{
    return plot.graph(index);
}

void PlotWidgetContractTests::toggleFrequency(PlotWidget& plot)
{
    QVERIFY(QMetaObject::invokeMethod(&plot, "ToggleTimeFreq", Qt::DirectConnection));
}

void PlotWidgetContractTests::initTestCase()
{
    QCOMPARE(qEnvironmentVariable("QT_QPA_PLATFORM"), QString("offscreen"));
    QCOMPARE(QSettings::defaultFormat(), QSettings::IniFormat);
    QVERIFY(QStandardPaths::isTestModeEnabled());
    originalWorkingDirectory = qEnvironmentVariable("LABANALYSER_TEST_WORKING_ROOT");
    QVERIFY(QDir(originalWorkingDirectory).exists());
}

void PlotWidgetContractTests::PLOT_001_construction_parenting_and_default_state()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());

    QCOMPARE(plot.parentWidget(), &host);
    QCOMPARE(plot.graphCount(), 0);
    QVERIFY(plot.acceptDrops());
    QCOMPARE(plot.xAxis->label(), QString("t [s]"));
    QCOMPARE(plot.yAxis->label(), QString("Data"));
    QCOMPARE(plot.xAxis->range().lower, -10.0);
    QCOMPARE(plot.xAxis->range().upper, 10.0);
    QCOMPARE(plot.yAxis->range().lower, -10.0);
    QCOMPARE(plot.yAxis->range().upper, 10.0);
    QVERIFY(plot.interactions().testFlag(QCP::iRangeDrag));
    QVERIFY(plot.interactions().testFlag(QCP::iRangeZoom));
    QVERIFY(plot.interactions().testFlag(QCP::iSelectAxes));
    QVERIFY(plot.legend->visible());
    QVERIFY(plot.findChild<QWidget*>("PlotToolbox"));
    QVERIFY(plot.findChild<QWidget*>("MeasurementPanel"));
    QVERIFY(plot.findChild<QTimer*>());
}

void PlotWidgetContractTests::PLOT_002_single_graph_alias_and_messenger_update()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("plot-messenger");
    const QString id = "Device::Signal";
    const InterfaceData initial = data({0.0, 1.0, 2.0}, {2.0, 4.0, 6.0}, 0.5);
    publish(window, id, initial);
    window.GetLogic()->SetAlias(id, "Voltage");

    QSignalSpy received(window.GetLogic()->GetMessenger(), &MessengerClass::NewDataReceived);
    plot.AddCustomGraph(id);
    QCOMPARE(plot.graphCount(), 1);
    QCPGraph* first = graph(plot, 0);
    QCOMPARE(first->ID(), id);
    QCOMPARE(first->name(), QString("Voltage"));
    QCOMPARE(first->GetXDataPointer()->size(), size_t(3));
    QCOMPARE(first->GetYDataPointer()->at(2), 6.0);
    QVERIFY(plot.legend->itemWithPlottable(first));

    const InterfaceData replacement = data({0.0, 1.0, 2.0}, {-1.0, 0.0, 1.0}, 0.5);
    window.GetLogic()->GetMessenger()->MessageReceiver("set", id, replacement);
    QCOMPARE(received.count(), 1);
    QCOMPARE(received.at(0).at(0).toString(), id);
    plot.UpdateGraphs(QString(), true);
    QCOMPARE(first->GetYDataPointer()->at(0), -1.0);
    QCOMPARE(first->GetYDataPointer()->at(2), 1.0);
}

void PlotWidgetContractTests::PLOT_003_multiple_repeated_graphs_visibility_and_removal()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("plot-graphs");
    publish(window, "D::A", data({0.0, 1.0}, {1.0, 2.0}));
    publish(window, "D::B", data({0.0, 1.0}, {3.0, 4.0}));

    plot.AddCustomGraph("D::A");
    plot.AddCustomGraph("D::B");
    plot.AddCustomGraph("D::A");
    QCOMPARE(plot.graphCount(), 3);
    QCOMPARE(graph(plot, 0)->ID(), QString("D::A"));
    QCOMPARE(graph(plot, 1)->ID(), QString("D::B"));
    QCOMPARE(graph(plot, 2)->ID(), QString("D::A"));
    graph(plot, 1)->setVisible(false);
    QVERIFY(!graph(plot, 1)->visible());

    graph(plot, 1)->setSelected(true);
    QVERIFY(QMetaObject::invokeMethod(&plot, "removeSelectedGraph", Qt::DirectConnection));
    QCOMPARE(plot.graphCount(), 2);
    QVERIFY(QMetaObject::invokeMethod(&plot, "removeAllGraphs", Qt::DirectConnection));
    QCOMPARE(plot.graphCount(), 0);
}

void PlotWidgetContractTests::PLOT_004_data_shapes_and_non_finite_values()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("plot-shapes");
    publish(window, "D::Empty", data({}, {}));
    publish(window, "D::Single", data({4.0}, {7.0}));
    publish(window, "D::NonFinite", data({0.0, 1.0}, {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()}));
    publish(window, "D::Mismatch", data({0.0, 1.0, 2.0}, {5.0, 6.0}));

    plot.AddCustomGraph("D::Empty", true);
    plot.AddCustomGraph("D::Single", true);
    plot.AddCustomGraph("D::NonFinite", true);
    plot.AddCustomGraph("D::Mismatch", true);
    QCOMPARE(plot.graphCount(), 4);
    QCOMPARE(graph(plot, 0)->GetXDataPointer()->size(), size_t(0));
    QCOMPARE(graph(plot, 1)->GetYDataPointer()->at(0), 7.0);
    QVERIFY(std::isnan(graph(plot, 2)->GetYDataPointer()->at(0)));
    QVERIFY(std::isinf(graph(plot, 2)->GetYDataPointer()->at(1)));
    QCOMPARE(graph(plot, 3)->GetXDataPointer()->size(), size_t(3));
    QCOMPARE(graph(plot, 3)->GetYDataPointer()->size(), size_t(2));
    plot.UpdateGraphs(QString(), true);
}

void PlotWidgetContractTests::PLOT_005_ranges_xml_and_navigation_state()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("plot-state");
    publish(window, "D::Range", data({0.0, 1.0, 2.0}, {1.0, 2.0, 3.0}));
    plot.AddCustomGraph("D::Range");
    QVERIFY(QMetaObject::invokeMethod(&plot, "ResetZoom", Qt::DirectConnection));
    QCOMPARE(plot.xAxis->range().lower, 0.0);
    QCOMPARE(plot.xAxis->range().upper, 2.0);
    QCOMPARE(plot.yAxis->range().lower, 0.9);
    QCOMPARE(plot.yAxis->range().upper, 3.1);

    QKeyEvent press(QEvent::KeyPress, Qt::Key_Control, Qt::NoModifier);
    plot.keyPressEvent(&press);
    QVERIFY(!plot.interactions().testFlag(QCP::iRangeDrag));
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Control, Qt::NoModifier);
    plot.keyReleaseEvent(&release);
    QVERIFY(plot.interactions().testFlag(QCP::iRangeDrag));
    QVERIFY(plot.interactions().testFlag(QCP::iRangeZoom));

    std::vector<std::pair<QString, QString>> attributes;
    QString text;
    QVERIFY(plot.SaveToXML(attributes, text));
    plot.xAxis->setLabel("changed");
    plot.yAxis->setLabel("changed");
    QVERIFY(plot.LoadFromXML(attributes, text));
    QCOMPARE(plot.xAxis->label(), QString("t [s]"));
    QCOMPARE(plot.yAxis->label(), QString("Data"));
}

void PlotWidgetContractTests::PLOT_006_connect_to_id_and_safe_cleanup()
{
    MainWindow window;
    QWidget host(&window);
    auto* plot = new PlotWidget(&window, &host, window.statusBar());
    plot->setObjectName("plot-cleanup");
    publish(window, "D::Connected", data({0.0, 1.0}, {8.0, 9.0}));
    plot->ConnectToID(window.GetLogic(), "D::Connected");
    QCOMPARE(plot->graphCount(), 1);
    QCOMPARE(graph(*plot, 0)->ID(), QString("D::Connected"));
    window.GetLogic()->GetMessenger()->MessageReceiver("set", "D::Connected", data({0.0, 1.0}, {10.0, 11.0}));
    plot->UpdateGraphs(QString(), true);
    QCOMPARE(graph(*plot, 0)->GetYDataPointer()->at(1), 11.0);

    QVERIFY(QMetaObject::invokeMethod(plot, "removeAllGraphs", Qt::DirectConnection));
    QCOMPARE(plot->graphCount(), 0);
    QPointer<PlotWidget> guard(plot);
    delete plot;
    QVERIFY(guard.isNull());
}

void PlotWidgetContractTests::FFT_001_fft_widget_construction_and_destruction()
{
    QWidget host;
    QPointer<FFTPlotWidget> plot(new FFTPlotWidget(&host));
    QCOMPARE(plot->parentWidget(), &host);
    QVERIFY(plot->testAttribute(Qt::WA_AcceptTouchEvents));
    QCOMPARE(plot->graphCount(), 0);
    delete plot;
    QVERIFY(plot.isNull());
}

void PlotWidgetContractTests::FFT_002_uniform_sine_frequency_bins_and_mode()
{
    // Eight samples at fs=8 Hz: y[n]=2*sin(2*pi*n/8), so bin 1 is 2 Hz? No:
    // n/fs gives a one-Hz waveform; the expected one-sided bin is 1 Hz, 2.0.
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("fft-uniform-sine");
    std::vector<double> time;
    std::vector<double> values;
    for (int n = 0; n < 8; ++n) {
        time.push_back(static_cast<double>(n) / 8.0);
        values.push_back(2.0 * std::sin(2.0 * M_PI * static_cast<double>(n) / 8.0));
    }
    publish(window, "FFT::Sine", data(time, values));
    plot.AddCustomGraph("FFT::Sine");

    toggleFrequency(plot);
    QCPGraph* fftGraph = graph(plot, 0);
    const auto frequencies = fftGraph->GetXFFTPointer();
    const auto amplitudes = fftGraph->GetYFFTPointer();
    QVERIFY(frequencies);
    QVERIFY(amplitudes);
    QCOMPARE(frequencies->size(), size_t(10)); // 2*(N/2+1)
    QCOMPARE(amplitudes->size(), size_t(10));
    QCOMPARE(frequencies->at(0), 0.0);
    QCOMPARE(frequencies->at(2), 1.0);
    QVERIFY(std::fabs(amplitudes->at(3) - 2.0) < 1e-10);
    QCOMPARE(plot.xAxis->label(), QString("f [Hz]"));
    QCOMPARE(plot.yAxis->label(), QString("Amplitude"));
    QCOMPARE(fftGraph->lineStyle(), QCPGraph::lsImpulse);
    QCOMPARE(fftGraph->scatterStyle().shape(), QCPScatterStyle::ssCross);
}

void PlotWidgetContractTests::FFT_003_dc_and_sine_amplitude_scaling()
{
    // N=8, fs=8 Hz, y[n]=1.5+2*sin(2*pi*n/8): DC=1.5 and bin 1=2.0.
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("fft-dc-sine");
    std::vector<double> time;
    std::vector<double> values;
    for (int n = 0; n < 8; ++n) {
        time.push_back(static_cast<double>(n) / 8.0);
        values.push_back(1.5 + 2.0 * std::sin(2.0 * M_PI * static_cast<double>(n) / 8.0));
    }
    publish(window, "FFT::DcSine", data(time, values));
    plot.AddCustomGraph("FFT::DcSine");
    toggleFrequency(plot);
    const auto amplitudes = graph(plot, 0)->GetYFFTPointer();
    QVERIFY(amplitudes);
    QVERIFY(std::fabs(amplitudes->at(1) - 1.5) < 1e-10);
    QVERIFY(std::fabs(amplitudes->at(3) - 2.0) < 1e-10);
    QVERIFY(std::fabs(amplitudes->at(5)) < 1e-10);
}

void PlotWidgetContractTests::FFT_004_multiple_graphs_keep_separate_fft_vectors()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("fft-multiple");
    std::vector<double> time;
    std::vector<double> oneHz;
    std::vector<double> twoHz;
    for (int n = 0; n < 8; ++n) {
        time.push_back(static_cast<double>(n) / 8.0);
        oneHz.push_back(std::sin(2.0 * M_PI * static_cast<double>(n) / 8.0));
        twoHz.push_back(3.0 * std::sin(4.0 * M_PI * static_cast<double>(n) / 8.0));
    }
    publish(window, "FFT::One", data(time, oneHz));
    publish(window, "FFT::Two", data(time, twoHz));
    plot.AddCustomGraph("FFT::One");
    plot.AddCustomGraph("FFT::Two");
    toggleFrequency(plot);
    QCOMPARE(plot.graphCount(), 2);
    const auto first = graph(plot, 0)->GetYFFTPointer();
    const auto second = graph(plot, 1)->GetYFFTPointer();
    QVERIFY(first && second);
    QVERIFY(std::fabs(first->at(3) - 1.0) < 1e-10);
    QVERIFY(std::fabs(second->at(5) - 3.0) < 1e-10);
    QVERIFY(std::fabs(second->at(3)) < 1e-10);
}

void PlotWidgetContractTests::FFT_005_return_to_time_domain_restores_state()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("fft-return");
    plot.xAxis->setRange(-2.0, 4.0);
    publish(window, "FFT::Return", data({0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875},
                                          {0.0, 1.0, 0.0, -1.0, 0.0, 1.0, 0.0, -1.0}));
    plot.AddCustomGraph("FFT::Return");
    plot.xAxis->setRange(-2.0, 4.0);
    QCPGraph* fftGraph = graph(plot, 0);
    const QPen timePen = fftGraph->pen();
    const QCPGraph::LineStyle timeStyle = fftGraph->lineStyle();
    const QCPScatterStyle timeScatter = fftGraph->scatterStyle();
    toggleFrequency(plot);
    toggleFrequency(plot);
    QCOMPARE(plot.xAxis->label(), QString("t [s]"));
    QCOMPARE(plot.yAxis->label(), QString("Data"));
    QCOMPARE(plot.xAxis->range().lower, -2.0);
    QCOMPARE(plot.xAxis->range().upper, 4.0);
    QCOMPARE(fftGraph->pen(), timePen);
    QCOMPARE(fftGraph->lineStyle(), timeStyle);
    QCOMPARE(fftGraph->scatterStyle().shape(), timeScatter.shape());
}

void PlotWidgetContractTests::FFT_006_messenger_update_recalculates_without_new_graph()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("fft-messenger");
    const std::vector<double> time{0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875};
    publish(window, "FFT::Updated", data(time, {0.0, std::sqrt(0.5), 1.0, std::sqrt(0.5), 0.0,
                                                  -std::sqrt(0.5), -1.0, -std::sqrt(0.5)}));
    plot.AddCustomGraph("FFT::Updated");
    toggleFrequency(plot);
    QCOMPARE(plot.graphCount(), 1);
    QVERIFY(std::fabs(graph(plot, 0)->GetYFFTPointer()->at(3) - 1.0) < 1e-10);
    window.GetLogic()->GetMessenger()->MessageReceiver("set", "FFT::Updated",
        data(time, {0.0, 2.0 * std::sqrt(0.5), 2.0, 2.0 * std::sqrt(0.5), 0.0,
                    -2.0 * std::sqrt(0.5), -2.0, -2.0 * std::sqrt(0.5)}));
    plot.UpdateGraphs(QString(), true);
    QCOMPARE(plot.graphCount(), 1);
    QVERIFY(std::fabs(graph(plot, 0)->GetYFFTPointer()->at(3) - 2.0) < 1e-10);
}

void PlotWidgetContractTests::FFT_007_non_uniform_samples_use_mean_delta_t()
{
    // x={0, .1, .4, .6}; mean delta T=.2, N=4, therefore df=1/(.2*4)=1.25 Hz.
    MainWindow window;
    QWidget host(&window);
    PlotWidget plot(&window, &host, window.statusBar());
    plot.setObjectName("fft-nonuniform");
    publish(window, "FFT::NonUniform", data({0.0, 0.1, 0.4, 0.6}, {0.0, 1.0, 0.0, -1.0}));
    plot.AddCustomGraph("FFT::NonUniform");
    toggleFrequency(plot);
    const auto frequencies = graph(plot, 0)->GetXFFTPointer();
    QVERIFY(frequencies);
    QCOMPARE(frequencies->size(), size_t(6));
    QVERIFY(std::fabs(frequencies->at(2) - 1.25) < 1e-12);
    QVERIFY(std::fabs(frequencies->at(4) - 2.5) < 1e-12);
}

void PlotWidgetContractTests::FFT_008_empty_and_mismatched_vectors_return_safely()
{
    MainWindow window;
    QWidget host(&window);
    PlotWidget emptyPlot(&window, &host, window.statusBar());
    emptyPlot.setObjectName("fft-empty");
    publish(window, "FFT::Empty", data({}, {}));
    emptyPlot.AddCustomGraph("FFT::Empty", true);
    toggleFrequency(emptyPlot);
    QCOMPARE(emptyPlot.graphCount(), 1);
    QVERIFY(!graph(emptyPlot, 0)->GetXFFTPointer());
    QVERIFY(!graph(emptyPlot, 0)->GetYFFTPointer());

    PlotWidget mismatchPlot(&window, &host, window.statusBar());
    mismatchPlot.setObjectName("fft-mismatch");
    publish(window, "FFT::Mismatch", data({0.0, 0.125}, {1.0}));
    mismatchPlot.AddCustomGraph("FFT::Mismatch", true);
    toggleFrequency(mismatchPlot);
    QCOMPARE(mismatchPlot.graphCount(), 1);
    QVERIFY(!graph(mismatchPlot, 0)->GetXFFTPointer());
    QVERIFY(!graph(mismatchPlot, 0)->GetYFFTPointer());
}

void PlotWidgetContractTests::FFT_009_zero_and_one_sample_do_not_create_fft_data()
{
    MainWindow window;
    QWidget host(&window);

    PlotWidget emptyPlot(&window, &host, window.statusBar());
    publish(window, "FFT::Zero", data({}, {}));
    emptyPlot.AddCustomGraph("FFT::Zero", true);
    toggleFrequency(emptyPlot);
    QVERIFY(!graph(emptyPlot, 0)->GetXFFTPointer());
    QVERIFY(!graph(emptyPlot, 0)->GetYFFTPointer());
    QCOMPARE(graph(emptyPlot, 0)->GetXDataPointer()->size(), size_t(0));
    QCOMPARE(graph(emptyPlot, 0)->GetYDataPointer()->size(), size_t(0));

    PlotWidget singlePlot(&window, &host, window.statusBar());
    publish(window, "FFT::Single", data({3.5}, {7.25}));
    singlePlot.AddCustomGraph("FFT::Single", true);
    toggleFrequency(singlePlot);
    QVERIFY(!graph(singlePlot, 0)->GetXFFTPointer());
    QVERIFY(!graph(singlePlot, 0)->GetYFFTPointer());
    QCOMPARE(graph(singlePlot, 0)->GetXDataPointer()->at(0), 3.5);
    QCOMPARE(graph(singlePlot, 0)->GetYDataPointer()->at(0), 7.25);
}

void PlotWidgetContractTests::FFT_010_fftw_allocation_and_plan_failures_leave_plot_usable()
{
    const std::vector<double> time{0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875};
    const std::vector<double> values{0.0, std::sqrt(0.5), 1.0, std::sqrt(0.5), 0.0,
                                     -std::sqrt(0.5), -1.0, -std::sqrt(0.5)};
    MainWindow window;
    QWidget host(&window);

    PlotWidget allocationFailure(&window, &host, window.statusBar());
    publish(window, "FFT::AllocationFailure", data(time, values));
    allocationFailure.AddCustomGraph("FFT::AllocationFailure");
    PlotWidgetTestHooks::setFftwAllocationFailure(true);
    toggleFrequency(allocationFailure);
    PlotWidgetTestHooks::setFftwAllocationFailure(false);
    QVERIFY(!graph(allocationFailure, 0)->GetXFFTPointer());
    QVERIFY(!graph(allocationFailure, 0)->GetYFFTPointer());

    PlotWidget planFailure(&window, &host, window.statusBar());
    publish(window, "FFT::PlanFailure", data(time, values));
    planFailure.AddCustomGraph("FFT::PlanFailure");
    PlotWidgetTestHooks::setFftwPlanFailure(true);
    toggleFrequency(planFailure);
    PlotWidgetTestHooks::setFftwPlanFailure(false);
    QVERIFY(!graph(planFailure, 0)->GetXFFTPointer());
    QVERIFY(!graph(planFailure, 0)->GetYFFTPointer());

    PlotWidget usablePlot(&window, &host, window.statusBar());
    publish(window, "FFT::AfterFailure", data(time, values));
    usablePlot.AddCustomGraph("FFT::AfterFailure");
    toggleFrequency(usablePlot);
    QVERIFY(graph(usablePlot, 0)->GetXFFTPointer());
    QVERIFY(std::fabs(graph(usablePlot, 0)->GetYFFTPointer()->at(3) - 1.0) < 1e-10);
}

int main(int argc, char** argv)
{
    QTemporaryDir settingsDirectory;
    QTemporaryDir workingDirectory;
    if (!settingsDirectory.isValid() || !workingDirectory.isValid())
        return 2;
    const QString startupWorkingDirectory = QDir::currentPath();
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("LABANALYSER_TEST_SETTINGS_ROOT", settingsDirectory.path().toUtf8());
    qputenv("LABANALYSER_TEST_WORKING_ROOT", workingDirectory.path().toUtf8());
    QStandardPaths::setTestModeEnabled(true);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());
    if (!QDir::setCurrent(workingDirectory.path()))
        return 3;
    QApplication application(argc, argv);
    application.setApplicationName("LabAnalyserPlotWidgetContractTest");
    application.setOrganizationName("LabAnalyserTests");
    PlotWidgetContractTests tests;
    const int result = QTest::qExec(&tests, argc, argv);
    QDir::setCurrent(startupWorkingDirectory);
    return result;
}

#include "PlotWidgetContractTests.moc"
