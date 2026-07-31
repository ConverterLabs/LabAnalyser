#include "PlotMeasurements.h"

#include <algorithm>
#include <cmath>
#include <limits>

#ifdef LABANALYSER_USE_FFTW
#include <fftw3.h>
#endif

namespace
{

const double kPi = 3.14159265358979323846;

bool isFinite(double value)
{
    return std::isfinite(value);
}

bool intervalSamples(const std::vector<PlotMeasurements::Sample> &samples,
                     double lowerX,
                     double upperX,
                     std::vector<PlotMeasurements::Sample> *interval)
{
    if (!interval || !isFinite(lowerX) || !isFinite(upperX) || lowerX >= upperX)
        return false;

    double lowerY = 0.0;
    double upperY = 0.0;
    if (!PlotMeasurements::interpolate(samples, lowerX, &lowerY) ||
        !PlotMeasurements::interpolate(samples, upperX, &upperY))
        return false;

    interval->clear();
    interval->push_back({lowerX, lowerY});
    for (const PlotMeasurements::Sample &sample : samples)
    {
        if (sample.x > lowerX && sample.x < upperX)
            interval->push_back(sample);
    }
    interval->push_back({upperX, upperY});
    return interval->size() >= 2;
}

int greatestPowerOfTwoNotGreaterThan(int value)
{
    int result = 1;
    while (result <= value / 2)
        result *= 2;
    return result;
}

double notAvailable()
{
    return std::numeric_limits<double>::quiet_NaN();
}

}

namespace PlotMeasurements
{

std::vector<Sample> normalizedSamples(const std::vector<double> &xData, const std::vector<double> &yData)
{
    std::vector<Sample> samples;
    const std::size_t count = std::min(xData.size(), yData.size());
    samples.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        if (isFinite(xData[i]) && isFinite(yData[i]))
            samples.push_back({xData[i], yData[i]});
    }

    std::sort(samples.begin(), samples.end(), [](const Sample &left, const Sample &right) {
        return left.x < right.x;
    });

    std::vector<Sample> uniqueSamples;
    uniqueSamples.reserve(samples.size());
    for (std::size_t i = 0; i < samples.size();)
    {
        const double x = samples[i].x;
        double ySum = 0.0;
        int duplicateCount = 0;
        do
        {
            ySum += samples[i].y;
            ++duplicateCount;
            ++i;
        }
        while (i < samples.size() && samples[i].x == x);

        uniqueSamples.push_back({x, ySum / duplicateCount});
    }

    return uniqueSamples;
}

bool interpolate(const std::vector<Sample> &samples, double x, double *y)
{
    if (!y || samples.empty() || !isFinite(x))
        return false;

    if (samples.size() == 1)
    {
        if (x != samples.front().x)
            return false;
        *y = samples.front().y;
        return true;
    }

    if (x < samples.front().x || x > samples.back().x)
        return false;

    const std::vector<Sample>::const_iterator upper = std::lower_bound(
        samples.begin(), samples.end(), x, [](const Sample &sample, double value) {
            return sample.x < value;
        });

    if (upper == samples.end())
    {
        *y = samples.back().y;
        return true;
    }
    if (upper->x == x || upper == samples.begin())
    {
        *y = upper->y;
        return true;
    }

    const std::vector<Sample>::const_iterator lower = upper - 1;
    const double width = upper->x - lower->x;
    if (width <= 0.0)
        return false;

    *y = lower->y + (x - lower->x) * (upper->y - lower->y) / width;
    return isFinite(*y);
}

IntervalStatistics calculateIntervalStatistics(const std::vector<Sample> &samples, double lowerX, double upperX)
{
    IntervalStatistics statistics;
    if (lowerX > upperX)
        std::swap(lowerX, upperX);

    std::vector<Sample> interval;
    if (!intervalSamples(samples, lowerX, upperX, &interval))
        return statistics;

    double integral = 0.0;
    double squaredIntegral = 0.0;
    statistics.minimum = interval.front().y;
    statistics.maximum = interval.front().y;
    statistics.count = static_cast<int>(interval.size());

    for (std::size_t i = 0; i < interval.size(); ++i)
    {
        statistics.minimum = std::min(statistics.minimum, interval[i].y);
        statistics.maximum = std::max(statistics.maximum, interval[i].y);
        if (i == 0)
            continue;

        const double dx = interval[i].x - interval[i - 1].x;
        integral += 0.5 * (interval[i - 1].y + interval[i].y) * dx;
        squaredIntegral += 0.5 * (interval[i - 1].y * interval[i - 1].y + interval[i].y * interval[i].y) * dx;
    }

    const double duration = upperX - lowerX;
    if (duration <= 0.0)
        return statistics;

    statistics.mean = integral / duration;
    statistics.rms = std::sqrt(std::max(0.0, squaredIntegral / duration));
    statistics.valid = isFinite(statistics.mean) && isFinite(statistics.rms);
    return statistics;
}

double calculateThdPercent(const std::vector<Sample> &samples, double lowerX, double upperX)
{
    if (lowerX > upperX)
        std::swap(lowerX, upperX);

    std::vector<Sample> interval;
    if (!intervalSamples(samples, lowerX, upperX, &interval))
        return notAvailable();

    const double duration = upperX - lowerX;
    if (duration <= 0.0 || interval.size() < 8)
        return notAvailable();

    const int pointCount = greatestPowerOfTwoNotGreaterThan(static_cast<int>(interval.size()) - 1);
    if (pointCount < 8)
        return notAvailable();

    std::vector<double> resampled(pointCount);
    for (int i = 0; i < pointCount; ++i)
    {
        const double x = lowerX + duration * i / pointCount;
        if (!interpolate(interval, x, &resampled[i]))
            return notAvailable();
    }

    const int highestHarmonic = pointCount / 2;
    std::vector<double> magnitudes(highestHarmonic + 1, 0.0);

#ifdef LABANALYSER_USE_FFTW
    fftw_complex *input = static_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * pointCount));
    fftw_complex *output = static_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * pointCount));
    if (!input || !output)
    {
        if (input)
            fftw_free(input);
        if (output)
            fftw_free(output);
        return notAvailable();
    }

    for (int i = 0; i < pointCount; ++i)
    {
        input[i][0] = resampled[i];
        input[i][1] = 0.0;
    }
    fftw_plan plan = fftw_plan_dft_1d(pointCount, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
    if (!plan)
    {
        fftw_free(input);
        fftw_free(output);
        return notAvailable();
    }
    fftw_execute(plan);
    for (int harmonic = 1; harmonic <= highestHarmonic; ++harmonic)
        magnitudes[harmonic] = 2.0 * std::sqrt(output[harmonic][0] * output[harmonic][0] + output[harmonic][1] * output[harmonic][1]) / pointCount;
    fftw_destroy_plan(plan);
    fftw_free(input);
    fftw_free(output);
#else
    for (int harmonic = 1; harmonic <= highestHarmonic; ++harmonic)
    {
        double real = 0.0;
        double imaginary = 0.0;
        for (int sample = 0; sample < pointCount; ++sample)
        {
            const double angle = 2.0 * kPi * harmonic * sample / pointCount;
            real += resampled[sample] * std::cos(angle);
            imaginary -= resampled[sample] * std::sin(angle);
        }
        magnitudes[harmonic] = 2.0 * std::sqrt(real * real + imaginary * imaginary) / pointCount;
    }
#endif

    const double fundamentalRms = magnitudes[1] / std::sqrt(2.0);
    if (!isFinite(fundamentalRms) || fundamentalRms <= std::numeric_limits<double>::epsilon())
        return notAvailable();

    double harmonicRmsSquares = 0.0;
    for (int harmonic = 2; harmonic <= highestHarmonic; ++harmonic)
    {
        const double harmonicRms = magnitudes[harmonic] / std::sqrt(2.0);
        harmonicRmsSquares += harmonicRms * harmonicRms;
    }

    return 100.0 * std::sqrt(harmonicRmsSquares) / fundamentalRms;
}

}
