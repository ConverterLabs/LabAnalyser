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

#ifndef DataManagementClass_H
#define DataManagementClass_H
#include <QMainWindow>
#include <map>
#include "mapper.h"
#include "../plugins/InterfaceDataType.h"

class Platform_Interface;

/** This Class contains all data that is necessary to mirror the complete data State of LabAnalyser, including the plugin handling.
 *
 *  @author Andreas Hoffmann
 *  @date 16.03.2018
 *
 *  @version 0.2
 *  Neu Strukturiert.
 *
 *  @version 0.1
 *  Kommentare hinzugefügt (Doxygen).
 */

class DataManagementClass : public QObject
{
   Q_OBJECT
public:

    DataManagementClass(QObject* parent = 0);

    /** @return Number of open PlotWidgets */
    int PlotCount(void);
    /** @return First unique Plot Number */
    int GetUniquePlotNumber(void);

    /** @param Name -  Name of the Plot QObject
    * @return Pointer of the plot object with the name Name */
    QObject* GetPlotByName(QString Name);

    /** Adds a new Plotobject to the Data Management
     * @param Name -  Name of the Plot QObject
     * @param object -  Pointer of the Plot QObject
     * @param number -  Number of Plot object */
    void AddPlotPointer(QString id, QObject* pointer);
    void AddPlotPointer(QString Name, QObject* object, int number);

    /** Renames a Plotobject
     * @param IdOld -  old Name of the Plot QObject
     * @param IdNew -  new Name of the Plot QObject */
    void RenamePlotPointer(QString IdOld, QString IdNew);

    /** Adds the Plot Window with the objectname id and its column und row count to the management
     * @param Name -  Name of the Window QObject
     * @param rows -  number of rows in the Subplot
     * @param cols -  number of columns in the Subplot */
    void AddPlotWindow(QString Name, int rows, int cols );
    void AddPlotWindow(QString id, int rows, int cols, int number );
    /** Deletes a managed plot pointer
     * @param Name -  Name of the Plot QObject*/
    void DeletePlotPointer(QString);

    /** Deletes a managed plot window
     * @param Name -  Name of the Window QObject*/
    void DeletePlotWindow(QString);

    /** Returns the number of Rows and Columns the the PlotWindow with the name Name
     * @param Name -  Name of the Plot QObject
       @return Pair of (Rows, Colums)*/
    std::pair<int,int> GetPlotWindowRowsCols(QString name);

    /** Returns the total number of created plot windows. It is used to Create a unique plotobject name
         @return number of created plot Windows*/
    int GetPlotWindowsIncrementer();

    /** Returns the Interface to a loaded dll
     *   @param Filename - Path of the corresponding xml
         @return DLL-Interface*/
    Platform_Interface* GetDevice(QString Filename);

    /** Adding a User Interface (*.ui) Form File to the Datamanagement
     * @param std::pair(FormName, UiFileName)  containing first the filename second the Formname*/
    void AddFormFile(std::pair<QString, QString> FormName_FileName);

    /** Marks that a Form File has to skip the loading of custum label text
     * @param  Filename - Name of the Form
     * @param  skip - true if skip*/
    void AddSkipFormFile(QString Filename, bool skip );

    /** Return true if the label texts on the form shell be skipped
     * @param  Filename - Name of the Form
     * @return  true if skip*/
    bool GetSkipFormFile(QString Filename );

    /** Remove a Userinterface Form form the Datamanagement
     * @param Objectname of the form to be removed*/
    void RemoveFormFile(QString);

    /** Returns number of loaded UI-Forms
     * @return number of loaded UI-Forms*/
    int GetFormFileCount(void);

    /** Returns the i-th Form Name and its corresponding path
     * @return std::pair (FormName, UiFileName)*/
    std::pair<QString, QString> GetFormFileEntry(int);

    /** Returns the class of the qwidget: label, spinbox...
     * @param Pointer of the object Object
     * @return Class of the object Object*/
    QString GetObjectType(QObject* Object);

    /** Returns the number of managed widget files...
     * @return number of managed widget files*/
    int GetContainerCount();

    /** Returns the number of managed widget files...
     * @param ElementName - unique Identidier of the data, that will be connected
       @param ContainerID - name of the widget that will be connected to ElementName
       @param ClassName   - class of the connected widget
       @param object      - pointer of the connected widget */
    void AddElementToContainerEntry(QString ElementName, QString ContainerID, QString ClassName, QObject* object);

    /** Returns true if the widget is connected to unique id
     * @param pointer of the connected widget
       @return true if the widget is connected*/
    bool IsObjectLinked(QObject*);

    /** If one of the input arguments is matches a stored one, the connection is deleted
      * @param id - unique data identifier
      * @param Object - Pointer to widget*/
    void DeleteEntryOfObject(QString id, QObject* Object);

    /** If widget pointer matches a stored one, the connection is deleted
      * @param Object - Pointer to widget*/
    void DeleteEntryOfObject(QObject* Object);

    /** Gets the unique data identifier, that is connected to the widget with the name ElementName
     *  @param ElementName - objectname of an widget
        @return unique data identifier
    */
    QString GetContainerID(QString ElementName);

    /** Gets the unique data identifier, that is connected to the widget with the given pointer
     *  @param  pointer of an widget
        @return unique data identifier
    */
    QString GetContainerID(QObject*);

    /** Returns the container that holds the data information for a given widget
     *  @param  pointer of an widget
        @return data container
    */
    ToFormMapper* GetContainer(QObject*);

    /** Returns the container that holds the data information for a given widget
     *  @param  objectname of the widget
        @return data container
    */
    ToFormMapper* GetContainer(QString);

    /** Returns the i-th unique identifier and all names of the therewith connected widgets
     *  @param  number of container index
        @return unique id and list of connected widget names
    */
    std::pair<QString, std::vector<QString>> GetContainerElementForms(int i);

    /** Returns the InterfaceData that is connected with the widget pointer
     *  @param  pointer to widget
        @return InterfaceData
    */
    InterfaceData GetInterfaceData(QObject* Object);

    /** Loads a new Plugin (dll) to LabAnalyser
     *  @param  path to the xml file that contains the dll path and necessary parameters for the dll-work
        @return returns 0, when loading was successful
    */
    int LoadPlugin(QString FilenameXML);

    /** Returns a list of all loaded dll paths
        @return list of the names or paths of the loaded dlls    */
    QList<QString> GetDevices(void);    
    QList<QString> GetDevicePaths();


    /** Adds a new loades Device to the data management
     *  @param  Filename - unique name of the plugin
        @param  Device   - pointer to the loaded dll Interface
     *  @param  Filepath - path to the xml file that contains the dll path and necessary parameters for the dll-work
    */    void AddDevice(QString Name, QString Filepath, Platform_Interface* Device);

    /** Removes all loades dlls*/
    void RemoveDevices(void);

    /** Closes the device with the name dev*/
    void CloseDevice(QString dev);

    /** Deletes all new variables
    */
    void CloseProjectLogic(void);   

    /** Check if unique id is already registered
     *  @param ID - unique id of the Data
        @return true when already known
    */
    bool ElementExists(QString ID);

    /** Sets an alias to the unique id
     *  @param ID - unique id of the Data
        @param Alias - new alias, shown e.g. in plot legends
    */
    void SetAlias(QString ID, QString Alias);

    /** Returns the alias for the unique id
     *  @param ID - unique id of the Data
        @return Alias - alias of the unique id
    */
    QString GetAlias(QString ID);

    /** Sets the Min and Max value of the data with the unique id
     *  @param ID - unique id of the Data
        @param Min - minimal value
        @param Max - maximum value
    */
    void SetMinMaxValue(QString ID, double Min,double Max);

    /** Gets the Min and Max value of the data with the unique id
     *  @param ID - unique id of the Data
        @return std::pair (Min,Max)
    */
    std::pair<double, double> MinMaxValue(QString ID);


    std::map<QString, ToFormMapper*> *GetContainerPointer(){ return &Container;}

public slots:

    /** Sets a virant variable to the unique id indexed data container
     *  @param ID - unique id of the Data
        @param InterfaceData - new Data
    */
    void SetData(const QString &ID, InterfaceData Data);

    /** Adds a new unique data id to the known Id Container list
     *  @param ID - unique id of the Data
        @param DataType - QString of known DataTypes int double...
        @param Type - type of the data, { Parameter, Data, State... }
        @param StateDependency is the setting of the data dependent on a StateVariable
    */
    void AddContainerElement(QString ID,QString DataType, QString Type,QString StateDependency );


signals:
    /** Signal the Close Project event*/
    void CloseProject(void);
    /** Send a message via the connected Messenger*/
    void MessageSender(const QString &Command, const QString &ID, InterfaceData Data);
    /** Send an Info message to the output Window via the connected Messenger*/
    void Info(QString text);
    /** Send an Error message to the output Window via the connected Messenger*/
    void Error(QString text);

    void PublishFinished();

private:

    /** Vector that contains the connection between (FormName and UiFileName)*/
    std::vector<std::pair<QString, QString>> FormFiles;
    /** Map that contains the information if FormName has user defined ui-labels*/
    std::map<QString,bool> SkipFormFiles;

    /** Map that contains the information of the loaded plugin paths and the corresponding Interface Pointers*/
    std::map<QString, Platform_Interface*> _Devices;
    /** Map that contains the path that belongs to the device name*/
    std::map<QString, QString> _Devicepaths;

    QString GetDevicePath(QString Name);


    /** Map that contains the information of the loaded plotnames and the corresponding pointers*/
    std::map<QString, QObject* > PlotObjects;
    std::map<QString, int > PlotObjectsNumber;
    /** std::vector which holds the PlotObject Numbers*/
    std::vector<int> PlotObjectsNumbers;


    /** Map that contains the information of the loaded PlotWindowNames and the corresponding pointers*/
    std::map<QString, std::pair<int,int>> PlotWindows;
    /** Number of total plot Window openings*/
    int PlotWindowsIncrementer = 0;
    std::map<QString, int > PlotWindowNumber;
    /** std::vector which holds the PlotObject Numbers*/
    std::vector<int> PlotWindowNumbers;


    /** Map that contains the connection between the objectname of an widget and the connected unique data identifier*/
    std::map<QString, QString> ElementsToContainerID;
    /** Map that contains the connection between the connected unique data identifier and its ToFormMapper Object */
    std::map<QString, ToFormMapper*> Container;
    /** Map that contains the connection between the unique data identifier and its alias */
    std::map<QString, QString> AliasMap;

};
#endif // GUIDataInterface_H
