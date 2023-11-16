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
#include "mainwindow.h"
#include <fftw3.h>

#define UseFFTW

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


   legend->setVisible(true);
   legend->setFont(NewFont);
   legend->setSelectedFont(NewFont);
   legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items


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

   timer.start();
   TimeSinceLastPlot.start();

   UpdateTimer = new QTimer(this);
   connect(UpdateTimer, SIGNAL(timeout()), this, SLOT(UpdataGraphs()));
   UpdateTimer->start(25);

   update();

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
          this->MainWindow_p->GetLogic()->DeleteEntryOfObject(selectedGraphs().at(k)->ID(),this);
          removeGraph(selectedGraphs().at(k));
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
            xAxis->setLabel("t [s]");
            yAxis->setLabel("Data");
        }
      ID_X.clear();
  }
  if(graphCount() == 0)
  {
        QualityCriteriaText->deleteLater();
        QualityCriteriaText = nullptr;
  }
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

                if(!skip)
                    ResetZoom();
            }
        }
    }
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
  QualityCriteriaText->deleteLater();
  QualityCriteriaText = nullptr;
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
        xAxis->setLabel("t [s]");
        yAxis->setLabel("Data");
    }
    ID_X.clear();

  }
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
      {
        menu->addAction("Show Quality Criteria", this, SLOT(ShowQualityCriteria()));
        menu->addSeparator();
      }

      menu->addAction("Update Data", this, SLOT(UpdataGraphs()));
      menu->addAction("Reset Zoom", this, SLOT(ResetZoom()));
    }


}
 // menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
        menu->addSeparator();
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

void PlotWidget::ToggleTimeFreq(void)
{
      QPen graphPen;



    if(__isFFT)
    {
        __ShowQualityCriteria = false;
        __isFFT = false;
        xAxis->setLabel("t [s]");
        yAxis->setLabel("Data");
        // set pen for all graphs
        for(int i=0; i<graphCount(); ++i)
        {
            switch (i) //MatlabColors
            {
                case 1: graphPen.setColor( QColor(0  ,  114  ,  189)); break;
                case 2: graphPen.setColor( QColor(217  ,  83  ,  25)); break;
                case 3: graphPen.setColor( QColor(237  , 177   , 32)); break;
                case 4: graphPen.setColor( QColor(126 ,   47  , 142)); break;
                case 5: graphPen.setColor( QColor( 119 ,  172  ,  48)); break;
                case 6: graphPen.setColor( QColor(77 ,  190 ,  238)); break;
                case 7: graphPen.setColor(  QColor( 162 ,   20   , 47)); break;
                default:  graphPen.setColor( QColor( 217  ,  83  ,  25)); break;;
            }
            graph(i)->setLineStyle(QCPGraph::lsLine);
            constexpr qreal lineWidth{ 1 };
            constexpr qreal plotScatterSize{ 5 };
            graphPen.setWidthF(1);
            graph(i)->setScatterStyle(
                    QCPScatterStyle{ QCPScatterStyle::ssCircle, Qt::transparent, plotScatterSize });
            graph(i)->setPen(graphPen);
        }

        replot();
    }
    else
    {
        __isFFT = true;
        CalculateFFT();

        xAxis->setLabel("f [Hz]");
        yAxis->setLabel("Amplitude");
         // set pen for all graphs
        for(int i=0; i<graphCount(); ++i)
        {
            int occupancy = 255;
            if(graphCount() > 1)
            {
                occupancy = 128;
            }
            switch (i) //MatlabColors
            {
                case 1: graphPen.setColor( QColor(0  ,  114  ,  189, occupancy)); break;
                case 2: graphPen.setColor( QColor(217  ,  83  ,  25, occupancy)); break;
                case 3: graphPen.setColor( QColor(237  , 177   , 32, occupancy)); break;
                case 4: graphPen.setColor( QColor(126 ,   47  , 142, occupancy)); break;
                case 5: graphPen.setColor( QColor( 119 ,  172  ,  48, occupancy)); break;
                case 6: graphPen.setColor( QColor(77 ,  190 ,  238, occupancy)); break;
                case 7: graphPen.setColor(  QColor( 162 ,   20   , 47, occupancy)); break;
                default:  graphPen.setColor( QColor( 217  ,  83  ,  25, occupancy)); break;;
            }
            graph(i)->setLineStyle(QCPGraph::lsImpulse);
            constexpr qreal plotScatterSize{ 10 };
            graphPen.setWidthF(5);
            graph(i)->setScatterStyle(
                    QCPScatterStyle{ QCPScatterStyle::ssCross, Qt::transparent, plotScatterSize });
            graph(i)->setPen(graphPen);

        }

        replot();
    }

    ResetZoom();

}

void PlotWidget::UpdataGraphs(QString ID, bool force)
{
    if(IsUpdateBlocked())
        return;
    if(!force)
    {
        //Update 20ms after last ID arreved
        if(ID.size())
        {
            timer.restart();
            UpdateCounter++;
        }

        if(timer.elapsed()<=25)
        {
            return;
        }
        if(TimeSinceLastPlot.elapsed() < 75)
            return;

        TimeSinceLastPlot.restart();

        if(XYPlot())
        {
            if(UpdateCounter < 2)
                return;
        }
        UpdateCounter=0;
    }

    DataPair XYX;
    DataPair XYY;
    int Yindex = 0;

    for(int i = 0; i < graphCount(); i++)
    {
        ToFormMapper* Element = MainWindow_p->GetLogic()->GetContainer(graph(i)->ID());
        if(Element)
        {
            DataPair DP = Element->GetPointerPair();
            if(DP.first && DP.second)
            {
                if(DP.first->size() && DP.second->size())
                {
                    if(i == 0)
                    {
                        Tmin = DP.third;
                    }
                    else
                    {
                        if(DP.third < Tmin)
                            Tmin = DP.third;
                    }
                }
            }
        }
    }
    for(int i = 0; i < graphCount(); i++)
    {
        ToFormMapper* Element = MainWindow_p->GetLogic()->GetContainer(graph(i)->ID());
        if(Element)
        {
            if(!XYPlot())
            {
                graph(i)->setData(Element->GetPointerPair().first,Element->GetPointerPair().second, Tmin);
                if(__isFFT)
                    CalculateFFT();
            }
            else
            {
                if(graph(i)->ID().compare(ID_X))
                {
                    XYY =Element->GetPointerPair();
                    Yindex = i;
                }
                else
                {
                    XYX =Element->GetPointerPair();
                }
                //if the name is equal it hast to be the
            }
        }
    }
    if(XYPlot())
    {
        if(XYX.second != nullptr)
            if(XYY.second != nullptr)
                 if(XYX.second->size() == XYY.second->size())
                    graph(Yindex)->setData(XYX.second,XYY.second, 0.0);
    }

    auto mmw = this->parentWidget();
    bool minimized = false;
    while(mmw->parentWidget())
    {
        if(mmw->isMinimized())
            minimized = true;
        if(this->MainWindow_p->isMinimized())
            minimized = true;

        mmw = mmw->parentWidget();
    }
    if(this->visibleRegion().rectCount() == 1 && !minimized)
    {
        int samesize = 1;
        for(int i = 0; i < this->graphCount()-2;i++)
            if(graph(i)->GetXDataPointer() && graph(i+1)->GetXDataPointer())
                samesize &=  graph(i)->GetXDataPointer()->size() == graph(i+1)->GetXDataPointer()->size();

            if(samesize || XYPlot())
                replot();
    }

    if(__ShowQualityCriteria)
    {
       if(!__isFFT)
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
        auto f = graph(i)->GetXFFTPointer();
        auto a = graph(i)->GetYFFTPointer();
        if(f && a)
        {
            //find nearest element in f to f1
            auto f1_l =  std::distance(f->begin(), std::lower_bound(f->begin(), f->end(), f1-5));
            auto f1_u =  std::distance(f->begin(), std::upper_bound(f->begin(), f->end(), f1+5));


            double U1 = 0;
            int f1m = std::round((f1_l+f1_u)/2);
            int delta = std::round(f1_u-f1_l)/2;

            for( int i = f1m-delta; i < f1m+delta; i++)
            {
                U1 += a->at(i);
            }

            auto U1sqare = U1*U1;

            //calculate THD
            double _THD = 0;
            double _WTHD = 0;
            double _RMS = 0;
            //iterate over all
            for( int i = 0; i < f->size(); i++)
            {
                _RMS += (a->at(i)*a->at(i));
             }
            int h = 2;
            for( int i = 2*f1m; i < f->size(); i = i + f1m)
            {
                for(int j = i-delta; j < i+delta; j++)
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
        boost::shared_ptr<std::vector<double>> x(graph(j)->GetXDataPointer());
        boost::shared_ptr<std::vector<double>> y(graph(j)->GetYDataPointer());

        if(!x || !y)
            return;

        if(x->size() != y->size())
            return;

        if(x->size() == 0)
            return;

        boost::shared_ptr<std::vector<double>> fft_x = boost::make_shared<std::vector<double>>();
        boost::shared_ptr<std::vector<double>> fft_y = boost::make_shared<std::vector<double>>();


        fftw_complex *in, *out;
        fftw_plan p;
        in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * x->size());
        out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * x->size());
        for(int i = 0; i < x->size(); i++)
        {
            in[i][0] = y->at(i);
            in[i][1] = 0;
        }
        p = fftw_plan_dft_1d(x->size(), in, out, FFTW_FORWARD, FFTW_ESTIMATE);
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
        boost::shared_ptr<std::vector<double>> y(graph(i)->GetYDataPointer());

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
        qDebug() << Tmin;
        qDebug() << x->front()-Tmin;
        qDebug() << x->back()-Tmin;

        xAxis->setRange(x->front()-Tmin,x->back()-Tmin);
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

    replot();
}

void PlotWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(event->source());
    if(!treeWidget || XYPlot())
    {
        return;
    }
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
   // auto si = treeWidget->parent();

    MainWindow *MW = this->MainWindow_p;

    QString id;
    auto sit = selectedItems[0];
    while(sit)
    {
        id.insert(0,sit->text( 0 ));
        sit = sit->parent();
        if(sit)
            id.insert(0,"::");
    }

    if (selectedItems[0]->childCount() == 0 &&
        MW->GetLogic()->GetContainer(id)->GetType().compare("Data")==0 &&
        MW->GetLogic()->GetContainer(id)->GetDataType().compare("vector<double>")==0)
        event->acceptProposedAction();
    else
    {
        QList<QString> SplitList  = id.split("::");
        id.clear();
        for(int i = 0; i < SplitList.size();i++)
        {
            id.append(SplitList[i]);
            if(i < SplitList.size() - 1)
            {
                id.append("::");
            }
            if(i == 0)
            {
                id.append("Buffered::");
            }
        }
        if(MW->GetLogic()->GetContainer(id))
            if (selectedItems[0]->childCount() == 0 &&
                MW->GetLogic()->GetContainer(id)->GetType().compare("Data")==0 &&
                MW->GetLogic()->GetContainer(id)->GetDataType().compare("vector<double>")==0)
                event->acceptProposedAction();
    }

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
        Tmin = DP.third;
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
            default:  graphPen.setColor( QColor( 217  ,  83  ,  25)); break;;
        }
        graph()->setPen(graphPen);
    }
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
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

    MainWindow *MW = this->MainWindow_p;

    QString id;
    int number = 0;

    for(auto sit :selectedItems )
    {
        id.clear();
        while(sit)
        {
            id.insert(0,sit->text( 0 ));
            sit = sit->parent();
            if(sit)
                id.insert(0,"::");
        }

        if (!(selectedItems[number]->childCount() == 0 &&
            MW->GetLogic()->GetContainer(id)->GetType().compare("Data")==0 &&
            MW->GetLogic()->GetContainer(id)->GetDataType().compare("vector<double>")==0))
        {
            QList<QString> SplitList  = id.split("::");
            id.clear();
            for(int i = 0; i < SplitList.size();i++)
            {
                id.append(SplitList[i]);
                if(i < SplitList.size() - 1)
                {
                    id.append("::");
                }
                if(i == 0)
                {
                    id.append("Buffered::");
                }
            }
        }
        if(MW->GetLogic()->GetContainer(id))
        {
            if ((selectedItems[number]->childCount() == 0 &&
                MW->GetLogic()->GetContainer(id)->GetType().compare("Data")==0 &&
                MW->GetLogic()->GetContainer(id)->GetDataType().compare("vector<double>")==0))
            {
                AddCustomGraph(id);
                if(QApplication::keyboardModifiers() & Qt::ShiftModifier && this->graphCount() == 2 &&  !__isFFT)
                {
                    graph(0)->setSelected(false);
                    graph(1)->setSelected(true);
                    SetAsXAxis();
                }
            }
         }
        //Get Data
        number++;
    }

    UpdataGraphs("", true);

}


void PlotWidget::mouseReleaseEventII(QMouseEvent *mouse)
{

    Q_UNUSED(mouse);

    if(this->MiddlePressed){
        this->MiddlePressed = 0;
    rectZoom->setVisible(false);
    auto x1 = rectZoom->topLeft->coords().x();
    auto y1 = rectZoom->topLeft->coords().y();
    auto x2 = rectZoom->bottomRight->coords().x();
    auto y2 = rectZoom->bottomRight->coords().y();

    xAxis->setRange(x1, x2);
    yAxis->setRange(y1, y2);

    rectZoom->topLeft->setCoords(0,0);
    rectZoom->bottomRight->setCoords(0,0);

    rectZoom->setVisible(false);

    replot();
    }

}


void PlotWidget::mouseMoveEventII(QMouseEvent *mouse)
{


    this->SB->showMessage(QString("Coordinates: x = '%1', y = '%2'").arg(xAxis->pixelToCoord(mouse->pos().x())).arg(yAxis->pixelToCoord(mouse->pos().y())),10000);
    if(this->ControlPressed && this->MiddlePressed)
    {
        double X = xAxis->pixelToCoord(mouse->x());
        double Y = yAxis->pixelToCoord(mouse->y());
        rectZoom->bottomRight->setCoords(X, Y);
        replot();
    }
}


void PlotWidget::keyPressEvent( QKeyEvent * event )
{
    if (event->key() == Qt::Key_Control)
    {
        this->ControlPressed = true;
         setInteractions( QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);
    }
}

void PlotWidget::keyReleaseEvent(QKeyEvent * event )
{

    if (event->key() == Qt::Key_Control)
    {
        this->ControlPressed = false;
        this->MiddlePressed = false;
        setInteractions( QCP::iRangeDrag |QCP::iRangeZoom | QCP::iSelectAxes |
                                            QCP::iSelectLegend | QCP::iSelectPlottables);

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

  if(this->ControlPressed && mouse->button()==Qt::LeftButton)
 {
      this->MiddlePressed = 1;
     double X = xAxis->pixelToCoord(mouse->x());
     double Y = yAxis->pixelToCoord(mouse->y());
      rectZoom->topLeft->setCoords(X, Y);
      rectZoom->setVisible(true);
      rectZoom->bottomRight->setCoords(X, Y);
      replot();
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


    Attribut.first =  "XData";
    if(XYPlot())
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
    UpdataGraphs("New Data");
}
void PlotWidget::GetVariantData(ToFormMapper *Data)
{

}

