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

#include "Plots/PlotWidget.h"

namespace
{
typedef QWidget* (*AdapterFactory)(QWidget* parent);

template <typename Adapter>
QWidget* CreateAdapter(QWidget* parent)
{
    return new Adapter(parent);
}

struct AdapterRegistration
{
    const char* designerClassName;
    AdapterFactory factory;
};

QWidget* CreateDropWidgetAdapter(const QString& className, QWidget* parent)
{
    static const AdapterRegistration adapters[] = {
        { "QPushButton", &CreateAdapter<QPushButtonD> },
        { "QLineEdit", &CreateAdapter<QLineEditD> },
        { "QCheckBox", &CreateAdapter<QCheckBoxD> },
        { "QDoubleSpinBox", &CreateAdapter<QDoubleSpinBoxD> },
        { "QSpinBox", &CreateAdapter<QSpinBoxD> },
        { "QProgressBar", &CreateAdapter<QProgressBarD> },
        { "QLCDNumber", &CreateAdapter<QLCDNumberD> },
        { "QLabel", &CreateAdapter<QLabelD> },
        { "QSlider", &CreateAdapter<QSliderD> },
        { "QListView", &CreateAdapter<QListViewD> },
        { "QLed", &CreateAdapter<QLed> },
        { "QComboBox", &CreateAdapter<QComboBoxD> },
        { "QTSLed", &CreateAdapter<QTSLed> },
        { "QBLed", &CreateAdapter<QBLed> },
        { "QTableWidget", &CreateAdapter<QTableWidgeD> }
    };

    for (const AdapterRegistration& adapter : adapters)
    {
        if (className == QLatin1String(adapter.designerClassName))
            return adapter.factory(parent);
    }
    return NULL;
}
}

QWidget* DropWidgetsUiLoader::createWidget(const QString &className, QWidget *parent , const QString &name)
{
  QWidget* widget = CreateDropWidgetAdapter(className, parent);
  if (widget)
  {
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
  else
  {
     // let base class handle any widgets for which we don't have a
     // custom class
     widget = QUiLoader::createWidget(className, parent, name);
  }
  return widget;
}

