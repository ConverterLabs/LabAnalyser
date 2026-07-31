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
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <QHash>
#include <QPoint>
#include <vector>
#include "../DropWidget.h"
#include "../../mainwindow.h"

class QButtonGroup;
class QComboBox;
class QFrame;
class QLabel;
class QResizeEvent;
class QToolButton;
class QTableWidget;

//This is class is customized for the LabAnalyser GUI

class PlotWidget : public QCustomPlot, public VariantDropWidget
{
    Q_OBJECT

public:
    explicit PlotWidget(MainWindow *MW , QWidget *parent = 0, QStatusBar* SBI = 0, bool isFFT = false);

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
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void UpdateGraphs(QString ID = NULL, bool force = false);
    void SetAsXAxis(bool skip =false);

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
        void CalculateFFT();
        void ToggleTimeFreq();
        void ShowQualityCriteria();
        void CalculateQualityCriteria();
        void ToggleCursors(bool visible);
        void ClearScopeCursors();
        void updateCursorItems();
        void updateReadout();
        void setTimeAxisUnit(int index);
        void updateTimeAxisTickLabels();


private:
    enum class PlotToolMode
    {
        Navigate,
        BoxZoom,
        DoubleCursor
    };

    enum class TimeAxisUnit
    {
        Seconds,
        Milliseconds,
        Microseconds
    };

    struct ScopeCursor
    {
        bool active = false;
        double x = 0.0;
    };

    struct GraphDisplayState
    {
        QPen pen;
        QCPGraph::LineStyle lineStyle = QCPGraph::lsLine;
        QCPScatterStyle scatterStyle;
    };

    void initializePlotTools();
    void initializeMeasurementPanel();
    void updateToolboxGeometry();
    void updateMeasurementPanelGeometry();
    void setToolMode(PlotToolMode mode);
    void setCursorsVisible(bool visible);
    void initializeCursorPositions();
    bool cursorMeasurementVisible() const;
    int measurementPanelHeight() const;
    void beginBoxZoom(QMouseEvent *mouse);
    void updateBoxZoomRectangle(const QPoint &position);
    void placeOrMoveCursor(QMouseEvent *mouse);
    QList<QCPGraph*> graphsForReadout() const;
    bool graphData(QCPGraph *graph,
                   boost::shared_ptr<std::vector<double>> &xData,
                   boost::shared_ptr<std::vector<double>> &yData) const;
    bool interpolateGraphValue(QCPGraph *graph, double x, double *y) const;
    double displayXOffset() const;
    QString scopeValueText(double value) const;
    double timeAxisScaleFactor() const;
    QString timeAxisUnitText() const;
    void updateTimeAxisPresentation(bool updateLabel = false);
    void saveTimeDomainGraphStyles();
    void restoreTimeDomainGraphStyles();

    bool MiddlePressed;
    bool ControlPressed;
    QPoint BoxZoomStartPosition;
    Qt::Orientations BoxZoomAxes = Qt::Horizontal | Qt::Vertical;
bool _release2touch;
bool _touchDevice;
    QCPItemRect* rectZoom;
    QCPItemLine *CursorLineA = nullptr;
    QCPItemLine *CursorLineB = nullptr;
    QFrame *PlotToolbox = nullptr;
    QFrame *MeasurementPanel = nullptr;
    QLabel *CursorSummaryLabel = nullptr;
    QTableWidget *MeasurementTable = nullptr;
    QButtonGroup *ScopeToolButtons = nullptr;
    QToolButton *NavigateToolButton = nullptr;
    QToolButton *BoxZoomToolButton = nullptr;
    QToolButton *DoubleCursorToolButton = nullptr;
    QToolButton *CursorsToolButton = nullptr;
    QComboBox *TimeUnitComboBox = nullptr;
    PlotToolMode CurrentToolMode = PlotToolMode::Navigate;
    TimeAxisUnit CurrentTimeAxisUnit = TimeAxisUnit::Seconds;
    ScopeCursor CursorA;
    ScopeCursor CursorB;
    int DraggedCursor = 0;
    bool CursorsVisible = false;
    QHash<QCPGraph*, GraphDisplayState> TimeDomainGraphStyles;
    QCPRange TimeDomainXRange;
    bool HasTimeDomainXRange = false;
    MainWindow *MainWindow_p; //Pointer to the mainwindow gui
    QStatusBar* SB; //Pointer to the status bar of the main window
    QString ID_X;
    bool __isFFT = false;
    bool __ShowQualityCriteria = false;
    std::vector<double> WTHD;
    std::vector<double> THD ;
    std::vector<double> RMS ;
    QCPItemText *QualityCriteriaText = nullptr;
    double f1 = 50.;

    QElapsedTimer timer;
    QElapsedTimer TimeSinceLastPlot;

    int UpdateCounter = 0;
    double Tmin = 0;
    QTimer *UpdateTimer = nullptr;


};
