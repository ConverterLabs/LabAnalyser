#ifndef MAINWINDOWFIGUREFACTORY_H
#define MAINWINDOWFIGUREFACTORY_H

class MainWindow;
class SubPlotMainWindow;

class MainWindowFigureFactory
{
public:
    static SubPlotMainWindow* Create(MainWindow& mainWindow, int rows, int columns, bool fft);
};

#endif // MAINWINDOWFIGUREFACTORY_H
