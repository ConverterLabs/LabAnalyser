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
#ifndef LoaderClass_H
#define LoaderClass_H

#include <QObject>
#include <QStringList>
#include "DataManagementSetClass.h"
#include "DataMessengerClass.h"


class UIDataManagementSetClass : public DataManagementSetClass
{
    Q_OBJECT
public:
    explicit UIDataManagementSetClass(QObject *parent = nullptr);
    bool LoadExperiment(QString Path);

    bool Export2Mat(QString Path, QStringList);
    bool Export2Hdf5(QString Path, QStringList);

    bool Export2Xml(QString Path,  QStringList);
    bool ImportFromXml(QString Path  );

    bool LoadPlugin(QString Path);

    int  RegisterChange(){ ChangeDetected = true;}

    void LoadForms();

public slots:
    bool SaveExperiment(QString Path);


signals:
    void LoadFormFromXML(QString UiFileName, QString LastFormName, bool skip);

private:
    ~UIDataManagementSetClass();
    bool ChangeDetected = true;
    QString LoadPath;
    QString StdSavePath;

};

#endif // LoaderClass_H
