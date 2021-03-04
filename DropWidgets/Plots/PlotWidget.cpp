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

PlotWidget::PlotWidget(MainWindow *MW, QWidget *parent, QStatusBar *SBI) :
    QCustomPlot(parent)
{


    setAttribute(Qt::WA_AcceptTouchEvents);
      _release2touch = false;
      _touchDevice = false;


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

   xAxis->setRange(-10, 10);
   yAxis->setRange(-10, 10);
   axisRect()->setupFullAxesBox();

   xAxis->setLabel("t [s]");

   QFont fontP  = parent->font();

   if(fontP.pointSize() < 10)
       fontP.setPointSize(10);

   QFont NewFont = font();
   NewFont.setPointSize(fontP.pointSize());
  // NewFont.setFamily(fontP.family());


   xAxis->setTickLabelFont(NewFont);
   yAxis->setTickLabelFont(NewFont);

   xAxis->setLabelFont(NewFont);
   yAxis->setLabelFont(NewFont);

   QFont Highlight = xAxis->selectedLabelFont();
   Highlight.setPointSize(fontP.pointSize());
   xAxis->setSelectedLabelFont(Highlight);
   yAxis->setSelectedLabelFont(Highlight);


   yAxis->setLabel("Data");
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
    QString newLabel = QInputDialog::getText(this, "QCustomPlot example", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
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
      xAxis->setLabel("t [s]");
      yAxis->setLabel("Data");
      ID_X.clear();
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

                //ResetZoom();
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
  if(!selectedGraphs().size())
  {
    SetXYPlot(false);
    legend->setVisible(true);
    xAxis->setLabel("t [s]");
    yAxis->setLabel("Data");
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
        menu->addAction("Toggle Marker", this, SLOT(ToggleMarker()));
      menu->addSeparator();
      menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
      menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
      menu->addSeparator();
      menu->addAction("Save to Pdf", this, SLOT(SaveToPdf()));
      menu->addSeparator();
      if (graphCount() == 2)
      {
        menu->addAction("Set As X-Axis", this, SLOT(SetAsXAxis()));
        menu->addSeparator();
      }
      menu->addAction("Update Data", this, SLOT(UpdataGraphs()));
      menu->addAction("Reset Zoom", this, SLOT(ResetZoom()));
    }


}
 // menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
  menu->popup(mapToGlobal(pos));
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
 auto filename =  QFileDialog::getSaveFileName(this, "Save file", "plot.pdf", "*.pdf");
 if(filename.size())
   savePdf(filename);
}



void PlotWidget::UpdataGraphs(QString ID, bool force)
{
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
                graph(i)->setData(Element->GetPointerPair().first,Element->GetPointerPair().second, Tmin);
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
    if(this->visibleRegion().rects().count() == 1 && !minimized)
    {
        int samesize = 1;
        for(int i = 0; i < this->graphCount()-2;i++)
            if(graph(i)->GetXDataPointer() && graph(i+1)->GetXDataPointer())
                samesize &=  graph(i)->GetXDataPointer()->size() == graph(i+1)->GetXDataPointer()->size();

            if(samesize || XYPlot())
                replot();
    }
    timer.restart();
}


void PlotWidget::ResetZoom()
{
    if(!graphCount())
        return;

    boost::shared_ptr<std::vector<double>> x(graph(0)->GetXDataPointer());
    boost::shared_ptr<std::vector<double>> y(graph(0)->GetYDataPointer());
    if(XYPlot())
    {
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


    if(DP.first && DP.second)
        graph()->setData(DP.first,DP.second, Tmin);
   // if(DP.first->size())
    {
        QPen graphPen;
        graphPen.setWidthF(1);

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
        if(graphCount() == 1)
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
                if(QApplication::keyboardModifiers() & Qt::ShiftModifier && this->graphCount() == 2)
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
                if(itt->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
                {
                    itt->xAxis->setRange(this->xAxis->range());
                    itt->replot();
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

         if(touchPoints.count() == 1 && touchEvent->touchPointStates().testFlag(Qt::TouchPointReleased))
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


             if (touchEvent->touchPointStates().testFlag(Qt::TouchPointReleased))
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

             if(!touchEvent->touchPointStates().testFlag(Qt::TouchPointReleased)){


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
        if(itt.first == "Xmin")
            xmin = itt.second.toDouble();
        else if(itt.first == "Xmax")
            xmax = itt.second.toDouble();
        else if(itt.first == "Ymin")
            ymin = itt.second.toDouble();
        else if(itt.first == "Ymax")
            ymax = itt.second.toDouble();
        else if(itt.first == "XLabel")
            XLabel = itt.second;
        else if(itt.first == "YLabel")
            YLabel = itt.second;
        else if(itt.first == "XYPlot")
            IsXYPlot = itt.second.toInt();
        else if(itt.first == "XData")
            XData = itt.second;
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
