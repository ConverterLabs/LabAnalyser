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

#include "PlotWidget.h"
#include "PlotWidgetDropBinding.h"
#include "PlotMeasurements.h"
#include "mainwindow.h"
#include <fftw3.h>
#include <QButtonGroup>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QPixmap>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <limits>

#define UseFFTW

#ifdef LABANALYSER_PLOTWIDGET_TEST_SEAMS
namespace PlotWidgetTestHooks
{
namespace
{
bool failFftwAllocation = false;
bool failFftwPlanCreation = false;
}

void setFftwAllocationFailure(bool enabled)
{
    failFftwAllocation = enabled;
}

void setFftwPlanFailure(bool enabled)
{
    failFftwPlanCreation = enabled;
}
}
#endif

namespace
{

fftw_complex *allocateFftwComplexes(std::size_t count)
{
#ifdef LABANALYSER_PLOTWIDGET_TEST_SEAMS
    if (PlotWidgetTestHooks::failFftwAllocation)
        return nullptr;
#endif
    return static_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * count));
}

fftw_plan createFftwPlan(int count, fftw_complex *input, fftw_complex *output)
{
#ifdef LABANALYSER_PLOTWIDGET_TEST_SEAMS
    if (PlotWidgetTestHooks::failFftwPlanCreation)
        return nullptr;
#endif
    return fftw_plan_dft_1d(count, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
}

class FlexibleDoubleSpinBox : public QDoubleSpinBox
{
public:
    using QDoubleSpinBox::QDoubleSpinBox;

protected:
    double valueFromText(const QString &text) const override
    {
        bool ok = false;
        QString normalized = text.trimmed();
        normalized.replace(',', '.');
        const double value = QLocale::c().toDouble(normalized, &ok);
        return ok ? value : this->value();
    }

    QString textFromValue(double value) const override
    {
        QString text = QLocale::c().toString(value, 'f', decimals());
        while (text.contains('.') && text.endsWith('0'))
            text.chop(1);
        if (text.endsWith('.'))
            text.chop(1);
        if (text == "-0")
            text = "0";
        return text;
    }

    QValidator::State validate(QString &text, int &position) const override
    {
        QString normalized = text;
        normalized.replace(',', '.');
        QDoubleValidator validator(minimum(), maximum(), decimals());
        validator.setLocale(QLocale::c());
        return validator.validate(normalized, position);
    }
};

class ScrollableLegend final : public QCPLegend
{
public:
    using QCPLegend::QCPLegend;

    QSize minimumSizeHint() const override
    {
        QSize size = QCPLayoutGrid::minimumSizeHint();
        if (!parentPlot())
            return size;

        const int viewportHeight = parentPlot()->viewport().height();
        const int maximumLegendHeight = qMax(1, viewportHeight * 2 / 3);
        const bool needsScrollBar = size.height() > maximumLegendHeight;
        if (needsScrollBar && scrollBar_)
            size.setWidth(size.width() + scrollBar_->sizeHint().width());
        size.setHeight(qMin(size.height(), maximumLegendHeight));
        return size;
    }

    void setScrollBar(QScrollBar *scrollBar)
    {
        scrollBar_ = scrollBar;
        connect(scrollBar_, &QScrollBar::valueChanged, this, [this](int) {
            if (parentPlot())
                parentPlot()->replot(QCustomPlot::rpQueued);
        });
    }

    void updateLayout() override
    {
        QCPLayoutGrid::updateLayout();

        if (!scrollBar_ || !visible() || itemCount() == 0)
        {
            if (scrollBar_)
                scrollBar_->hide();
            return;
        }

        const QRect viewportRect = rect();
        const int viewportTop = viewportRect.top();
        const int viewportBottom = viewportRect.bottom();
        const int viewportHeight = qMax(0, viewportRect.height());
        if (viewportHeight <= 0)
        {
            scrollBar_->hide();
            return;
        }

        QVector<int> rowHeights;
        rowHeights.reserve(itemCount());
        int contentHeight = 0;
        for (int index = 0; index < itemCount(); ++index)
        {
            const int rowHeight = qMax(1, item(index)->minimumSizeHint().height());
            rowHeights.append(rowHeight);
            contentHeight += rowHeight;
        }
        contentHeight += qMax(0, itemCount() - 1) * rowSpacing();

        const int maximumScroll = qMax(0, contentHeight - viewportHeight);
        {
            const QSignalBlocker blocker(scrollBar_);
            scrollBar_->setRange(0, maximumScroll);
            scrollBar_->setPageStep(viewportHeight);
            scrollBar_->setSingleStep(rowHeights.isEmpty() ? 1 : rowHeights.first() + rowSpacing());
            if (scrollBar_->value() > maximumScroll)
                scrollBar_->setValue(maximumScroll);
        }

        const QRect legendRect = outerRect();
        const int scrollBarWidth = scrollBar_->sizeHint().width();
        scrollBar_->setGeometry(legendRect.right() - scrollBarWidth + 1,
                                legendRect.top(),
                                scrollBarWidth,
                                legendRect.height());
        scrollBar_->setVisible(maximumScroll > 0);
        scrollBar_->raise();

        const int scrollOffset = scrollBar_->value();
        int rowTop = viewportTop - scrollOffset;
        for (int index = 0; index < itemCount(); ++index)
        {
            QCPAbstractLegendItem *legendItem = item(index);
            if (!legendItem)
                continue;

            QRect itemRect = legendItem->outerRect();
            itemRect.moveTop(rowTop);
            itemRect.setHeight(rowHeights.at(index));
            legendItem->setOuterRect(itemRect);

            const bool fullyVisible = itemRect.top() >= viewportTop && itemRect.bottom() <= viewportBottom;
            legendItem->setVisible(fullyVisible);
            rowTop += rowHeights.at(index) + rowSpacing();
        }
    }

private:
    QScrollBar *scrollBar_ = nullptr;
};

QColor legendColorForIndex(int index)
{
    static const QList<QColor> colors = {
        QColor(0, 114, 189),
        QColor(217, 83, 25),
        QColor(237, 177, 32),
        QColor(126, 47, 142),
        QColor(119, 172, 48),
        QColor(77, 190, 238),
        QColor(162, 20, 47),
        QColor(0, 158, 115),
        QColor(230, 159, 0),
        QColor(86, 180, 233),
        QColor(204, 121, 167),
        QColor(0, 0, 0),
        QColor(240, 228, 66),
        QColor(213, 94, 0),
        QColor(0, 114, 178),
        QColor(117, 112, 179),
        QColor(166, 206, 227),
        QColor(255, 127, 0),
        QColor(51, 160, 44),
        QColor(227, 26, 28)
    };

    if (colors.isEmpty())
        return Qt::black;
    const int paletteIndex = ((index % colors.size()) + colors.size()) % colors.size();
    return colors.at(paletteIndex);
}

}

PlotWidget::PlotWidget(MainWindow *MW, QWidget *parent, QStatusBar *SBI, bool isFFT) :
    QCustomPlot(parent)
{

    setAttribute(Qt::WA_AcceptTouchEvents);
      _release2touch = false;
      _touchDevice = false;

    __isFFT = isFFT;


    this->SB = SBI;
    this->MainWindow_p = MW;
    this->MiddlePressed = 0;
    this->ControlPressed = false;
    setAcceptDrops(true);
    rectZoom = new QCPItemRect(this);
    this->addItem(rectZoom);
    rectZoom->setVisible(false);
    rectZoom->setSelectable(false);
    rectZoom->setPen(QPen(QColor(30, 100, 180), 1, Qt::DashLine));
    rectZoom->setBrush(QBrush(QColor(80, 150, 220, 35)));

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setFocusPolicy(Qt::ClickFocus);

   setInteractions( QCP::iRangeDrag |QCP::iRangeZoom | QCP::iSelectAxes |
                                     QCP::iSelectLegend | QCP::iSelectPlottables);//



if(__isFFT)
{
    xAxis->setLabel("f [Hz]");
    yAxis->setLabel("Amp");
    xAxis->setRange(0, 100);
    yAxis->setRange(0, 10);
    axisRect()->setupFullAxesBox();
}
else
{
     xAxis->setRange(-10, 10);
     yAxis->setRange(-10, 10);
    axisRect()->setupFullAxesBox();
    xAxis->setLabel("t [s]");
    yAxis->setLabel("Data");
}

   QFont fontP  = parent->font();

   if(fontP.pointSize() < 10)
       fontP.setPointSize(10);

   QFont NewFont = font();
   NewFont.setStyleStrategy(QFont::PreferAntialias);

   NewFont.setPointSize(fontP.pointSize());

   xAxis->setTickLabelFont(NewFont);
   yAxis->setTickLabelFont(NewFont);

   xAxis->setLabelFont(NewFont);
   yAxis->setLabelFont(NewFont);

   QFont Highlight = xAxis->selectedLabelFont();
   Highlight.setStyleStrategy(QFont::NoAntialias);

   Highlight.setPointSize(fontP.pointSize());
   xAxis->setSelectedLabelFont(Highlight);
   yAxis->setSelectedLabelFont(Highlight);

   QCPLayer *legendLayer = legend->layer();
   axisRect()->insetLayout()->remove(legend);
   auto *scrollableLegend = new ScrollableLegend;
   scrollableLegend->setBrush(QBrush(Qt::white));
   scrollableLegend->setBorderPen(QPen(Qt::black));
   legend = scrollableLegend;
   axisRect()->insetLayout()->addElement(legend, Qt::AlignRight | Qt::AlignTop);
   if (legendLayer)
       scrollableLegend->setLayer(legendLayer);
   else
       scrollableLegend->setLayer("legend");
   LegendScrollBar = new QScrollBar(Qt::Vertical, this);
   LegendScrollBar->setObjectName("LegendScrollBar");
   LegendScrollBar->setFocusPolicy(Qt::NoFocus);
   LegendScrollBar->setStyleSheet(
       "QScrollBar#LegendScrollBar {"
       "  background: rgba(30, 40, 50, 28);"
       "  width: 10px;"
       "  margin: 3px 2px 3px 2px;"
       "  border: none;"
       "  border-radius: 5px;"
       "}"
       "QScrollBar#LegendScrollBar::handle:vertical {"
       "  background: rgb(125, 135, 145);"
       "  min-height: 28px;"
       "  border: 1px solid rgb(105, 115, 125);"
       "  border-radius: 5px;"
       "}"
       "QScrollBar#LegendScrollBar::handle:vertical:hover {"
       "  background: rgb(90, 105, 120);"
       "}"
       "QScrollBar#LegendScrollBar::handle:vertical:pressed {"
       "  background: rgb(65, 80, 95);"
       "}"
       "QScrollBar#LegendScrollBar::add-line:vertical,"
       "QScrollBar#LegendScrollBar::sub-line:vertical {"
       "  height: 0px;"
       "  border: none;"
       "  background: transparent;"
       "}"
       "QScrollBar#LegendScrollBar::add-page:vertical,"
       "QScrollBar#LegendScrollBar::sub-page:vertical {"
       "  background: transparent;"
       "}");
   scrollableLegend->setScrollBar(LegendScrollBar);


   legend->setVisible(true);
   legend->setFont(NewFont);
   legend->setSelectedFont(NewFont);
   legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

   initializeMeasurementPanel();
   initializePlotTools();

   connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressEventII(QMouseEvent*)));
   connect(this, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleaseEventII(QMouseEvent*)));
   connect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoveEventII(QMouseEvent*)));

   connect(this, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

   // make bottom and left axes transfer their ranges to top and right axes:
   connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)),this->xAxis2, SLOT(setRange(QCPRange)));
   connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)),this->yAxis2, SLOT(setRange(QCPRange)));

   // connect some interaction slots:
   connect(this, SIGNAL(titleDoubleClick(QMouseEvent*,QCPPlotTitle*)), this, SLOT(titleDoubleClick(QMouseEvent*,QCPPlotTitle*)));
   connect(this, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));

   //No change of legend allowed
   connect(this, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

   // connect slot that shows a message in the status bar when a graph is clicked:
   connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));

   connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

   connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

   connect(this, SIGNAL(mouseWheelDone()), this, SLOT(mouseWheelDone()));
   connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(updateReadout()));
   connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(updateCursorItems()));
   connect(this->xAxis, SIGNAL(ticksRequest()), this, SLOT(updateTimeAxisTickLabels()));

   timer.start();
   TimeSinceLastPlot.start();

   UpdateTimer = new QTimer(this);
   connect(UpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateGraphs()));
   UpdateTimer->start(25);

   update();

}

void PlotWidget::initializeMeasurementPanel()
{
    MeasurementPanel = new QFrame(this);
    MeasurementPanel->setObjectName("MeasurementPanel");
    MeasurementPanel->setFrameShape(QFrame::StyledPanel);
    MeasurementPanel->setStyleSheet(
        "QFrame#MeasurementPanel { background: palette(base); border-top: 1px solid palette(mid); }"
        "QTableWidget { border: 0; gridline-color: palette(midlight); }");

    QVBoxLayout *panelLayout = new QVBoxLayout(MeasurementPanel);
    panelLayout->setContentsMargins(3, 2, 3, 3);
    panelLayout->setSpacing(1);

    CursorSummaryLabel = new QLabel("Data", MeasurementPanel);
    CursorSummaryLabel->setMinimumHeight(18);
    panelLayout->addWidget(CursorSummaryLabel);

    MeasurementTable = new QTableWidget(MeasurementPanel);
    MeasurementTable->setColumnCount(12);
    MeasurementTable->setHorizontalHeaderLabels({"Name", "Cursor 1", "Cursor 2", "Delta", "1/dT [Hz]", "Slope", "Min", "Max", "Mean", "RMS", "THD", ""});
    MeasurementTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    MeasurementTable->setSelectionMode(QAbstractItemView::NoSelection);
    MeasurementTable->setFocusPolicy(Qt::NoFocus);
    MeasurementTable->setAlternatingRowColors(true);
    MeasurementTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    MeasurementTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    MeasurementTable->verticalHeader()->setVisible(false);
    MeasurementTable->horizontalHeader()->setStretchLastSection(false);
    MeasurementTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    for (int column = 0; column < MeasurementTable->columnCount() - 1; ++column)
        MeasurementTable->horizontalHeader()->setSectionResizeMode(column, QHeaderView::Fixed);
    MeasurementTable->horizontalHeader()->setSectionResizeMode(MeasurementTable->columnCount() - 1, QHeaderView::Stretch);

    const int valueColumnWidth = 86;
    MeasurementTable->setColumnWidth(0, 220);
    for (int column = 1; column < MeasurementTable->columnCount() - 2; ++column)
        MeasurementTable->setColumnWidth(column, valueColumnWidth);
    MeasurementTable->setColumnWidth(MeasurementTable->columnCount() - 2, 76);
    panelLayout->addWidget(MeasurementTable);

    connect(MeasurementTable, &QTableWidget::cellDoubleClicked,
            this, &PlotWidget::measurementCellDoubleClicked);

    MeasurementPanel->hide();
}

void PlotWidget::initializePlotTools()
{
    CursorLineA = new QCPItemLine(this);
    CursorLineB = new QCPItemLine(this);

    CursorLineA->setPen(QPen(QColor(230, 185, 25), 1.5, Qt::DashLine));
    CursorLineB->setPen(QPen(QColor(30, 190, 255), 1.5, Qt::DashLine));
    for (QCPItemLine *cursorLine : { CursorLineA, CursorLineB })
    {
        cursorLine->setSelectable(false);
        cursorLine->setVisible(false);
        cursorLine->start->setType(QCPItemPosition::ptPlotCoords);
        cursorLine->end->setType(QCPItemPosition::ptPlotCoords);
        cursorLine->start->setAxes(xAxis, yAxis);
        cursorLine->end->setAxes(xAxis, yAxis);
    }

    PlotToolbox = new QFrame(this);
    PlotToolbox->setObjectName("PlotToolbox");
    PlotToolbox->setFrameShape(QFrame::StyledPanel);
    PlotToolbox->setStyleSheet(
        "QFrame#PlotToolbox { background: palette(button); border: 0; border-bottom: 1px solid palette(mid); }"
        "QToolButton { padding: 2px 5px; }"
        "QToolButton:checked { background-color: rgba(65, 130, 190, 90); }");

    QHBoxLayout *toolLayout = new QHBoxLayout(PlotToolbox);
    toolLayout->setContentsMargins(3, 3, 3, 3);
    toolLayout->setSpacing(2);
    toolLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    const auto makeToolButton = [this, toolLayout](const QString &iconPath, const QString &toolTip) {
        QToolButton *button = new QToolButton(PlotToolbox);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(16, 16));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setToolTip(toolTip);
        button->setAutoRaise(true);
        toolLayout->addWidget(button);
        return button;
    };

    NavigateToolButton = makeToolButton(":/icons/icons/Pan.png", "Navigate: pan and wheel zoom");
    BoxZoomToolButton = makeToolButton(":/icons/icons/Zoom.png", "Box zoom");
    TimeUnitComboBox = new QComboBox(PlotToolbox);
    TimeUnitComboBox->addItems({"s", "ms", "us"});
    TimeUnitComboBox->setCurrentIndex(static_cast<int>(CurrentTimeAxisUnit));
    TimeUnitComboBox->setFixedWidth(56);
    TimeUnitComboBox->setToolTip("Time axis unit");
    toolLayout->addWidget(TimeUnitComboBox);
    DoubleCursorToolButton = makeToolButton(":/icons/icons/12.png", "Move the nearest cursor");
    CursorsToolButton = makeToolButton(":/icons/icons/Cursors.png", "Show measurement cursors");
    CursorSyncToolButton = makeToolButton(":/icons/icons/sync.png", "Keep the cursor distance synchronized");
    CursorSyncToolButton->setCheckable(true);
    SpectrumToolButton = makeToolButton(":/icons/icons/spectrum-line.png", "Toggle time/frequency domain");
    SpectrumToolButton->setCheckable(true);
    SpectrumToolButton->setChecked(__isFFT);
    QToolButton *clearButton = makeToolButton(":/icons/icons/Clear.png", "Hide cursors");
    QToolButton *resetButton = makeToolButton(":/icons/icons/Reset.png", "Reset zoom");

    ScopeToolButtons = new QButtonGroup(this);
    ScopeToolButtons->setExclusive(true);
    for (QToolButton *button : { NavigateToolButton, BoxZoomToolButton, DoubleCursorToolButton })
    {
        button->setCheckable(true);
        ScopeToolButtons->addButton(button);
    }

    CursorsToolButton->setCheckable(true);
    CursorsToolButton->setChecked(CursorsVisible);

    connect(NavigateToolButton, &QToolButton::clicked, this, [this] { setToolMode(PlotToolMode::Navigate); });
    connect(BoxZoomToolButton, &QToolButton::clicked, this, [this] { setToolMode(PlotToolMode::BoxZoom); });
    connect(DoubleCursorToolButton, &QToolButton::clicked, this, [this] { setToolMode(PlotToolMode::DoubleCursor); });
    connect(TimeUnitComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PlotWidget::setTimeAxisUnit);
    connect(CursorsToolButton, &QToolButton::toggled, this, &PlotWidget::ToggleCursors);
    connect(CursorSyncToolButton, &QToolButton::toggled, this, &PlotWidget::setCursorSyncEnabled);
    connect(SpectrumToolButton, &QToolButton::clicked, this, &PlotWidget::ToggleTimeFreq);
    connect(clearButton, &QToolButton::clicked, this, &PlotWidget::ClearScopeCursors);
    connect(resetButton, &QToolButton::clicked, this, &PlotWidget::ResetZoom);
    toolLayout->addStretch(1);

    setToolMode(PlotToolMode::Navigate);
    updateCursorItems();
    updateReadout();
    updateTimeAxisPresentation(true);
    updateToolboxGeometry();
}

void PlotWidget::updateToolboxGeometry()
{
    if (!PlotToolbox)
        return;

    const int toolboxHeight = qMax(28, PlotToolbox->sizeHint().height() + 4);
    PlotToolbox->setGeometry(0, 0, width(), toolboxHeight);
    PlotToolbox->raise();
}

void PlotWidget::updateLegendGeometry()
{
    if (!legend || !LegendScrollBar)
        return;

    const int maximumHeight = qMax(96, viewport().height() * 2 / 3);
    legend->setMaximumSize(QWIDGETSIZE_MAX, maximumHeight);
    if (!legend->visible())
        LegendScrollBar->hide();
}

int PlotWidget::measurementPanelHeight() const
{
    return qBound(100, height() / 3, 230);
}

bool PlotWidget::cursorMeasurementVisible() const
{
    return CursorsVisible && !__isFFT && CursorA.active;
}

void PlotWidget::updateMeasurementPanelGeometry()
{
    if (!MeasurementPanel)
        return;

    const int toolboxHeight = PlotToolbox ? qMax(28, PlotToolbox->sizeHint().height() + 4) : 32;
    if (!cursorMeasurementVisible())
    {
        MeasurementPanel->hide();
        setViewport(QRect(0, toolboxHeight, width(), qMax(0, height() - toolboxHeight)));
        updateToolboxGeometry();
        updateLegendGeometry();
        return;
    }

    const int panelHeight = measurementPanelHeight();
    const int plotHeight = qMax(0, height() - toolboxHeight - panelHeight);
    setViewport(QRect(0, toolboxHeight, width(), plotHeight));
    MeasurementPanel->setGeometry(0, toolboxHeight + plotHeight, width(), panelHeight);
    MeasurementPanel->show();
    MeasurementPanel->raise();
    updateToolboxGeometry();
    updateLegendGeometry();
}

void PlotWidget::setToolMode(PlotToolMode mode)
{
    CurrentToolMode = mode;
    MiddlePressed = false;
    DraggedCursor = 0;
    rectZoom->setVisible(false);

    QCP::Interactions interactions = QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables;
    if (CurrentToolMode == PlotToolMode::Navigate)
        interactions |= QCP::iRangeDrag | QCP::iRangeZoom;
    setInteractions(interactions);

    QToolButton *selectedButton = NavigateToolButton;
    if (CurrentToolMode == PlotToolMode::BoxZoom)
        selectedButton = BoxZoomToolButton;
    else if (CurrentToolMode == PlotToolMode::DoubleCursor)
        selectedButton = DoubleCursorToolButton;

    if (selectedButton)
        selectedButton->setChecked(true);

    if (CursorsVisible && CurrentToolMode == PlotToolMode::DoubleCursor && !CursorB.active)
    {
        if (!CursorA.active)
            initializeCursorPositions();
        else
        {
            const QCPRange range = xAxis->range();
            CursorB.x = range.lower + 2.0 * range.size() / 3.0;
            CursorB.active = true;
        }
    }

    Qt::CursorShape cursorShape = Qt::ArrowCursor;
    if (CurrentToolMode == PlotToolMode::BoxZoom)
        cursorShape = Qt::CrossCursor;
    else if (CurrentToolMode == PlotToolMode::DoubleCursor)
        cursorShape = Qt::SizeHorCursor;
    setCursor(cursorShape);
    updateCursorItems();
    updateReadout();
}

void PlotWidget::initializeCursorPositions()
{
    const QCPRange range = xAxis->range();
    const double span = range.upper - range.lower;
    CursorA.x = range.lower + span / 3.0;
    CursorB.x = range.lower + 2.0 * span / 3.0;
    CursorA.active = true;
    CursorB.active = true;
}

void PlotWidget::setCursorsVisible(bool visible)
{
    if (visible && !CursorA.active)
        initializeCursorPositions();
    else if (visible && CurrentToolMode == PlotToolMode::DoubleCursor && !CursorB.active)
    {
        const QCPRange range = xAxis->range();
        CursorB.x = range.lower + 2.0 * range.size() / 3.0;
        CursorB.active = true;
    }

    CursorsVisible = visible;
    if (CursorsToolButton && CursorsToolButton->isChecked() != CursorsVisible)
    {
        CursorsToolButton->blockSignals(true);
        CursorsToolButton->setChecked(CursorsVisible);
        CursorsToolButton->blockSignals(false);
    }

    updateCursorItems();
    updateReadout();
    replot(QCustomPlot::rpQueued);
}

void PlotWidget::setCursorSyncEnabled(bool enabled)
{
    if (enabled && !CursorSynced)
    {
        if (!CursorA.active || !CursorB.active)
            enabled = false;
        else
        {
            const double currentDelta = std::fabs(CursorB.x - CursorA.x);
            if (!std::isfinite(currentDelta) || currentDelta <= 0.0)
                enabled = false;
            else
            {
                const double scale = 1000.0;
                QDialog dialog(this);
                dialog.setWindowTitle("Synchronize cursors");
                dialog.setMinimumWidth(280);

                QFormLayout formLayout(&dialog);
                FlexibleDoubleSpinBox deltaInput;
                FlexibleDoubleSpinBox frequencyInput;
                for (QDoubleSpinBox *input : {&deltaInput, &frequencyInput})
                {
                    input->setButtonSymbols(QAbstractSpinBox::NoButtons);
                    input->setMinimumWidth(145);
                    input->setAlignment(Qt::AlignLeft);
                    input->setStyleSheet(
                        "QDoubleSpinBox { padding: 5px 8px; border: 1px solid #9aa7b5; "
                        "border-radius: 4px; background: palette(base); }"
                        "QDoubleSpinBox:focus { border: 2px solid #3d8bfd; padding: 4px 7px; }");
                }
                deltaInput.setRange(std::numeric_limits<double>::min(),
                                    std::numeric_limits<double>::max());
                frequencyInput.setRange(std::numeric_limits<double>::min(),
                                        std::numeric_limits<double>::max());
                deltaInput.setDecimals(12);
                frequencyInput.setDecimals(12);
                const double currentDeltaMs = currentDelta * scale;
                const double threeDecimalDeltaMs = std::floor(currentDeltaMs * 1000.0) / 1000.0;
                const double initialDeltaMs = threeDecimalDeltaMs > 0.0
                        ? threeDecimalDeltaMs
                        : currentDeltaMs;
                deltaInput.setValue(initialDeltaMs);
                frequencyInput.setValue(1.0 / (initialDeltaMs / scale));
                formLayout.addRow("Δt [ms]:", &deltaInput);
                formLayout.addRow("f1 [Hz]:", &frequencyInput);

                QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                formLayout.addRow(&buttons);
                connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
                connect(&deltaInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
                        [&frequencyInput, scale](double value) {
                            if (value > 0.0)
                            {
                                frequencyInput.blockSignals(true);
                                frequencyInput.setValue(1.0 / (value / scale));
                                frequencyInput.blockSignals(false);
                            }
                        });
                connect(&frequencyInput, qOverload<double>(&QDoubleSpinBox::valueChanged),
                        [&deltaInput, scale](double value) {
                            if (value > 0.0)
                            {
                                deltaInput.blockSignals(true);
                                deltaInput.setValue(scale / value);
                                deltaInput.blockSignals(false);
                            }
                        });

                if (dialog.exec() != QDialog::Accepted || deltaInput.value() <= 0.0)
                    enabled = false;
                else
                {
                    CursorSynced = true;
                    setCursorSpacingFromFrequency(1.0 / (deltaInput.value() / scale));
                }
            }
        }
    }

    if (!enabled)
        CursorSynced = false;
    if (CursorSyncToolButton && CursorSyncToolButton->isChecked() != enabled)
    {
        CursorSyncToolButton->blockSignals(true);
        CursorSyncToolButton->setChecked(enabled);
        CursorSyncToolButton->blockSignals(false);
    }
    updateReadout();
}

void PlotWidget::setCursorPosition(int cursor, double x)
{
    if (!CursorSynced || !CursorA.active || !CursorB.active)
    {
        if (cursor == 1)
            CursorA.x = x;
        else if (cursor == 2)
            CursorB.x = x;
        return;
    }

    const double spacing = CursorB.x - CursorA.x;
    const QCPRange range = xAxis->range();
    double newA = cursor == 1 ? x : x - spacing;
    double newB = cursor == 2 ? x : x + spacing;
    const double requiredSpan = std::fabs(spacing);

    if (requiredSpan >= range.size())
    {
        const double center = (newA + newB) / 2.0;
        xAxis->setRange(center - requiredSpan * 0.55, center + requiredSpan * 0.55);
    }
    else
    {
        const double lowerCursor = std::min(newA, newB);
        const double upperCursor = std::max(newA, newB);
        if (lowerCursor < range.lower)
        {
            const double shift = range.lower - lowerCursor;
            newA += shift;
            newB += shift;
        }
        if (upperCursor > range.upper)
        {
            const double shift = range.upper - upperCursor;
            newA += shift;
            newB += shift;
        }
    }

    CursorA.x = newA;
    CursorB.x = newB;
}

void PlotWidget::setCursorSpacingFromFrequency(double frequency)
{
    if (!std::isfinite(frequency) || frequency <= 0.0 || !CursorA.active || !CursorB.active)
        return;

    const double spacing = 1.0 / frequency;
    const double direction = CursorB.x >= CursorA.x ? 1.0 : -1.0;
    const double center = (CursorA.x + CursorB.x) / 2.0;
    CursorA.x = center - direction * spacing / 2.0;
    CursorB.x = center + direction * spacing / 2.0;
    setCursorPosition(1, CursorA.x);
    updateCursorItems();
    updateReadout();
    replot(QCustomPlot::rpQueued);
}

void PlotWidget::measurementCellDoubleClicked(int row, int column)
{
    if (row != 0 || column != 4 || !CursorA.active || !CursorB.active)
        return;

    const double delta = std::fabs(CursorB.x - CursorA.x);
    if (!std::isfinite(delta) || delta <= 0.0)
        return;

    bool accepted = false;
    const double frequency = QInputDialog::getDouble(
        this, "Set cursor frequency", "1/dT [Hz]:", 1.0 / delta,
        std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), 9,
        &accepted);
    if (accepted)
    {
        CursorSynced = true;
        if (CursorSyncToolButton && !CursorSyncToolButton->isChecked())
        {
            CursorSyncToolButton->blockSignals(true);
            CursorSyncToolButton->setChecked(true);
            CursorSyncToolButton->blockSignals(false);
        }
        setCursorSpacingFromFrequency(frequency);
    }
}

void PlotWidget::ToggleCursors(bool visible)
{
    setCursorsVisible(visible);
}

void PlotWidget::beginBoxZoom(QMouseEvent *mouse)
{
    MiddlePressed = true;
    BoxZoomStartPosition = mouse->pos();
    BoxZoomAxes = Qt::Horizontal | Qt::Vertical;
    updateBoxZoomRectangle(BoxZoomStartPosition);
    rectZoom->setVisible(true);
}

void PlotWidget::updateBoxZoomRectangle(const QPoint &position)
{
    constexpr int axisOnlyTolerancePixels = 6;

    const int deltaX = std::abs(position.x() - BoxZoomStartPosition.x());
    const int deltaY = std::abs(position.y() - BoxZoomStartPosition.y());
    if (deltaX <= axisOnlyTolerancePixels && deltaY > axisOnlyTolerancePixels)
        BoxZoomAxes = Qt::Vertical;
    else if (deltaY <= axisOnlyTolerancePixels && deltaX > axisOnlyTolerancePixels)
        BoxZoomAxes = Qt::Horizontal;
    else
        BoxZoomAxes = Qt::Horizontal | Qt::Vertical;

    const double startX = xAxis->pixelToCoord(BoxZoomStartPosition.x());
    const double startY = yAxis->pixelToCoord(BoxZoomStartPosition.y());
    const double endX = xAxis->pixelToCoord(position.x());
    const double endY = yAxis->pixelToCoord(position.y());
    const double selectedXLower = std::min(startX, endX);
    const double selectedXUpper = std::max(startX, endX);
    const double selectedYLower = std::min(startY, endY);
    const double selectedYUpper = std::max(startY, endY);

    if (BoxZoomAxes == Qt::Vertical)
    {
        rectZoom->topLeft->setCoords(xAxis->range().lower, selectedYUpper);
        rectZoom->bottomRight->setCoords(xAxis->range().upper, selectedYLower);
    }
    else if (BoxZoomAxes == Qt::Horizontal)
    {
        rectZoom->topLeft->setCoords(selectedXLower, yAxis->range().upper);
        rectZoom->bottomRight->setCoords(selectedXUpper, yAxis->range().lower);
    }
    else
    {
        rectZoom->topLeft->setCoords(selectedXLower, selectedYUpper);
        rectZoom->bottomRight->setCoords(selectedXUpper, selectedYLower);
    }
}

void PlotWidget::placeOrMoveCursor(QMouseEvent *mouse)
{
    if (!CursorsVisible || __isFFT)
        return;

    const double x = xAxis->pixelToCoord(mouse->pos().x());
    if (CurrentToolMode == PlotToolMode::DoubleCursor)
    {
        if (std::fabs(x - CursorA.x) <= std::fabs(x - CursorB.x))
        {
            setCursorPosition(1, x);
            DraggedCursor = 1;
        }
        else
        {
            setCursorPosition(2, x);
            DraggedCursor = 2;
        }
    }

    updateCursorItems();
    updateReadout();
}

QList<QCPGraph*> PlotWidget::graphsForReadout() const
{
    QList<QCPGraph*> visibleGraphs;
    for (int i = 0; i < graphCount(); ++i)
    {
        QCPGraph *plotGraph = graph(i);
        if (plotGraph && plotGraph->visible())
            visibleGraphs.append(plotGraph);
    }
    return visibleGraphs;
}

bool PlotWidget::graphData(QCPGraph *plotGraph,
                           boost::shared_ptr<std::vector<double>> &xData,
                           boost::shared_ptr<std::vector<double>> &yData) const
{
    if (!plotGraph)
        return false;

    xData = plotGraph->GetXDataPointer();
    yData = plotGraph->GetYDataPointer();
    return xData && yData && !xData->empty() && !yData->empty();
}

double PlotWidget::displayXOffset() const
{
    return (!mXYPlot) ? Tmin : 0.0;
}

double PlotWidget::timeAxisScaleFactor() const
{
    switch (CurrentTimeAxisUnit)
    {
        case TimeAxisUnit::Milliseconds:
            return 1e3;
        case TimeAxisUnit::Microseconds:
            return 1e6;
        case TimeAxisUnit::Seconds:
        default:
            return 1.0;
    }
}

QString PlotWidget::timeAxisUnitText() const
{
    switch (CurrentTimeAxisUnit)
    {
        case TimeAxisUnit::Milliseconds:
            return "ms";
        case TimeAxisUnit::Microseconds:
            return "us";
        case TimeAxisUnit::Seconds:
        default:
            return "s";
    }
}

void PlotWidget::updateTimeAxisPresentation(bool updateLabel)
{
    const bool isTimeDomain = !__isFFT && !mXYPlot;
    if (TimeUnitComboBox)
        TimeUnitComboBox->setEnabled(isTimeDomain);

    xAxis->setAutoTickLabels(!isTimeDomain);
    if (!isTimeDomain)
        return;

    if (updateLabel)
        xAxis->setLabel(QString("t [%1]").arg(timeAxisUnitText()));
    updateTimeAxisTickLabels();
}

void PlotWidget::setTimeAxisUnit(int index)
{
    if (index < static_cast<int>(TimeAxisUnit::Seconds) ||
        index > static_cast<int>(TimeAxisUnit::Microseconds))
        return;

    CurrentTimeAxisUnit = static_cast<TimeAxisUnit>(index);
    updateTimeAxisPresentation(true);
    updateReadout();
    replot(QCustomPlot::rpQueued);
}

void PlotWidget::updateTimeAxisTickLabels()
{
    if (__isFFT || mXYPlot)
        return;

    const QVector<double> ticks = xAxis->tickVector();
    QVector<QString> labels;
    labels.reserve(ticks.size());
    const double scale = timeAxisScaleFactor();
    for (double tick : ticks)
        labels.append(scopeValueText(tick * scale));
    xAxis->setTickVectorLabels(labels);
}

bool PlotWidget::interpolateGraphValue(QCPGraph *plotGraph, double x, double *y) const
{
    boost::shared_ptr<std::vector<double>> xData;
    boost::shared_ptr<std::vector<double>> yData;
    if (!graphData(plotGraph, xData, yData))
        return false;

    const std::vector<PlotMeasurements::Sample> samples = PlotMeasurements::normalizedSamples(*xData, *yData);
    return PlotMeasurements::interpolate(samples, x + displayXOffset(), y);
}

QString PlotWidget::scopeValueText(double value) const
{
    if (!std::isfinite(value))
        return "N/A";

    constexpr int maximumSignificantDigits = 6;
    constexpr int maximumCharacters = 11;
    QString text = QString::number(value, 'g', maximumSignificantDigits);
    if (text.size() > maximumCharacters)
        text = QString::number(value, 'e', maximumSignificantDigits - 3);
    return text;
}

void PlotWidget::updateCursorItems()
{
    const auto updateCursorLine = [this](QCPItemLine *line, const ScopeCursor &cursor) {
        if (!line)
            return;

        line->setVisible(CursorsVisible && !__isFFT && cursor.active);
        if (cursor.active)
        {
            line->start->setCoords(cursor.x, yAxis->range().lower);
            line->end->setCoords(cursor.x, yAxis->range().upper);
        }
    };

    updateCursorLine(CursorLineA, CursorA);
    updateCursorLine(CursorLineB, CursorB);
}

void PlotWidget::updateReadout()
{
    if (!MeasurementPanel || !MeasurementTable || !CursorSummaryLabel)
        return;

    updateMeasurementPanelGeometry();
    if (!cursorMeasurementVisible())
    {
        CursorSummaryLabel->setText("Data");
        MeasurementTable->setRowCount(0);
        return;
    }

    const bool hasSecondCursor = CursorB.active;
    const double dx = hasSecondCursor ? CursorB.x - CursorA.x : std::numeric_limits<double>::quiet_NaN();
    CursorSummaryLabel->setText("Data");

    const QList<QCPGraph*> visibleGraphs = graphsForReadout();
    MeasurementTable->setUpdatesEnabled(false);
    MeasurementTable->clearContents();
    MeasurementTable->setRowCount(visibleGraphs.size() + 1);
    const double dataX1 = CursorA.x + displayXOffset();
    const double dataX2 = CursorB.x + displayXOffset();
    const double lowerDataX = std::min(dataX1, dataX2);
    const double upperDataX = std::max(dataX1, dataX2);

    const auto setValue = [this](int row, int column, double value, const QString &suffix) {
        QTableWidgetItem *item = new QTableWidgetItem(scopeValueText(value) + suffix);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        MeasurementTable->setItem(row, column, item);
    };

    const bool isTimeDomain = !__isFFT && !mXYPlot;
    const double timeScale = isTimeDomain ? timeAxisScaleFactor() : 1.0;
    QTableWidgetItem *timeName = new QTableWidgetItem(isTimeDomain
                                                        ? QString("Time [%1]").arg(timeAxisUnitText())
                                                        : QString("X"));
    MeasurementTable->setItem(0, 0, timeName);
    setValue(0, 1, CursorA.x * timeScale, QString());
    setValue(0, 2, hasSecondCursor ? CursorB.x * timeScale : std::numeric_limits<double>::quiet_NaN(), QString());
    setValue(0, 3, dx * timeScale, QString());
    setValue(0, 4, !std::isfinite(dx) || dx == 0.0 ? std::numeric_limits<double>::quiet_NaN() : 1.0 / std::fabs(dx), QString());

    for (int graphIndex = 0; graphIndex < visibleGraphs.size(); ++graphIndex)
    {
        const int row = graphIndex + 1;
        QCPGraph *plotGraph = visibleGraphs.at(graphIndex);
        const QString graphName = plotGraph->name().isEmpty() ? QString("Graph %1").arg(graphIndex + 1) : plotGraph->name();
        QTableWidgetItem *nameItem = new QTableWidgetItem(graphName);
        QPixmap colorReference(10, 10);
        colorReference.fill(plotGraph->pen().color());
        nameItem->setIcon(QIcon(colorReference));
        MeasurementTable->setItem(row, 0, nameItem);

        boost::shared_ptr<std::vector<double>> xData;
        boost::shared_ptr<std::vector<double>> yData;
        const bool hasData = graphData(plotGraph, xData, yData);
        const std::vector<PlotMeasurements::Sample> samples = hasData
                ? PlotMeasurements::normalizedSamples(*xData, *yData)
                : std::vector<PlotMeasurements::Sample>();

        double y1 = std::numeric_limits<double>::quiet_NaN();
        double y2 = std::numeric_limits<double>::quiet_NaN();
        const bool hasY1 = PlotMeasurements::interpolate(samples, dataX1, &y1);
        const bool hasY2 = hasSecondCursor && PlotMeasurements::interpolate(samples, dataX2, &y2);
        const double dy = hasY1 && hasY2 ? y2 - y1 : std::numeric_limits<double>::quiet_NaN();
        const double slope = std::isfinite(dx) && dx != 0.0 && std::isfinite(dy) ? dy / dx : std::numeric_limits<double>::quiet_NaN();
        const double statisticsLowerX = hasSecondCursor ? lowerDataX : xAxis->range().lower + displayXOffset();
        const double statisticsUpperX = hasSecondCursor ? upperDataX : xAxis->range().upper + displayXOffset();
        const PlotMeasurements::IntervalStatistics statistics = PlotMeasurements::calculateIntervalStatistics(samples, statisticsLowerX, statisticsUpperX);
        const double thd = hasSecondCursor
                ? PlotMeasurements::calculateThdPercent(samples, statisticsLowerX, statisticsUpperX)
                : std::numeric_limits<double>::quiet_NaN();

        setValue(row, 1, y1, QString());
        setValue(row, 2, y2, QString());
        setValue(row, 3, dy, QString());
        setValue(row, 5, slope, QString());
        setValue(row, 6, statistics.valid ? statistics.minimum : std::numeric_limits<double>::quiet_NaN(), QString());
        setValue(row, 7, statistics.valid ? statistics.maximum : std::numeric_limits<double>::quiet_NaN(), QString());
        setValue(row, 8, statistics.valid ? statistics.mean : std::numeric_limits<double>::quiet_NaN(), QString());
        setValue(row, 9, statistics.valid ? statistics.rms : std::numeric_limits<double>::quiet_NaN(), QString());
        setValue(row, 10, thd, std::isfinite(thd) ? QString(" %") : QString());
    }

    MeasurementTable->setUpdatesEnabled(true);
    MeasurementTable->viewport()->update();
}

void PlotWidget::ClearScopeCursors()
{
    DraggedCursor = 0;
    setCursorsVisible(false);
}

void PlotWidget::ClearAllGraphs()
{
    MainWindow *MW = this->MainWindow_p;
    MW->GetLogic()->DeletePlotPointer(this->objectName());

    for(int k = 0; k < graphCount();k++)
    {
         MW->GetLogic()->DeleteEntryOfObject(graph(k)->ID(),this);
    }
}

PlotWidget::~PlotWidget()
{

}

void PlotWidget::closeEvent ( QCloseEvent * event )
{
   Q_UNUSED(event);
   QWidget::closeEvent(event);

  //delete this;
}

void PlotWidget::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
    updateMeasurementPanelGeometry();
}


void PlotWidget::titleDoubleClick(QMouseEvent* event, QCPPlotTitle* title)
{
  Q_UNUSED(event)
  // Set the plot title by double clicking on it
  bool ok;
  QString newTitle = QInputDialog::getText(this, "QCustomPlot example", "New plot title:", QLineEdit::Normal, title->text(), &ok);
  if (ok)
  {
    title->setText(newTitle);
    if(timer.elapsed()>=50)
    {
        replot();
        timer.restart();
    }
        //ui->customPlot[0]->replot();
  }
}

void PlotWidget::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
  // Set an axis label by double clicking on it
  if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
  {
    bool ok;
    QString newLabel = QInputDialog::getText(this, "Plot", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
    if (ok)
    {
      axis->setLabel(newLabel);
      auto SenderOC = QObject::sender();
      QCustomPlot *Sender = qobject_cast<QCustomPlot*>(SenderOC);
      Sender->replot();

     // ui->customPlot[0]->replot();
    }
  }
}

void PlotWidget::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
  // Rename a graph by double clicking on its legend item
  Q_UNUSED(legend)
  if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
  {
    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
    bool ok;
    QString newName = QInputDialog::getText(this, "Set Alias", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
    if (ok)
    {
        MainWindow *MW = this->MainWindow_p;
        MW->GetLogic()->SetAlias(plItem->plottable()->ID(), newName);

      plItem->plottable()->setName(newName);
      auto SenderOC = QObject::sender();
      QCustomPlot *Sender = qobject_cast<QCustomPlot*>(SenderOC);
      Sender->replot();
    }
  }
}

void PlotWidget::selectionChanged()
{
    auto SenderOC = QObject::sender();
    QCustomPlot *Sender = qobject_cast<QCustomPlot*>(SenderOC);
    if (!Sender)
        return;
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (Sender->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || Sender->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      Sender->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || Sender->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    Sender->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    Sender->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (Sender->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || Sender->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      Sender->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || Sender->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    Sender->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    Sender->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }




  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<Sender->graphCount(); ++i)
  {
    QCPGraph *graph = Sender->graph(i);
    QCPPlottableLegendItem *item = Sender->legend->itemWithPlottable(graph);

    if  ((item && item->selected()) || graph->selected())
    {
        if(item)
            item->setSelected(true);
        graph->setSelected(true);
    }
  }

  updateReadout();
}


void PlotWidget::addRandomGraph()
{

     int n =50000; // number of points in graph
     double xScale = (rand()/(double)RAND_MAX + 0.5)*2;
     double yScale = (rand()/(double)RAND_MAX + 0.5)*2;
     double xOffset = (rand()/(double)RAND_MAX - 0.5)*4;
     double yOffset = (rand()/(double)RAND_MAX - 0.5)*5;
     double r1 = (rand()/(double)RAND_MAX - 0.5)*2;
     double r2 = (rand()/(double)RAND_MAX - 0.5)*2;
     double r3 = (rand()/(double)RAND_MAX - 0.5)*2;
     double r4 = (rand()/(double)RAND_MAX - 0.5)*2;


    auto x = new std::vector<double>(n);
    auto y = new std::vector<double>(n);
     boost::shared_ptr<std::vector<double>> xp(x);
     boost::shared_ptr<std::vector<double>> yp(y);

     for (int i=0; i<n; i++)
     {
       xp->at(i) = (i/(double)n-0.5)*10.0*xScale + xOffset;
       yp->at(i) = (qSin(xp->at(i)*r1*5)*qSin(qCos(xp->at(i)*r2)*r4*3)+r3*qCos(qSin(xp->at(i))*r4*2))*yScale + yOffset + (rand()/(double)RAND_MAX - 0.5)*0;
     }

     addGraph();
     graph()->setAdaptiveSampling(true);
     graph()->setName(QString("New graph %1").arg(graphCount()-1));

    graph()->setData(xp, yp, 0.0);
    QPen graphPen;
    graphPen.setColor(QColor(rand()%245+10, rand()%245+10, rand()%245+10));
    graphPen.setWidthF(1);
    graph()->setPen(graphPen);
    ResetZoom();

     //replot();
}

void PlotWidget::removeSelectedGraph()
{
  if (selectedGraphs().size() > 0)
  {
      for(int k = 0; k < selectedGraphs().size();k++)
      {
          QCPGraph *selectedGraph = selectedGraphs().at(k);
          this->MainWindow_p->GetLogic()->DeleteEntryOfObject(selectedGraph->ID(),this);
          TimeDomainGraphStyles.remove(selectedGraph);
          removeGraph(selectedGraph);
      }
    replot();
  }
  if(!selectedGraphs().size())
    {
      SetXYPlot(false);
      legend->setVisible(true);
        if(__isFFT)
        {
            xAxis->setLabel("f [Hz]");
            yAxis->setLabel("Amplitude");
        }
        else
        {
            updateTimeAxisPresentation(true);
            yAxis->setLabel("Data");
        }
      ID_X.clear();
  }
  if(graphCount() == 0)
  {
      if(QualityCriteriaText)
      {
        QualityCriteriaText->deleteLater();
        QualityCriteriaText = nullptr;
      }
  }

  updateReadout();
  replot(QCustomPlot::rpQueued);
}


void PlotWidget::SetAsXAxis(bool skip)
{
    if (selectedGraphs().size() ==1)
    {
        for(int i = 0; i < graphCount();i++)
        {
            if(graph(i) != selectedGraphs().at(0))
            {
                auto Y = graph(i)->GetYDataPointer();
                auto X = selectedGraphs().at(0)->GetYDataPointer();
                if(X != NULL)
                    if(Y != NULL)
                        if(X->size()==Y->size())
                            graph(i)->setData(X,Y, 0.0);
                graph(i)->SetXYPlot(true);
                selectedGraphs().at(0)->setVisible(false);
                this->ID_X = selectedGraphs().at(0)->ID();

                this->legend->setVisible(false);
                if(!skip)
                {
                    xAxis->setLabel(selectedGraphs().at(0)->name());
                    yAxis->setLabel(graph(i)->name());
                }
                SetXYPlot(true);
                updateTimeAxisPresentation();

                if(!skip)
                    ResetZoom();
            }
        }
    }

    updateReadout();
    replot(QCustomPlot::rpQueued);
}

void PlotWidget::ToggleMarker()
{
  if (selectedGraphs().size() > 0)
  {
      for(int k = 0; k < selectedGraphs().size();k++)
      {
          if(selectedGraphs().at(k)->scatterStyle().isNone())
              selectedGraphs().at(k)->setScatterStyle(QCPScatterStyle::ssCross);
          else
              selectedGraphs().at(k)->setScatterStyle(QCPScatterStyle::ssNone);
             // 1;
          //selectedGraphs().at(k)->setScatterStyle();
      }
    replot();
  }
}

void PlotWidget::removeAllGraphs()
{
    MainWindow *MW = this->MainWindow_p;
    for(int k = 0; k < graphCount();k++)
    {
        MW->GetLogic()->DeleteEntryOfObject(graph(k)->ID(),this);
    }

  clearGraphs();
  TimeDomainGraphStyles.clear();

  if(!selectedGraphs().size())
  {
    SetXYPlot(false);
    legend->setVisible(true);
    if(__isFFT)
    {
        xAxis->setLabel("f [Hz]");
        yAxis->setLabel("Amplitude");
    }
    else
    {
        updateTimeAxisPresentation(true);
        yAxis->setLabel("Data");
    }
    ID_X.clear();

  }
  if(QualityCriteriaText)
  {
      QualityCriteriaText->deleteLater();
      QualityCriteriaText = nullptr;
  }

  updateReadout();
  replot();



}

void PlotWidget::contextMenuRequest(QPoint pos)
{

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

if (graphCount() > 0)
{
  if (legend->selectTest(pos, false) >= 0 && !selectedGraphs().size()) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {
    if (selectedGraphs().size() > 0)
    {
        menu->addAction("Toggle Marker", this, SLOT(ToggleMarker()));
        menu->addSeparator();
        QAction *Highlight = new QAction;
        connect(Highlight, &QAction::triggered, [=]{
            MainWindow_p->HighLightConnection(selectedGraphs().at(0)->ID());});
        Highlight->setText("Highlight Connection");
        menu->addAction(Highlight);

        menu->addSeparator();
        menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    }
    else
        menu->addSeparator();

      menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
      menu->addSeparator();
      menu->addAction("Save to Pdf", this, SLOT(SaveToPdf()));
      menu->addSeparator();
      if (graphCount() == 2 && !__isFFT)
      {
        menu->addAction("Set As X-Axis", this, SLOT(SetAsXAxis()));
        menu->addSeparator();
      }
      if(!mXYPlot)
      {
        menu->addAction("Toggle Time/Frequency Domain", this, SLOT(ToggleTimeFreq()));
        menu->addSeparator();
      }
      QAction *showCursorsAction = menu->addAction("Show Cursor Data");
      showCursorsAction->setCheckable(true);
      showCursorsAction->setChecked(CursorsVisible);
      connect(showCursorsAction, &QAction::toggled, this, &PlotWidget::ToggleCursors);
      menu->addSeparator();
      menu->addAction("Update Data", this, [this]() { UpdateGraphs("", true); });
      menu->addAction("Reset Zoom", this, SLOT(ResetZoom()));
    }


}
 menu->popup(mapToGlobal(pos));
// menu->resize(menu->size().width(), menu->size().height()*1.1); //Bugfix QT6 and DPI Scaling

}

void PlotWidget::moveLegend()
{

  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
      replot();
    }
  }
}

void PlotWidget::graphClicked(QCPAbstractPlottable *plottable)
{
    Q_UNUSED(plottable);
    if(this->SB)
    this->SB->showMessage(QString("Clicked on graph '%1'.").arg(plottable->name()), 1000);
}



void PlotWidget::SaveToPdf(void)
{

  // Open a file dialog for the user to choose a filename for the PDF


    // The native dialog always causes a segfault. Dont know why so we use QFileDialog::DontUseNativeDialog
  auto filename =  QFileDialog::getSaveFileName(MainWindow_p, "Save file", "plot.pdf", "*.pdf", 0,QFileDialog::DontUseNativeDialog);

  // If the user selected a filename
  if(filename.size())
  {
    // Create a QFile object for the selected filename
    QFile file (filename);

    // If the file already exists
    if(file.exists())
    {
      // Attempt to remove the file
      if(file.remove() == 0)
      {
          MainWindow_p->ErrorWriter("QCustomPlot PDF Writer", QString("Unable to remove file!"));
          return;
      }
    }
    // If the file was successfully removed, save the PDF with the selected filename
    SetBlockUpdates();
    savePdf(filename);
    UnblockUpdates();
  }

}

void PlotWidget::ShowQualityCriteria(void)
{
    if(__ShowQualityCriteria)
    {
        __ShowQualityCriteria = false;
        if (QualityCriteriaText)
            QualityCriteriaText->deleteLater();
        QualityCriteriaText = nullptr;
    }
    else
    {
        // User Dialog to input fundamental frequency
        bool ok;
        double f0 = QInputDialog::getDouble(this, tr("Fundamental Frequency"),
                                            tr("Fundamental Frequency [Hz]:"), f1, 0, 10000, 2, &ok);
        if (!ok)
            return;
        
        f1 = f0;
        __ShowQualityCriteria = true;
    }
}

void PlotWidget::saveTimeDomainGraphStyles()
{
    TimeDomainGraphStyles.clear();
    for (int i = 0; i < graphCount(); ++i)
    {
        QCPGraph *plotGraph = graph(i);
        if (!plotGraph)
            continue;

        GraphDisplayState state;
        state.pen = plotGraph->pen();
        state.lineStyle = plotGraph->lineStyle();
        state.scatterStyle = plotGraph->scatterStyle();
        TimeDomainGraphStyles.insert(plotGraph, state);
    }
}

void PlotWidget::restoreTimeDomainGraphStyles()
{
    for (int i = 0; i < graphCount(); ++i)
    {
        QCPGraph *plotGraph = graph(i);
        const QHash<QCPGraph*, GraphDisplayState>::const_iterator style = TimeDomainGraphStyles.constFind(plotGraph);
        if (!plotGraph || style == TimeDomainGraphStyles.constEnd())
            continue;

        plotGraph->setPen(style->pen);
        plotGraph->setLineStyle(style->lineStyle);
        plotGraph->setScatterStyle(style->scatterStyle);
    }
    TimeDomainGraphStyles.clear();
}

void PlotWidget::ToggleTimeFreq(void)
{
    if (__isFFT)
    {
        __ShowQualityCriteria = false;
        __isFFT = false;
        updateTimeAxisPresentation(true);
        yAxis->setLabel("Data");
        restoreTimeDomainGraphStyles();

        ResetZoom();
        if (HasTimeDomainXRange)
            xAxis->setRange(TimeDomainXRange);
    }
    else
    {
        TimeDomainXRange = xAxis->range();
        HasTimeDomainXRange = true;
        saveTimeDomainGraphStyles();
        CalculateFFT();
        __isFFT = true;
        updateTimeAxisPresentation();
        xAxis->setLabel("f [Hz]");
        yAxis->setLabel("Amplitude");

        for (int i = 0; i < graphCount(); ++i)
        {
            graph(i)->setLineStyle(QCPGraph::lsImpulse);
            graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, Qt::transparent, 10));
        }

        ResetZoom();
        xAxis->setRange(0, 500);
    }

    updateCursorItems();
    updateReadout();
    if (SpectrumToolButton && SpectrumToolButton->isChecked() != __isFFT)
    {
        SpectrumToolButton->blockSignals(true);
        SpectrumToolButton->setChecked(__isFFT);
        SpectrumToolButton->blockSignals(false);
    }
    replot();
}
void PlotWidget::UpdateGraphs(QString ID, bool force)
{
    if (IsUpdateBlocked())
        return;

    DataPair XData, YData;
    int yIndex = 0;

    if (!force)
    {
        // Update 20 ms after the last ID arrived
        if (!ID.isEmpty())
            timer.restart();

        if (timer.elapsed() <= 25)
            return;

        if (TimeSinceLastPlot.elapsed() < 75)
            return;

        TimeSinceLastPlot.restart();

        if (XYPlot())
        {
            // Retrieve pointers to X and Y data
            uint32_t foundElements = 0;
            for (int i = 0; i < graphCount(); ++i)
            {
                ToFormMapper* element = MainWindow_p->GetLogic()->GetContainer(graph(i)->ID());
                if (!element)
                    continue;

                if (graph(i)->ID().compare(ID_X))
                {
                    YData = element->GetPointerPair();
                    yIndex = i;
                }
                else
                {
                    XData = element->GetPointerPair();
                }
            }

            // Validate data consistency
            if (!XData.first || !XData.second || !YData.first || !YData.second)
                return;

            if (XData.first->empty() || XData.second->empty() ||
                YData.first->empty() || YData.second->empty())
                return;

            if (XData.second->size() != YData.second->size() ||
                XData.first->size() != YData.first->size())
                return;

            // Check matching first and last elements
            if (XData.first->at(0) != YData.first->at(0))
                return;

            if (XData.first->back() != YData.first->back())
                return;

            // Verify delta of first two elements
            if (XData.first->size() > 2)
            {
                double deltaX = XData.first->at(1) - XData.first->at(0);
                double deltaY = YData.first->at(1) - YData.first->at(0);
                if (std::fabs(deltaX - deltaY) > 1e-9)
                    return;
            }
        }
    }

    // Determine Tmin across all graphs
    for (int i = 0; i < graphCount(); ++i)
    {
        ToFormMapper* element = MainWindow_p->GetLogic()->GetContainer(graph(i)->ID());
        if (!element)
            continue;

        DataPair dp = element->GetPointerPair();
        if (!dp.first || !dp.second || dp.first->empty() || dp.second->empty())
            continue;

        if (i == 0)
            Tmin = *(dp.third);
        else if (*(dp.third) < Tmin)
            Tmin = *(dp.third);
    }

    // Update data for each graph
    for (int i = 0; i < graphCount(); ++i)
    {
        ToFormMapper* element = MainWindow_p->GetLogic()->GetContainer(graph(i)->ID());
        if (!element)
            continue;

        if (!XYPlot())
        {
            graph(i)->setData(element->GetPointerPair().first, element->GetPointerPair().second, Tmin);
        }
        else
        {
            if (graph(i)->ID().compare(ID_X))
            {
                YData = element->GetPointerPair();
                yIndex = i;
            }
            else
            {
                XData = element->GetPointerPair();
            }
        }
    }

    // XY plot update
    if (XYPlot())
    {
        if (XData.second && YData.second && XData.second->size() == YData.second->size())
            graph(yIndex)->setData(XData.second, YData.second, 0.0);
    }
    else if (__isFFT)
    {
        CalculateFFT();
    }

    updateReadout();

    // Check if visible and not minimized before replotting
    QWidget* parent = this->parentWidget();
    bool minimized = false;
    while (parent->parentWidget())
    {
        if (parent->isMinimized() || MainWindow_p->isMinimized())
            minimized = true;
        parent = parent->parentWidget();
    }

    if (!visibleRegion().isEmpty() && !minimized)
    {
        bool sameSize = true;
        for (int i = 0; i < graphCount() - 2; ++i)
        {
            if (graph(i)->GetXDataPointer() && graph(i + 1)->GetXDataPointer())
            {
                sameSize &= (graph(i)->GetXDataPointer()->size() ==
                             graph(i + 1)->GetXDataPointer()->size());
            }
        }

        if (sameSize || XYPlot())
            replot();
    }

    // Optional FFT and quality calculation
    if (__ShowQualityCriteria)
    {
        if (!__isFFT)
            CalculateFFT();

        CalculateQualityCriteria();
    }

    timer.restart();
}


void PlotWidget::CalculateQualityCriteria()
{

    //Calculate THD and WTHD
    //get the data
    //iterate over all graphs
    THD.clear();
    WTHD.clear();
    RMS.clear();

    for(int i = 0; i < graphCount(); i++)
    {
        QCPGraph *plotGraph = graph(i);
        if (!plotGraph)
            continue;

        auto f = plotGraph->GetXFFTPointer();
        auto a = plotGraph->GetYFFTPointer();
        const int sampleCount = f && a ? static_cast<int>(std::min(f->size(), a->size())) : 0;
        if(sampleCount > 0)
        {
            //find nearest element in f to f1
            auto f1_l =  std::distance(f->begin(), std::lower_bound(f->begin(), f->end(), f1-5));
            auto f1_u =  std::distance(f->begin(), std::upper_bound(f->begin(), f->end(), f1+5));


            double U1 = 0;
            int f1m = std::min(sampleCount - 1, std::max(0, static_cast<int>(std::round((f1_l+f1_u)/2))));
            int delta = std::max(0, static_cast<int>(std::round(f1_u-f1_l)/2));

            if (f1m <= 0)
                continue;

            const int fundamentalFirst = std::max(0, f1m-delta);
            const int fundamentalLast = std::min(sampleCount, f1m+delta);
            for( int i = fundamentalFirst; i < fundamentalLast; i++)
            {
                U1 += a->at(i);
            }

            auto U1sqare = U1*U1;
            if (!std::isfinite(U1sqare) || U1sqare <= std::numeric_limits<double>::epsilon())
                continue;

            //calculate THD
            double _THD = 0;
            double _WTHD = 0;
            double _RMS = 0;
            //iterate over all
            for( int i = 0; i < sampleCount; i++)
            {
                _RMS += (a->at(i)*a->at(i));
             }
            int h = 2;
            for( int i = 2*f1m; i < sampleCount; i = i + f1m)
            {
                const int harmonicFirst = std::max(0, i-delta);
                const int harmonicLast = std::min(sampleCount, i+delta);
                for(int j = harmonicFirst; j < harmonicLast; j++)
                {
                    _THD += (a->at(i)*a->at(i))/U1sqare;
                    _WTHD += a->at(i) * a->at(i)/(U1sqare*h*h) ;
                }
               

                h++;
            }

            WTHD.push_back(sqrt(_WTHD)*100.0);
            THD.push_back(sqrt(_THD)*100.0);
            RMS.push_back(sqrt(1./2.*_RMS));
            }

    }

    // add the text label at the top:
    if(QualityCriteriaText == nullptr)
    {
        

        QualityCriteriaText = new QCPItemText(this);
        QualityCriteriaText->setPositionAlignment(Qt::AlignBottom|Qt::AlignRight);
        QualityCriteriaText->position->setType(QCPItemPosition::ptAxisRectRatio);
        QualityCriteriaText->position->setCoords(1, 1); // place position at center/top of axis rect
        // incease padding
        QualityCriteriaText->setPadding(QMargins(8, 0, 8, 0));
        QualityCriteriaText->setFont(QFont(font().family(), 12)); // make font a bit larger
        QualityCriteriaText->setPen(QPen(Qt::black)); // show black border around text
        // set background color to white:
        QualityCriteriaText->setBrush(QBrush(QColor(255, 255, 255, 200)));

    }
    //output WTHD   and THD of all graphs
    QString text;
    for(int i = 0; i < THD.size(); i++)
    {
        text += "RMS: " + QString::number(RMS[i], 'f', 2) + " WTHD: " + QString::number(WTHD[i], 'f', 2) + " % " + " THD: " + QString::number(THD[i], 'f', 2) + " %" ;
        if(i < THD.size()-1)
            text += "\n";
    }
    QualityCriteriaText->setText(text);




}

void PlotWidget::CalculateFFT()
{


#ifndef UseFFTW
    boost::shared_ptr<std::vector<double>> x(graph(0)->GetXDataPointer());
    boost::shared_ptr<std::vector<double>> y(graph(0)->GetYDataPointer());

    if(!x || !y)
        return;

    if(x->size() != y->size())
        return;

    if(x->size() == 0)
        return;

    boost::shared_ptr<std::vector<double>> fft_x = boost::make_shared<std::vector<double>>();
    boost::shared_ptr<std::vector<double>> fft_y = boost::make_shared<std::vector<double>>();



    for(int i = 0 ; i < __Nmax; i++)
    {
        std::vector<double> fft_y_s ;
        std::vector<double> fft_y_c ;
        //calculate sine vector
        fft_x->push_back(i*__f1);
        fft_x->push_back(i*__f1);
        double t0 = x->at(0);
        for(int j = 0; j < x->size(); j++)
        {
            //qDebug() << "x: " << x->at(j)-t0 << " y: " << y->at(j) << "2.0*M_PI*fft_x->back()" << 2.0*M_PI*fft_x->back()  ;
            fft_y_s.push_back(sin(2.0*M_PI*fft_x->back()*(x->at(j)-t0))*y->at(j));
            fft_y_c.push_back(cos(2.0*M_PI*fft_x->back()*(x->at(j)-t0))*y->at(j));
        }
        //calculate intergral of fft_y_s over time x using trapezoidal rule
        double sum_s = 0.0;
        double sum_c = 0.0;
        for(int j = 0; j < x->size()-1; j++)
        {
           sum_s += (fft_y_s.at(j+1)+fft_y_s.at(j))/2.*(x->at(j+1)-x->at(j));
           sum_c += (fft_y_c.at(j+1)+fft_y_c.at(j))/2.*(x->at(j+1)-x->at(j));
        }
        //window length
        double WL = x->back()-x->front();
        //Periodendauer
        double T = 1.0/(i*__f1);
        if(i>0)
        {
            fft_y->push_back(0);
            fft_y->push_back(2*sqrt(sum_c*sum_c + sum_s*sum_s)/(WL));
        }
        else
        {
            fft_y->push_back(0);
            fft_y->push_back(2*sqrt(sum_c*sum_c));
        }

    }
#else

    //for all graphs
    //number of graphs
    int N = graphCount();
    for(int j = 0; j < graphCount(); j++)
    {
        QCPGraph *currentGraph = graph(j);
        if (!currentGraph)
            continue;

        boost::shared_ptr<std::vector<double>> x(currentGraph->GetXDataPointer());
        boost::shared_ptr<std::vector<double>> y(currentGraph->GetYDataPointer());

        if(!x || !y)
            return;

        if(x->size() != y->size())
            return;

        if(x->size() < 2)
            return;

        boost::shared_ptr<std::vector<double>> fft_x = boost::make_shared<std::vector<double>>();
        boost::shared_ptr<std::vector<double>> fft_y = boost::make_shared<std::vector<double>>();


        fftw_complex *in, *out;
        fftw_plan p;
        in = allocateFftwComplexes(x->size());
        if (!in)
            return;
        out = allocateFftwComplexes(x->size());
        if (!out)
        {
            fftw_free(in);
            return;
        }
        for(int i = 0; i < x->size(); i++)
        {
            in[i][0] = y->at(i);
            in[i][1] = 0;
        }
        p = createFftwPlan(x->size(), in, out);
        if (!p)
        {
            fftw_free(in);
            fftw_free(out);
            return;
        }
        fftw_execute(p); /* repeat as needed */

        //Calculate Ts as mean of time difference
        std::vector<double> dTs;
        for(int i = 0; i < x->size()-1; i++)
            dTs.push_back(x->at(i+1)-x->at(i));

        double Ts = std::accumulate(dTs.begin(), dTs.end(), 0.0)/dTs.size();
        auto fsN = 1.0/Ts/x->size();

        for(int i = 0; i < x->size()/2 +1; i++)
        {
            fft_x->push_back(i*fsN);
            fft_x->push_back(i*fsN);
            fft_y->push_back(0);
            if(i>0)
                fft_y->push_back(2*sqrt(out[i][0]*out[i][0]+out[i][1]*out[i][1])/x->size());
            else
                fft_y->push_back(sqrt(out[i][0]*out[i][0]+out[i][1]*out[i][1])/x->size());
        }
        fftw_destroy_plan(p);
        fftw_free(in); fftw_free(out);
        //set data
        graph(j)->setFFTData(fft_x,fft_y);
    }
#endif


}

void PlotWidget::ResetZoom()
{
    if(!graphCount())
        return;


    boost::shared_ptr<std::vector<double>> x(graph(0)->GetXDataPointer());
    boost::shared_ptr<std::vector<double>> y(graph(0)->GetYDataPointer());

    if(__isFFT)
    {
        x = graph(0)->GetXFFTPointer();
        y = graph(0)->GetYFFTPointer();
    }

    if(XYPlot() )
    {
        if(graphCount() != 2)
            return;
        if( graph(1)->ID() == XDataName())
        {
            x = graph(0)->GetXDataPointer();
            y = graph(0)->GetYDataPointer();
        }
        else
        {
            x = graph(1)->GetXDataPointer();
            y = graph(1)->GetYDataPointer();
        }
        volatile double xmin = *(std::min_element(x->begin(),x->end())); //(l.begin(), l.end(
        volatile double xmax = *(std::max_element(x->begin(),x->end()));

        volatile double ymin = *(std::min_element(y->begin(),y->end())); //(l.begin(), l.end(
        volatile double ymax = *(std::max_element(y->begin(),y->end()));
        xAxis->setRange(xmin-fabs(xmax-xmin)*0.05, xmax+fabs(xmax-xmin)*0.05);
        yAxis->setRange(ymin-fabs(ymax-ymin)*0.05, ymax+fabs(ymax-ymin)*0.05);
        updateReadout();
        replot();
        return;
    }

    if(!x)
        return;
    if(! x->size())
        return;
    if(!y)
        return;
    if(! y->size())
        return;

    double ymin = *(std::min_element(y->begin(),y->end())); //(l.begin(), l.end(
    double ymax = *(std::max_element(y->begin(),y->end()));

    for(int i = 1; i< graphCount();i++)
    {
        boost::shared_ptr<std::vector<double>> y(__isFFT
                                                  ? graph(i)->GetYFFTPointer()
                                                  : graph(i)->GetYDataPointer());

        if(!y)
            break;
        if(! y->size())
            break;

        double ymint = *(std::min_element(y->begin(),y->end())); //(l.begin(), l.end(
        double ymaxt = *(std::max_element(y->begin(),y->end()));
        if(ymint<ymin)
            ymin = ymint;
        if(ymaxt>ymax)
           ymax = ymaxt;
    }

    if(XYPlot())
    {
        double xmin = *(std::min_element(x->begin(),x->end())); //(l.begin(), l.end(
        double xmax = *(std::max_element(x->begin(),x->end()));
        xAxis->setRange(xmin,xmax);

    }
    else
    {
        const double xOffset = __isFFT ? 0.0 : Tmin;
        xAxis->setRange(x->front() - xOffset, x->back() - xOffset);
}
    if(ymin == ymax)
    {
        if(ymax==0)
            yAxis->setRange(-0.1, 0.1);
        else
            yAxis->setRange(ymin-fabs(ymax)*0.1, ymax+fabs(ymax)*0.1);
    }
    else
        yAxis->setRange(ymin-fabs(ymax-ymin)*0.05, ymax+fabs(ymax-ymin)*0.05);

    updateReadout();
    replot();
}

void PlotWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget || XYPlot() || !MainWindow_p || !MainWindow_p->GetLogic())
    {
        return;
    }
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
    if (selectedItems.isEmpty())
        return;

    QString id;
    if (PlotWidgetDropBinding::ResolveSupportedItem(MainWindow_p->GetLogic(),
                                                    selectedItems.first(), &id))
        event->acceptProposedAction();

}

void PlotWidget::AddCustomXAxis(QString id)
{
    MainWindow *MW = this->MainWindow_p;
    MW->GetLogic()->AddElementToContainerEntry(this->objectName(),id,this->metaObject()->className(),this);
    DataPair DP = MW->GetLogic()->GetContainer(id)->GetPointerPair();
    if(DP.first)
    {
        ToFormMapper* Y = MainWindow_p->GetLogic()->GetContainer(graph(0)->name());
        if(Y)
        {
            if(Y->GetPointerPair().second->size() == DP.second->size())
            {
                ID_X = id;
            }
        }
    }

}

void PlotWidget::AddCustomGraph(QString id, bool skip_register)
{
    MainWindow *MW = this->MainWindow_p;
    if(!skip_register)
        MW->GetLogic()->AddElementToContainerEntry(this->objectName(),id,this->metaObject()->className(),this);

    if(!(MW->GetLogic()->GetContainer(id)))
        return;



    DataPair DP = MW->GetLogic()->GetContainer(id)->GetPointerPair();
    if(DP.first && DP.second)
    if(graphCount() == 0 &&  DP.first->size())
    {
        Tmin = *(DP.third);
    }

    addGraph();
    graph()->setID(id);
    graph()->setName(MW->GetLogic()->GetAlias(id));
    QPen graphPen;

    if(__isFFT)
    {
        graph()->setLineStyle(QCPGraph::lsImpulse);
        constexpr qreal lineWidth{ 3 };
        constexpr qreal plotScatterSize{ 10 };
        graph()->setPen(QPen{ QBrush{ Qt::black }, lineWidth });
        graph()->setScatterStyle(
                QCPScatterStyle{ QCPScatterStyle::ssCross, Qt::transparent, plotScatterSize });


        graph()->keyAxis()->setTickLengthIn(0);
        graph()->keyAxis()->setSubTickLengthIn(0);
        graphPen.setWidthF(5);

    }
    else
        graphPen.setWidthF(1);


    if(DP.first && DP.second)
    {
        graph()->setData(DP.first,DP.second, Tmin);
        if(__isFFT)
        {
            CalculateFFT();
        }

    }







   // if(DP.first->size())
    {

        switch (graphCount()) //MatlabColors
        {
            case 1: graphPen.setColor( QColor(0  ,  114  ,  189)); break;
            case 2: graphPen.setColor( QColor(217  ,  83  ,  25)); break;
            case 3: graphPen.setColor( QColor(237  , 177   , 32)); break;
            case 4: graphPen.setColor( QColor(126 ,   47  , 142)); break;
            case 5: graphPen.setColor( QColor( 119 ,  172  ,  48)); break;
            case 6: graphPen.setColor( QColor(77 ,  190 ,  238)); break;
            case 7: graphPen.setColor(  QColor( 162 ,   20   , 47)); break;
            default: graphPen.setColor(legendColorForIndex(graphCount() - 1)); break;
        }
        graph()->setPen(graphPen);
    }
    updateReadout();
    replot();

    if(DP.first && DP.second)
    {
        if(graphCount() == 1 && !skip_register)
            this->ResetZoom();
    }


}

void PlotWidget::dropEvent(QDropEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if (!treeWidget || XYPlot() || !MainWindow_p || !MainWindow_p->GetLogic())
        return;

    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
    MainWindow *MW = this->MainWindow_p;
    bool addedGraph = false;

    for (QTreeWidgetItem* item : selectedItems)
    {
        QString id;
        if (PlotWidgetDropBinding::ResolveSupportedItem(MW->GetLogic(), item, &id))
        {
            AddCustomGraph(id);
            addedGraph = true;
            if(QApplication::keyboardModifiers() & Qt::ShiftModifier && this->graphCount() == 2 &&  !__isFFT)
            {
                graph(0)->setSelected(false);
                graph(1)->setSelected(true);
                SetAsXAxis();
            }
        }
    }

    if (addedGraph)
        UpdateGraphs("", true);

}


void PlotWidget::mouseReleaseEventII(QMouseEvent *mouse)
{
    if (MiddlePressed)
    {
        MiddlePressed = false;
        updateBoxZoomRectangle(mouse->pos());
        rectZoom->setVisible(false);
        const double x1 = rectZoom->topLeft->coords().x();
        const double y1 = rectZoom->topLeft->coords().y();
        const double x2 = rectZoom->bottomRight->coords().x();
        const double y2 = rectZoom->bottomRight->coords().y();

        if (BoxZoomAxes.testFlag(Qt::Horizontal) && x1 != x2)
            xAxis->setRange(std::min(x1, x2), std::max(x1, x2));
        if (BoxZoomAxes.testFlag(Qt::Vertical) && y1 != y2)
            yAxis->setRange(std::min(y1, y2), std::max(y1, y2));

        rectZoom->topLeft->setCoords(0, 0);
        rectZoom->bottomRight->setCoords(0, 0);
        updateReadout();
        replot();
    }

    DraggedCursor = 0;
}


void PlotWidget::mouseMoveEventII(QMouseEvent *mouse)
{
    if (SB)
    {
        const double rawX = xAxis->pixelToCoord(mouse->pos().x());
        const bool isTimeDomain = !__isFFT && !mXYPlot;
        const double displayX = isTimeDomain ? rawX * timeAxisScaleFactor() : rawX;
        const QString unit = isTimeDomain ? QString(" %1").arg(timeAxisUnitText()) : QString();
        SB->showMessage(QString("Coordinates: x = '%1%2', y = '%3'")
                            .arg(scopeValueText(displayX), unit, scopeValueText(yAxis->pixelToCoord(mouse->pos().y()))),
                        10000);
    }

    if (MiddlePressed)
    {
        updateBoxZoomRectangle(mouse->pos());
        replot();
    }
    else if (DraggedCursor != 0 && mouse->buttons().testFlag(Qt::LeftButton))
    {
        const double x = xAxis->pixelToCoord(mouse->pos().x());
        if (DraggedCursor == 1)
            setCursorPosition(1, x);
        else if (DraggedCursor == 2)
            setCursorPosition(2, x);

        updateCursorItems();
        updateReadout();
        replot(QCustomPlot::rpQueued);
    }
}


void PlotWidget::keyPressEvent( QKeyEvent * event )
{
    if (event->key() == Qt::Key_Control)
    {
        this->ControlPressed = true;
        setInteractions(QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);
    }
}

void PlotWidget::keyReleaseEvent(QKeyEvent * event )
{

    if (event->key() == Qt::Key_Control)
    {
        this->ControlPressed = false;
        this->MiddlePressed = false;
        setToolMode(CurrentToolMode);
    }
}


void PlotWidget::mousePressEventII(QMouseEvent *mouse)
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged

  if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
   axisRect()->setRangeDrag(xAxis->orientation());
  else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    axisRect()->setRangeDrag(yAxis->orientation());
  else
    axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);

  if (mouse->button() != Qt::LeftButton)
      return;

  if (ControlPressed || CurrentToolMode == PlotToolMode::BoxZoom)
  {
      beginBoxZoom(mouse);
      replot();
  }
  else if (CurrentToolMode == PlotToolMode::DoubleCursor)
  {
      placeOrMoveCursor(mouse);
      replot(QCustomPlot::rpQueued);
  }
}

void PlotWidget::mouseWheelDone()
{
    //if(qobject_cast<SubPlotMainWindow*>(this->SB->parent()))
    {
        QList<PlotWidget*> PlotWidgetFound = (this->MainWindow_p->findChildren<PlotWidget*>());
        for(auto itt : PlotWidgetFound)
        {
            if(itt != this)
            {
                if(itt->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
                {
                    itt->xAxis->setRange(this->xAxis->range());
                    itt->replot();
                }
                if(itt->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
                {
                    itt->yAxis->setRange(this->yAxis->range());
                    itt->replot();
                }
            }
        }
    }
}

void PlotWidget::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed

  if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
  {
   axisRect()->setRangeZoom(xAxis->orientation());
   //this->MainWindow_p->
  }
  else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
   axisRect()->setRangeZoom(yAxis->orientation());
  else
    axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}


bool PlotWidget::event( QEvent *event ){
    if(_touchDevice)
     {
         if(event->type() == QEvent::MouseButtonDblClick ||
                 event->type() == QEvent::MouseButtonPress ||
                 event->type() == QEvent::MouseButtonRelease ||
                 event->type() == QEvent::MouseMove ||
                 event->type() == QEvent::MouseTrackingChange)
         {
             event->ignore();
             return true;
         }

         if(event->type() == QEvent::Wheel)
             _touchDevice = false;
     }

     if(event->type() == QEvent::TouchBegin ||
             event->type() == QEvent::TouchUpdate ||
             event->type() == QEvent::TouchEnd ){

         if(!_touchDevice)
             _touchDevice = true;

         QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
         QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

         if(touchPoints.count() == 1 && touchEvent->touchPointStates().testFlag(QEventPoint::State::Released))
             _release2touch = false;

         if (touchPoints.count() == 1 && !_release2touch)
         {
             switch (event->type()) {
             case QEvent::TouchBegin:
             {
                 QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
                 QTouchEvent::TouchPoint touchPoints = touchEvent->touchPoints().first();
                 QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonPress,
                                                  touchPoints.pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

                 mousePressEvent(e); //==> meilleure methode
             }break;

             case QEvent::TouchUpdate:
             {
                 QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
                 QTouchEvent::TouchPoint touchPoints = touchEvent->touchPoints().first();
                 QMouseEvent *e = new QMouseEvent(QEvent::MouseMove,
                                                  touchPoints.pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

                 mouseMoveEvent(e);
             }break;

             case QEvent::TouchEnd:{
                 QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
                 QTouchEvent::TouchPoint touchPoints = touchEvent->touchPoints().first();
                 QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonRelease,
                                                  touchPoints.pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

                 mouseReleaseEvent(e);
             }break;

             default:
                 break;
             }
         }else if (touchPoints.count() == 2) {
             _release2touch = true;
             // determine scale factor
             const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
             const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();//   touchPoints.last();

             double currentScaleFactor =
                     QLineF(touchPoint0.pos(), touchPoint1.pos()).length() /
                     QLineF(touchPoint0.lastPos(), touchPoint1.lastPos()).length();

             double currentScaleFactorX = (abs(touchPoint1.pos().x() - touchPoint0.pos().x())
                                        - abs(touchPoint1.lastPos().x() - touchPoint0.lastPos().x()))
                                        / QLineF(touchPoint0.lastPos(), touchPoint1.lastPos()).length() +1.;

             double currentScaleFactorY = (abs(touchPoint1.pos().y() - touchPoint0.pos().y())
                                         - abs(touchPoint1.lastPos().y() - touchPoint0.lastPos().y()))
                                         / QLineF(touchPoint0.lastPos(), touchPoint1.lastPos()).length() +1.;



             QPointF centreZoom = QPointF((touchPoint0.pos().x()+ touchPoint1.pos().x())/2 ,
                                          (touchPoint0.pos().y()+ touchPoint1.pos().y())/2);
             QPointF lastCenterZoom = QPointF((touchPoint0.lastPos().x()+ touchPoint1.lastPos().x())/2 ,
                                              (touchPoint0.lastPos().y()+ touchPoint1.lastPos().y())/2);


             if (touchEvent->touchPointStates().testFlag(QEventPoint::State::Released))
             {
                 currentScaleFactor = 1;
                 currentScaleFactorX = 1;
                 currentScaleFactorY = 1;
             }



             if(currentScaleFactor<1)
                 currentScaleFactor = currentScaleFactor + (1-currentScaleFactor)/8;
             else
                 currentScaleFactor = currentScaleFactor+ (currentScaleFactor-1)/8;

             currentScaleFactor =1/currentScaleFactor;

             if(currentScaleFactorX<1)
                 currentScaleFactorX = currentScaleFactorX + (1-currentScaleFactorX)/8;
             else
                 currentScaleFactorX = currentScaleFactorX+ (currentScaleFactorX-1)/8;

             currentScaleFactorX =1/currentScaleFactorX;

             if(currentScaleFactorY<1)
                 currentScaleFactorY = currentScaleFactorY + (1-currentScaleFactorY)/8;
             else
                 currentScaleFactorY = currentScaleFactorY+ (currentScaleFactorY-1)/8;

             currentScaleFactorY =1/currentScaleFactorY;







             double diffX = this->xAxis->pixelToCoord(lastCenterZoom.x())
                     - this->xAxis->pixelToCoord(centreZoom.x());

             double diffY = this->yAxis->pixelToCoord(lastCenterZoom.y())
                     - this->yAxis->pixelToCoord(centreZoom.y());

             if(!touchEvent->touchPointStates().testFlag(QEventPoint::State::Released)){


                 if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
                 {
                   this->xAxis->moveRange(diffX);
                   this->xAxis->scaleRange(currentScaleFactor,this->xAxis->pixelToCoord(centreZoom.x()));
                 }
                 else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
                 {
                     this->yAxis->moveRange(diffY);
                     this->yAxis->scaleRange(currentScaleFactor,this->yAxis->pixelToCoord(centreZoom.y()));
                 }
                 else
                 {
                     this->xAxis->moveRange(diffX);
                     this->yAxis->moveRange(diffY);
                     this->xAxis->scaleRange(currentScaleFactorX,this->xAxis->pixelToCoord(centreZoom.x()));
                     this->yAxis->scaleRange(currentScaleFactorY,this->yAxis->pixelToCoord(centreZoom.y()));
                 }

                 this->replot();
             }
         }
         return true;
     }
     return QWidget::event(event);
}


bool PlotWidget::LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text)
{
    double xmin = 0.0;
    double xmax = 0.0;
    double ymin = 0.0;
    double ymax = 0.0;
    QString XLabel;
    QString YLabel;
    bool IsXYPlot = false;
    QString XData;
    int timeAxisUnit = static_cast<int>(TimeAxisUnit::Seconds);
    int scopeToolMode = static_cast<int>(PlotToolMode::Navigate);
    bool showCursors = false;
    bool cursorAActive = false;
    bool cursorBActive = false;
    double cursorAX = 0.0;
    double cursorBX = 0.0;

    for(auto itt : Attributes)
    {
        if(itt.first ==  QString("Xmin"))
            xmin = itt.second.toDouble();
        else if(itt.first ==  QString("Xmax"))
            xmax = itt.second.toDouble();
        else if(itt.first ==  QString("Ymin"))
            ymin = itt.second.toDouble();
        else if(itt.first ==  QString("Ymax"))
            ymax = itt.second.toDouble();
        else if(itt.first ==  QString("XLabel"))
            XLabel = itt.second;
        else if(itt.first ==  QString("YLabel"))
            YLabel = itt.second;
        else if(itt.first ==  QString("XYPlot"))
            IsXYPlot = itt.second.toInt();
        else if(itt.first ==  QString("XData"))
            XData = itt.second;
        else if(itt.first == QString("isFFT"))
            __isFFT = itt.second.toInt();
        else if(itt.first == QString("TimeAxisUnit"))
            timeAxisUnit = itt.second.toInt();
        else if(itt.first == QString("ScopeToolMode"))
            scopeToolMode = itt.second.toInt();
        else if(itt.first == QString("ShowCursors"))
            showCursors = itt.second.toInt();
        else if(itt.first == QString("ScopeReadoutVisible"))
            showCursors = itt.second.toInt();
        else if(itt.first == QString("ScopeCursorAActive"))
            cursorAActive = itt.second.toInt();
        else if(itt.first == QString("ScopeCursorBActive"))
            cursorBActive = itt.second.toInt();
        else if(itt.first == QString("ScopeCursorAX"))
            cursorAX = itt.second.toDouble();
        else if(itt.first == QString("ScopeCursorBX"))
            cursorBX = itt.second.toDouble();

    }

      //Doppelt, da die grenzen beim ersten setzen getauscht werden können
       xAxis->setRangeUpper(xmax);
       xAxis->setRangeLower(xmin);
       yAxis->setRangeUpper(ymax);
       yAxis->setRangeLower(ymin);
       xAxis->setRangeUpper(xmax);
       xAxis->setRangeLower(xmin);
       yAxis->setRangeUpper(ymax);
       yAxis->setRangeLower(ymin);

       xAxis->setLabel(XLabel);
       yAxis->setLabel(YLabel);
       SetXDataName(XData);
       SetXYPlot(IsXYPlot);
       if (timeAxisUnit >= static_cast<int>(TimeAxisUnit::Seconds) &&
           timeAxisUnit <= static_cast<int>(TimeAxisUnit::Microseconds))
           CurrentTimeAxisUnit = static_cast<TimeAxisUnit>(timeAxisUnit);
       if (TimeUnitComboBox)
       {
           TimeUnitComboBox->blockSignals(true);
           TimeUnitComboBox->setCurrentIndex(static_cast<int>(CurrentTimeAxisUnit));
           TimeUnitComboBox->blockSignals(false);
       }
       updateTimeAxisPresentation();

       CursorA.active = cursorAActive;
       CursorA.x = cursorAX;
       CursorB.active = cursorAActive && cursorBActive;
       CursorB.x = cursorBX;
       if (scopeToolMode == 3)
           scopeToolMode = static_cast<int>(PlotToolMode::DoubleCursor);
       if (scopeToolMode >= static_cast<int>(PlotToolMode::Navigate) &&
           scopeToolMode <= static_cast<int>(PlotToolMode::DoubleCursor))
           setToolMode(static_cast<PlotToolMode>(scopeToolMode));
       else
           setToolMode(PlotToolMode::Navigate);
       setCursorsVisible(showCursors);

       return true;
}
bool PlotWidget::SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text)
{
    std::pair<QString, QString> Attribut;
    Attribut.first =  "XLabel";
    Attribut.second =  xAxis->label();
    Attributes.push_back(Attribut);

    Attribut.first =  "YLabel";
    Attribut.second =  yAxis->label();
    Attributes.push_back(Attribut);

    Attribut.first =  "Xmin";
    Attribut.second =  QString::number(xAxis->range().lower);
    Attributes.push_back(Attribut);

    Attribut.first =  "Xmax";
    Attribut.second =  QString::number(xAxis->range().upper);
    Attributes.push_back(Attribut);

    Attribut.first =  "Ymin";
    Attribut.second =  QString::number(yAxis->range().lower);
    Attributes.push_back(Attribut);

    Attribut.first =  "Ymax";
    Attribut.second =  QString::number(yAxis->range().upper);
    Attributes.push_back(Attribut);

    Attribut.first =  "XYPlot";
    Attribut.second =  QString::number(XYPlot());
    Attributes.push_back(Attribut);

    //save __isFFT
    Attribut.first =  "isFFT";
    Attribut.second =  QString::number(__isFFT);
    Attributes.push_back(Attribut);

    Attribut.first = "TimeAxisUnit";
    Attribut.second = QString::number(static_cast<int>(CurrentTimeAxisUnit));
    Attributes.push_back(Attribut);

    Attribut.first = "ScopeToolMode";
    Attribut.second = QString::number(static_cast<int>(CurrentToolMode));
    Attributes.push_back(Attribut);

    Attribut.first = "ShowCursors";
    Attribut.second = QString::number(CursorsVisible);
    Attributes.push_back(Attribut);

    Attribut.first = "ScopeCursorAActive";
    Attribut.second = QString::number(CursorA.active);
    Attributes.push_back(Attribut);

    Attribut.first = "ScopeCursorAX";
    Attribut.second = QString::number(CursorA.x, 'g', 17);
    Attributes.push_back(Attribut);

    Attribut.first = "ScopeCursorBActive";
    Attribut.second = QString::number(CursorB.active);
    Attributes.push_back(Attribut);

    Attribut.first = "ScopeCursorBX";
    Attribut.second = QString::number(CursorB.x, 'g', 17);
    Attributes.push_back(Attribut);


    Attribut.first =  "XData";
    Attribut.second.clear();
    if(XYPlot() && graphCount() > 1)
    {
        if(graph(0)->visible())
            Attribut.second =  graph(1)->ID();
        else
            Attribut.second =  graph(0)->ID();
    }
    Attributes.push_back(Attribut);

    Text = objectName();
    return true;

}
void PlotWidget::ConnectToID(DataManagementSetClass* DM, QString ID)
{
    AddCustomGraph(ID, true);
    if(XYPlot())
    {
        if(graphCount()==2)
        {
            graph(0)->setSelected(false);
            graph(1)->setSelected(false);
           if( graph(0)->ID() == XDataName())
               graph(0)->setSelected(true);
           if( graph(1)->ID() == XDataName())
               graph(1)->setSelected(true);
           SetAsXAxis(true);
        }
    }
}

void PlotWidget::SetVariantData(ToFormMapper Data)
{
    UpdateGraphs("New Data");
}
void PlotWidget::GetVariantData(ToFormMapper *Data)
{

}

