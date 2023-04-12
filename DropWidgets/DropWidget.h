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

#include <QtCore/QVariant>
#include <QAction>
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



/*!
 * \class VariantDropWidget
 * \brief This class provides a widget for displaying and manipulating variant data.
 *
 * The VariantDropWidget class provides a widget for displaying and manipulating variant data. It has
 * several pure virtual methods that must be implemented by subclasses.
 */
class VariantDropWidget
{
public:
    /*!
     * \brief Constructs a VariantDropWidget with the given parent.
     * \param parent The parent widget.
     */
    VariantDropWidget(QWidget *parent=0){};

    /*!
     * \brief Destructs the VariantDropWidget.
     */
    virtual ~VariantDropWidget(){};

    /*!
     * \brief Sets the variant data for the widget.
     * \param Data The data to set.
     */
    virtual void SetVariantData(ToFormMapper Data) = 0;

    /*!
     * \brief Gets the variant data from the widget.
     * \param Data Pointer to a ToFormMapper object to store the data.
     */
    virtual void GetVariantData(ToFormMapper *Data) = 0;

    /*!
     * \brief Loads the widget's data from an XML file.
     * \param Attributes The attributes of the XML element.
     * \param Text The text content of the XML element.
     * \returns true if the data was successfully loaded, false otherwise.
     */
    virtual bool LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text) = 0;

    /*!
     * \brief Saves the widget's data to an XML file.
     * \param Attributes The attributes of the XML element.
     * \param Text The text content of the XML element.
     * \returns true if the data was successfully saved, false otherwise.
     */
    virtual bool SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text) = 0;

    /*!
     * \brief Connects the widget to a DataManagementSetClass object with the given ID.
     * \param DM Pointer to the DataManagementSetClass object.
     * \param ID The ID of the object to connect to.
     */
    virtual void ConnectToID(DataManagementSetClass* DM, QString ID) = 0;

    //virtual void GetVariantData() = 0;

};
