#pragma once

#include <vector>

namespace PlotMeasurements
{

struct Sample
{
    double x = 0.0;
    double y = 0.0;
};

struct IntervalStatistics
{
    bool valid = false;
    int count = 0;
    double minimum = 0.0;
    double maximum = 0.0;
    double mean = 0.0;
    double rms = 0.0;
};

std::vector<Sample> normalizedSamples(const std::vector<double> &xData, const std::vector<double> &yData);
bool interpolate(const std::vector<Sample> &samples, double x, double *y);
IntervalStatistics calculateIntervalStatistics(const std::vector<Sample> &samples, double lowerX, double upperX);
double calculateThdPercent(const std::vector<Sample> &samples, double lowerX, double upperX);

}
