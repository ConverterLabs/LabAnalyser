#include "FFTPlotWidget.h"




FFTPlotWidget::FFTPlotWidget( QWidget *parent) :
    QCustomPlot(parent)
{

    setAttribute(Qt::WA_AcceptTouchEvents);

}

FFTPlotWidget::~FFTPlotWidget()
{

}

