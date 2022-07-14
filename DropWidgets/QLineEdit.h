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
#include "QTimer"



class QLineEditD : public QLineEdit, public VariantDropWidget
{
Q_OBJECT
public:
    QLineEditD(QWidget *parent=0);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *de);
    virtual void dropEvent(QDropEvent *event);

    void SetVariantData(ToFormMapper Data) override;
    void GetVariantData(ToFormMapper *Data) override;

    bool LoadFromXML(const std::vector<std::pair<QString, QString>> &Attributes, const QString &Text) override;
    bool SaveToXML(std::vector<std::pair<QString, QString>> &Attributes, QString &Text) override;
    void ConnectToID(DataManagementSetClass* DM, QString ID) override;

Q_SIGNALS:
    void RequestUpdate();
public slots:
    void contextMenu(QPoint);
    void RemoveConnection();

};
