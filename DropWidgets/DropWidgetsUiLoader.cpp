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

#include "DropWidgetsUiLoader.h"
#include "DropWidgets.h"


#include "../CustomWidgets/QLedIndicator.h"
#include "../CustomWidgets/QTSLedIndicator.h"
#include "../CustomWidgets/QTSLedIndicator.h"

#include "Plots/PlotWidget.h"


QWidget* DropWidgetsUiLoader::createWidget(const QString &className, QWidget *parent , const QString &name)
{
  //P->XMLCounder++;
  QWidget* widget = NULL;
  if (className == "QPushButton")
  {
     // replace any QPushButton instances with instance of our
     // own QPushButtonD
     widget = new QPushButtonD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QLineEdit")
  {
     // replace any QLineEdit instances with instance of our
     // own QLineEditD
     widget = new QLineEditD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QCheckBox")
  {
     // replace any QCheckBox instances with instance of our
     // own QCheckBoxD
     widget = new QCheckBoxD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QDoubleSpinBox")
  {
     // replace any QDoubleSpinBox instances with instance of our
     // own QDoubleSpinBoxD
     widget = new QDoubleSpinBoxD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QSpinBox")
  {
     // replace any QDoubleSpinBox instances with instance of our
     // own QDoubleSpinBoxD
     widget = new QSpinBoxD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QProgressBar")
  {
     // replace any QDoubleSpinBox instances with instance of our
     // own QDoubleSpinBoxD
     widget = new QProgressBarD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QLCDNumber")
  {
     // replace any QLCDNumber instances with instance of our
     // own QLCDNumberD
     widget = new QLCDNumberD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QLabel")
  {
     // replace any QLabel instances with instance of our
     // own QLabelD
     widget = new QLabelD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QSlider")
  {
     // replace any QSlider instances with instance of our
     // own QSliderD
     widget = new QSliderD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QListView")
  {
     // replace any QListView instances with instance of our
     // own QListViewD
     widget = new QListViewD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QLed")
  {
     // replace any QLed instances with instance of our
     // own QLedD
     widget = new QLed(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QComboBox")
  {
     // replace any QComboBox instances with instance of our
     // own QComboBoxD
     widget = new QComboBoxD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QTSLed")
  {
     // replace any QTSLed instances with instance of our
     // own QTSLed
     widget = new QTSLed(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QBLed")
  {
     // replace any QBLed instances with instance of our
     // own QBLed
     widget = new QBLed(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "QTableWidget")
  {
     // replace any QBLed instances with instance of our
     // own QBLed
     widget = new QTableWidgeD(parent);
     widget->setAcceptDrops(true);
  }
  else if (className == "PlotWidget")
  {
     // replace any PlotWidget instances with instance of our
     // own PlotWidget

      int i = 0;
      MainWindow *MW = qobject_cast<MainWindow*>( QApplication::topLevelWidgets().at(i++));
      while(!MW)
           MW = qobject_cast<MainWindow*>( QApplication::topLevelWidgets().at(i++));

     widget = new PlotWidget((MainWindow*)MW, parent, ((MainWindow*)MW)->GetStatusBar());
  }
  else if (NULL != widget)
  {
     widget->setObjectName(name);
  }
  else
  {
     // let base class handle any widgets for which we don't have a
     // custom class
     widget = QUiLoader::createWidget(className, parent, name);
  }
  return widget;
}

