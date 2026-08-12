#include "MainWindowFigureFactory.h"

#include "mainwindow.h"
#include "DropWidgets/Plots/PlotWidget.h"
#include "UIFunctions/SubPlotMainWindow.h"

#include <QGridLayout>

SubPlotMainWindow* MainWindowFigureFactory::Create(MainWindow& mainWindow, int rows, int columns, bool fft)
{
    SubPlotMainWindow* figure = new SubPlotMainWindow(&mainWindow, &mainWindow);
    QWidget* centralWidget = new QWidget;
    QGridLayout* layout = new QGridLayout(centralWidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setObjectName(QStringLiteral("gridLayout"));

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            PlotWidget* plot = new PlotWidget(&mainWindow, centralWidget, figure->GetStatusBar(), fft);
            const int number = mainWindow.GetLogic()->GetUniquePlotNumber();
            const QString name = "Plot#" + QString::number(number + 1);
            plot->setObjectName(name);
            mainWindow.GetLogic()->AddPlotPointer(name, plot, number);
            layout->addWidget(plot, row, column, 1, 1);
        }
    }

    figure->setCentralWidget(centralWidget);
    figure->resize(600, 400);
    const int number = mainWindow.GetLogic()->GetPlotWindowsIncrementer();
    const QString name = "Figure#" + QString::number(number);
    figure->setObjectName(name);
    mainWindow.GetLogic()->AddPlotWindow(name, rows, columns, number);
    figure->setWindowTitle("Figure " + QString::number(number + 1));
    figure->show();
    return figure;
}
