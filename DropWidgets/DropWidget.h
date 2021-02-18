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

#pragma once

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
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QProgressBar>
#include <QtUiTools>
#include <QtWidgets/QSlider>
#include "QTimer"
#include "../DataManagement/mapper.h"
#include "../DataManagement/DataManagementSetClass.h"

class VariantDropWidget
{
public:
    VariantDropWidget(QWidget *parent=0);
    virtual void SetVariantData(ToFormMapper Data) = 0;
    virtual void GetVariantData(ToFormMapper *Data) = 0;

    virtual bool LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text) = 0;
    virtual bool SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text) = 0;
    virtual void ConnectToID(DataManagementSetClass* DM, QString ID) = 0;


    //virtual void GetVariantData() = 0;

};
