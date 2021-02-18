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

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "../plugins/InterfaceDataType.h"

#ifndef MAPPER
#define MAPPER


//Defines one Connection between an ID and a Form

struct ObjectStruct
{
    ObjectStruct(QString FormNameI, QObject* FormPI,QString FormTypeI){FormName = FormNameI; FormP = FormPI;FormType=FormTypeI;}
    ObjectStruct(){return;}
    QString FormName; //vector of unique names which are connected with the widget
    QObject* FormP;   //vector of Pointers to the connected widgets
    QString FormType; //type of the widgets
};

class ToFormMapper: public InterfaceData
{
    public:
    ToFormMapper(QString DataTypeI, QString TypeI):InterfaceData(DataTypeI,TypeI){}
    std::vector<ObjectStruct> Objects;
    double MinValue = 0.0;         //minimal Value of the current ID (needed for e.g. slider widget)
    double MaxValue = 0.0;         //maximum Value of the current ID (needed for e.g. slider widget)
};


#endif // MAPPER

