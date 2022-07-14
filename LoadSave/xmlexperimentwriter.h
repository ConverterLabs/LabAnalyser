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
#include <QObject>
#include <QXmlStreamReader>
#include "../DataManagement/DataManagementClass.h"
#include "../DataManagement/DataMessengerClass.h"
#include <QDir>
#include <QFile>

class xmlexperimentwriter: public QObject
{
   Q_OBJECT
public:
    xmlexperimentwriter(QObject *parent, MessengerClass& Messenger_, DataManagementClass& Manager_ );
    bool write(QString LoadPath_);
private:
    MessengerClass& m_Messenger ;
    DataManagementClass& m_Manager ;
    QXmlStreamWriter m_xmlWriter;
    QDir m_T;

    void writeExperiment();
    void writeTabs();
    void writeDevices();
    void writeFigureWindows();
    void writeWidgets();
    void writeConnections();
    void writeState();
};

