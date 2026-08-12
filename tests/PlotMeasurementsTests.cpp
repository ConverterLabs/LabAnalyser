#include <QtTest>

#include "DropWidgets/Plots/PlotMeasurements.h"

#include <cmath>

namespace
{
const double kPi = 3.14159265358979323846;
}

class PlotMeasurementsTests : public QObject
{
    Q_OBJECT

private slots:
    void constantSignal();
    void linearSignal();
    void sineAndHarmonics();
    void nonUniformSamples();
    void invalidIntervals();
    void degenerateInputNormalization();
};

void PlotMeasurementsTests::constantSignal()
{
    const std::vector<double> x{0.0, 1.0, 2.0};
    const std::vector<double> y{5.0, 5.0, 5.0};
    const auto samples = PlotMeasurements::normalizedSamples(x, y);
    const auto statistics = PlotMeasurements::calculateIntervalStatistics(samples, 0.0, 2.0);
    QVERIFY(statistics.valid);
    QCOMPARE(statistics.mean, 5.0);
    QCOMPARE(statistics.rms, 5.0);

    double value = 0.0;
    QVERIFY(PlotMeasurements::interpolate(samples, 0.5, &value));
    QCOMPARE(value, 5.0);
    double first = 0.0;
    double second = 0.0;
    QVERIFY(PlotMeasurements::interpolate(samples, 0.25, &first));
    QVERIFY(PlotMeasurements::interpolate(samples, 1.75, &second));
    QCOMPARE(second - first, 0.0);
    QCOMPARE((second - first) / 1.5, 0.0);
}

void PlotMeasurementsTests::linearSignal()
{
    const std::vector<double> x{0.0, 1.0, 2.0};
    const std::vector<double> y{1.0, 3.0, 5.0};
    const auto samples = PlotMeasurements::normalizedSamples(x, y);
    const auto statistics = PlotMeasurements::calculateIntervalStatistics(samples, 0.25, 1.75);
    QVERIFY(statistics.valid);
    QCOMPARE(statistics.mean, 3.0);

    double y1 = 0.0;
    double y2 = 0.0;
    QVERIFY(PlotMeasurements::interpolate(samples, 0.25, &y1));
    QVERIFY(PlotMeasurements::interpolate(samples, 1.75, &y2));
    QCOMPARE(y2 - y1, 3.0);
    QCOMPARE((y2 - y1) / 1.5, 2.0);
}

void PlotMeasurementsTests::sineAndHarmonics()
{
    std::vector<double> x;
    std::vector<double> sine;
    std::vector<double> harmonic;
    for (int i = 0; i <= 128; ++i)
    {
        const double t = i / 128.0;
        x.push_back(t);
        sine.push_back(2.0 * std::sin(2.0 * kPi * t));
        harmonic.push_back(std::sin(2.0 * kPi * t) + 0.1 * std::sin(4.0 * kPi * t));
    }

    const auto sineSamples = PlotMeasurements::normalizedSamples(x, sine);
    const auto sineStatistics = PlotMeasurements::calculateIntervalStatistics(sineSamples, 0.0, 1.0);
    QVERIFY(std::fabs(sineStatistics.mean) < 1e-12);
    QVERIFY(std::fabs(sineStatistics.rms - std::sqrt(2.0)) < 0.01);
    QVERIFY(PlotMeasurements::calculateThdPercent(sineSamples, 0.0, 1.0) < 0.01);

    const auto harmonicSamples = PlotMeasurements::normalizedSamples(x, harmonic);
    QVERIFY(std::fabs(PlotMeasurements::calculateThdPercent(harmonicSamples, 0.0, 1.0) - 10.0) < 0.1);
}

void PlotMeasurementsTests::nonUniformSamples()
{
    const std::vector<double> x{2.0, 0.0, 1.5, 0.2, 0.9};
    const std::vector<double> y{5.0, 1.0, 4.0, 1.4, 2.8};
    const auto samples = PlotMeasurements::normalizedSamples(x, y);
    const auto statistics = PlotMeasurements::calculateIntervalStatistics(samples, 0.0, 2.0);
    QVERIFY(statistics.valid);
    QVERIFY(std::fabs(statistics.mean - 3.0) < 1e-12);
    // RMS follows the specified trapezoidal integration of y squared.
    QVERIFY(std::fabs(statistics.rms - std::sqrt(10.564)) < 1e-12);
}

void PlotMeasurementsTests::invalidIntervals()
{
    const std::vector<double> empty;
    const auto emptySamples = PlotMeasurements::normalizedSamples(empty, empty);
    QVERIFY(emptySamples.empty());
    QVERIFY(!PlotMeasurements::calculateIntervalStatistics(emptySamples, 0.0, 1.0).valid);
    QVERIFY(std::isnan(PlotMeasurements::calculateThdPercent(emptySamples, 0.0, 1.0)));

    const std::vector<double> x{0.0};
    const std::vector<double> y{1.0};
    const auto samples = PlotMeasurements::normalizedSamples(x, y);
    QVERIFY(!PlotMeasurements::calculateIntervalStatistics(samples, 0.0, 0.0).valid);
    QVERIFY(std::isnan(PlotMeasurements::calculateThdPercent(samples, 0.0, 1.0)));

    double value = 0.0;
    QVERIFY(!PlotMeasurements::interpolate(samples, 1.0, &value));

    const std::vector<double> rangeX{0.0, 1.0};
    const std::vector<double> rangeY{0.0, 1.0};
    const auto rangeSamples = PlotMeasurements::normalizedSamples(rangeX, rangeY);
    QVERIFY(!PlotMeasurements::interpolate(rangeSamples, -0.1, &value));
    QVERIFY(!PlotMeasurements::interpolate(rangeSamples, 1.1, &value));

    std::vector<double> zeroX;
    std::vector<double> zeroY;
    for (int i = 0; i <= 128; ++i)
    {
        zeroX.push_back(i / 128.0);
        zeroY.push_back(0.0);
    }
    const auto zeroSamples = PlotMeasurements::normalizedSamples(zeroX, zeroY);
    QVERIFY(std::isnan(PlotMeasurements::calculateThdPercent(zeroSamples, 0.0, 1.0)));
}

void PlotMeasurementsTests::degenerateInputNormalization()
{
    const std::vector<double> x{2.0, 1.0, 1.0, std::numeric_limits<double>::quiet_NaN(), 3.0};
    const std::vector<double> y{20.0, 10.0, 14.0, 99.0};
    const auto samples = PlotMeasurements::normalizedSamples(x, y);
    QCOMPARE(samples.size(), size_t(2));
    QCOMPARE(samples.at(0).x, 1.0);
    QCOMPARE(samples.at(0).y, 12.0);
    QCOMPARE(samples.at(1).x, 2.0);
    QCOMPARE(samples.at(1).y, 20.0);

    double value = 0.0;
    QVERIFY(!PlotMeasurements::interpolate(samples, std::numeric_limits<double>::infinity(), &value));
    QVERIFY(!PlotMeasurements::interpolate(samples, 1.5, nullptr));
    const auto statistics = PlotMeasurements::calculateIntervalStatistics(samples, 2.0, 1.0);
    QVERIFY(statistics.valid);
    QCOMPARE(statistics.mean, 16.0);
}

QTEST_APPLESS_MAIN(PlotMeasurementsTests)

#include "PlotMeasurementsTests.moc"
