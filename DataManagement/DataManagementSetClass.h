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
#ifndef DATASETTERCLASS_H
#define DATASETTERCLASS_H

#include <QObject>
#include "DataManagementClass.h"
#include "DataMessengerClass.h"


class DataManagementSetClass : public DataManagementClass
{
    Q_OBJECT
public:
    explicit DataManagementSetClass(QObject *parent = nullptr);
    MessengerClass* GetMessenger(){return Messenger;};

public slots:
    void SetData(const QString &ID);
    void SendNewValue();
    void UpdateRequest();
    void UpdateRequest(QString ID);

private:
    MessengerClass* Messenger = nullptr;


};

#endif // DATASETTERCLASS_H
