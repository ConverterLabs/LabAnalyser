/***************************************************************************
**                                                                        **
**  This file is part of LabAnlyser.                                      **
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

#include "SubPlotMainWindow.h"
#include "DropWidgets/Plots/qcustomplot.h"
#include <boost\any.hpp>
#include <boost\function.hpp>
#include <boost\bind.hpp>
#include <boost\shared_ptr.hpp>

#include "DropWidgets/Plots/PlotWidget.h"
#include"mainwindow.h"

SubPlotMainWindow::SubPlotMainWindow(QMainWindow* MWI, QWidget *parent) :
    QMainWindow(parent)
{
    statusBar = new QStatusBar(this);
    statusBar->setObjectName(QStringLiteral("Figure"));
    this->setStatusBar(statusBar);
    this->MW = MWI;
}


SubPlotMainWindow::~SubPlotMainWindow()
{

}

void  SubPlotMainWindow::closeEvent(QCloseEvent *event)
{
   // Delete the ids
   MainWindow* MWS = qobject_cast<MainWindow*>(this->MW);
   MWS->DeleteFigure(this);
   QWidget* CW =  this->centralWidget();
   QObjectList childs = CW->children();

   for(int i = 0; i < childs.size(); i++)
   {
       PlotWidget* PW = qobject_cast<PlotWidget*>(childs[i]);
       if(PW)
           PW ->ClearAllGraphs();
   }
   event->accept();
   this->deleteLater();
}


 QStatusBar* SubPlotMainWindow::GetStatusBar()
 {
        return this->statusBar;
 }
