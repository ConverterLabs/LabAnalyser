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

#include "DropWidget.h"
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
#include "../CustomWidgets/QLedIndicator.h"
#include "QTimer"


/*!
 * \class QLed
 * \brief The QLed class is a subclass of QLedIndicator and VariantDropWidget.
 *
 * QLed provides several member functions for handling drag and drop events, setting and getting a bit value, and interacting with data management and XML files. It also has a public slot for displaying a context menu and a public signal for requesting updates. The class has a private member variable for storing a bit value and a static member variable for keeping track of the number of bits.
 */
class QLed : public QLedIndicator, public VariantDropWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructs a QLed object with the given parent widget.
     * \param parent The parent widget of the QLed object.
     */
    QLed(QWidget *parent=0);

    /*!
     * \brief Handles drag enter events for the QLed object.
     * \param event The drag enter event.
     */
    virtual void dragEnterEvent(QDragEnterEvent *event);

    /*!
     * \brief Handles drag move events for the QLed object.
     * \param de The drag move event.
     */
    virtual void dragMoveEvent(QDragMoveEvent *de);

    /*!
     * \brief Handles drop events for the QLed object.
     * \param event The drop event.
     */
    virtual void dropEvent(QDropEvent *event);

    /*!
     * \brief Sets the bit value for the QLed object.
     * \param bit_in The new bit value.
     */
    void SetBit(uint32_t bit_in) { bit = bit_in;}

    /*!
     * \brief Returns the current bit value for the QLed object.
     * \return The current bit value.
     */
    uint32_t GetBit() { return bit;}

    /*!
     * \brief Sets variant data for the QLed object.
     * \param Data The data to be set.
     */
    void SetVariantData(ToFormMapper Data) override;

    /*!
     * \brief Gets variant data for the QLed object.
     * \param Data A pointer to the data to be returned.
     */
    void GetVariantData(ToFormMapper *Data) override;

    /*!
     * \brief Loads data from an XML file for the QLed object.
     * \param Attributes A vector of attribute name-value pairs from the XML file.
     * \param Text The text content of the XML element.
     * \return True if the data was successfully loaded, false otherwise.
     */
    bool LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text) override;

    /*!
     * \brief Saves data to an XML file for the QLed object.
     * \param Attributes A vector of attribute name-value pairs to be written to the XML file.
     * \param Text The text content to be written to the XML element.
     * \return True if the data was successfully saved, false otherwise.
     */
    bool SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text) override;

    /*!
     * \brief Connects the QLed object to a data management class using a given ID.
     * \param DM A pointer to the data management class.
     * \param ID The ID to be used for the connection.
     */
    void ConnectToID(DataManagementSetClass* DM, QString ID) override;
    public slots:
    /*!
    * \brief Displays a context menu for the QLed object.
    * \param pos The position at which to display the menu.
    */
    void contextMenu(QPoint pos);

    /*!
     * \brief Removes the connection of the QLed object to a data management class.
     */
    void RemoveConnection();
    Q_SIGNALS:
    /*!
    * \brief Requests an update for the QLed object.
    */
    void RequestUpdate();

    private:
    /*!
    * \brief The current bit value for the QLed object.
    */
    uint32_t bit = 0;

    /*!
     * \brief A static counter for the number of bits.
     */
    static uint32_t bitcounter;
    };






