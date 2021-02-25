/***************************************************************************
**                                                                        **
**  LabAnlyser, a plugin based data modification and visualization tool   **
**  Copyright (C) 2015-2021 Andreas Hoffmann                              **
**                                                                        **
**  LabAnlyser is free software: you can redistribute it and/or modify ´  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
****************************************************************************/

#pragma once

#include <QMainWindow>
#include "qcustomplot.h"
#include <boost\any.hpp>
#include <boost\function.hpp>
#include <boost\bind.hpp>
#include <boost\shared_ptr.hpp>
#include "../DropWidget.h"
#include "../../mainwindow.h"

//This is class is customized for the LabAnalyser GUI

class PlotWidget : public QCustomPlot, public VariantDropWidget
{
    Q_OBJECT

public:
    explicit PlotWidget(MainWindow *MW , QWidget *parent = 0, QStatusBar* SBI = 0);

    void keyPressEvent( QKeyEvent * event );
    void keyReleaseEvent(QKeyEvent * event );
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void AddCustomGraph(QString, bool skip_register = false);
    void ClearAllGraphs();

    void SetXDataName(QString namein){
        ID_X = namein;
    }
    QString  XDataName() const {
        return ID_X;
    }

    void SetVariantData(ToFormMapper Data) override;
    void GetVariantData(ToFormMapper *Data) override;

    bool LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text) override;
    bool SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text) override;
    void ConnectToID(DataManagementSetClass* DM, QString ID) override;



    ~PlotWidget();
protected:
    void closeEvent ( QCloseEvent * event ) override;

public slots:
    void UpdataGraphs(QString ID = NULL);
    void SetAsXAxis();

private slots:
        void titleDoubleClick(QMouseEvent *event, QCPPlotTitle *title);
        void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
        void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
        void selectionChanged();
        void mouseWheel();
        void addRandomGraph();
        void removeSelectedGraph();
        void removeAllGraphs();
        void contextMenuRequest(QPoint pos);
        void moveLegend();
        void graphClicked(QCPAbstractPlottable *plottable);
        void mousePressEventII(QMouseEvent*);
        void mouseReleaseEventII(QMouseEvent*);
        void mouseMoveEventII(QMouseEvent*);
        void SaveToPdf();
        void ResetZoom();
        void AddCustomXAxis(QString id);
        void ToggleMarker();
        void mouseWheelDone();
        bool event( QEvent *event );

private:  
    bool MiddlePressed;
    bool ControlPressed;
bool _release2touch;
bool _touchDevice;
    QCPItemRect* rectZoom;
    MainWindow *MainWindow_p; //Pointer to the mainwindow gui
    QStatusBar* SB; //Pointer to the status bar of the main window
    QString ID_X;
    QElapsedTimer timer;
    int UpdateCounter = 0;
    double Tmin = 0;

};
