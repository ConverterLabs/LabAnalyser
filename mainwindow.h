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

#include <QMainWindow>
#include "DataManagement/mapper.h"
#include "plugins/platforminterface.h"
#include "UIFunctions/SubPlotMainWindow.h"
#include "RemoteControl/RemoteControlServer.h"
#include "DataManagement/UIDataManagementSetClass.h"
#include "ui_About.h"
#include <QAction>


class TreeWidgetItem : public QTreeWidgetItem {
  public:
  TreeWidgetItem(QTreeWidget* parent = NULL):QTreeWidgetItem(parent){}
  private:
  bool operator<(const QTreeWidgetItem &other)const {
     int column = treeWidget()->sortColumn();
     bool ok = true;
     text(column).toInt(&ok);
     if(ok)
          other.text(column).toInt(&ok);
     if(ok)
          return text(column).toInt(&ok) <  other.text(column).toInt(&ok);
     else
          return text(column) < other.text(column);
  }
};

class MyUrlHandler : public QObject
{
  Q_OBJECT
public:
  MyUrlHandler(QObject* parent=0):QObject(parent){}
public slots:
  void handleUrl(const QUrl &url)
  {
      qDebug() << url.toString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(  url.toString().toLatin1() ));
  }
};

namespace Ui {
class MainWindow;
}

class QSimpleUpdater;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QStatusBar* GetStatusBar();
    UIDataManagementSetClass* GetLogic(void){return ExtendedDataManagement ; }
    bool ChangeForSaveDetected = false;
    SubPlotMainWindow* CreateSubPlotWindow(int rows, int cols, bool IsFFT = false);
    void CreateFFTPlotWindow();
    Ui::MainWindow* UI(){return ui;}
    QString StdSavePath;
    QString SavePath;


void DeleteFigure(SubPlotMainWindow* FigurePointer);

    ~MainWindow();

public slots:

    void Info(QString text);
    void Error(QString text);
    void closeEvent ( QCloseEvent * event );
    void CloseProject();
    void resizeEvent(QResizeEvent* event);
    void changeEvent(QEvent *e);
    void dockWidget_topLevelChanged(bool isFloating);
    void dockWidget_destroyed(QObject*);
    void HighLightConnection(QString);
    void RemoveConnection(QString);
    void TrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void AddElementToWidget(QString ID, InterfaceData Data);
    void LoadFormFromXML(QString Path,QString LastFormName, bool skip = false);
    void LoadFormFromXML(QString UiFileName);
    void PublishFinished();
    void PublishStart();
    void OutputTextMenu( QPoint );
    void ErrorWriter(const QString &ID, const QString Data);
    void InfoWriter(const QString &ID, const QString Data);



signals:

    void MessageSender(const QString &Command, const QString &ID, InterfaceData Data);          //Extra Klasse
    void SaveExperiment(QString Path);

private slots:


    void AppendWidgetNames(QObjectList,QString);
    void ChangeMinMaxValue();
    void RemoveDevice();
    void SetAlias(QString);
    void RemoveAlias(QString);

    void contextMenuTreeWidget(QPoint);
    void contextMenuTreeWidgetData(QPoint);
    void contextMenuTreeWidgetState(QPoint);
    void on_actionLoad_Form_triggered();
    void on_actionBeenden_triggered();
    void on_actionCreatePlot_triggered();
    void on_actionCreate_Subplot_triggered();
    void on_actionDaten_Exportieren_mat_triggered();
    void on_actionSave_triggered();
    void on_actionSave_Experiment_triggered();
    void on_actionLoadExperiment_triggered();
    void on_Close_Project_triggered();
    void on_actionLoadPlugin_triggered();
    void on_actionMinimize_to_Tray_triggered();
    void NotificationWriter(const QString &ID, const QString Data);
    void on_actionLoad_Parameter_File_triggered();
    void on_ParameterTreeWidget_customContextMenuRequested(const QPoint &pos);
    void on_StateTreeWidget_customContextMenuRequested(const QPoint &pos);
    void on_actionSave_Parameter_Set_triggered();
    void on_actionAbout_LabAnalyzer_triggered();
    void on_actionAbout_triggered();
    void on_pushButton_clicked();

    void on_actionExport_Data_h5_triggered();

    void on_actionRemote_Connection_Port_2_triggered();

    void on_actionFFT_triggered();

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:

    void SelectedItems(QStringList  &Ids, QList<QTreeWidgetItem*> selit);
    void AddSelectedItems(QTreeWidgetItem* elemtent, QStringList& itt);


    void ParseInputArguments();
    Ui::MainWindow *ui;
    void RemoveElementFromWidget(QString ID);
    int SubPlotWindowCount = 0;
    RemoteControlServer *Remote = NULL;
    QSystemTrayIcon* icon;
    QAction* restore;
    UIDataManagementSetClass *ExtendedDataManagement;
    bool isloading = false;

};

