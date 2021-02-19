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

#include "DataManagementClass.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>
#include "../plugins/platforminterface.h"


DataManagementClass::DataManagementClass(QObject* parent): QObject(parent)
{
    connect(this, SIGNAL(CloseProject()),this->parent(),SLOT(CloseProject()));
}

int DataManagementClass::PlotCount()
{
    return (int)(this->PlotObjects.size());
}

int DataManagementClass::GetUniquePlotNumber()
{
    int LastNumber = 0;
    if(!PlotObjectsNumbers.size())
        return 0;
    if(PlotObjectsNumbers[0] != 0)
        return 0;
    for(auto index = 1; index < PlotObjectsNumbers.size(); index++)
    {
        if (PlotObjectsNumbers[index] - LastNumber > 1)
        {
            break;
        }
        LastNumber= PlotObjectsNumbers[index];
    }
    return LastNumber+1;

}

void DataManagementClass::AddPlotPointer(QString id, QObject* pointer)
{
    this->PlotObjects[id] = pointer;
}

void DataManagementClass::AddPlotPointer(QString id, QObject* pointer, int number)
{
    this->PlotObjects[id] = pointer;
    PlotObjectsNumber[id]  = number;
    PlotObjectsNumbers.push_back(number);
    std::sort(PlotObjectsNumbers.begin(), PlotObjectsNumbers.end());

}

void DataManagementClass::RenamePlotPointer(QString IdOld, QString IdNew)
{
    auto it = this->PlotObjects.find(IdOld);
    auto itNumber = this->PlotObjectsNumber.find(IdOld);

    if(it != this->PlotObjects.end())
    {
        auto data =  this->PlotObjects[IdOld];
        this->PlotObjects.erase (it);

        if(itNumber != this->PlotObjectsNumber.end())
        {
            auto number = this->PlotObjectsNumber[IdOld];
            this->PlotObjectsNumber.erase(itNumber);
            auto itNumber_vec = find (PlotObjectsNumbers.begin(), PlotObjectsNumbers.end(), number);
            this->PlotObjectsNumbers.erase(itNumber_vec);
            AddPlotPointer(IdNew, data, number);
        }
        else
            AddPlotPointer(IdNew, data);
    }
}

void DataManagementClass::AddPlotWindow(QString id, int rows, int cols, int number )
{
    this->PlotWindows[id] = std::pair<int,int>(rows,cols);
    this->PlotWindowNumber[id]  = number;
    this->PlotWindowNumbers.push_back(number);
    std::sort(PlotWindowNumbers.begin(), PlotWindowNumbers.end());

    PlotWindowsIncrementer++;
}


void DataManagementClass::AddPlotWindow(QString id, int rows, int cols )
{
    this->PlotWindows[id] = std::pair<int,int>(rows,cols);
    PlotWindowsIncrementer++;
}

void DataManagementClass::DeletePlotPointer(QString id )
{
    //Check if map element exists

    auto it = this->PlotObjects.find(id);
    if(it != this->PlotObjects.end())
        this->PlotObjects.erase (it);

    auto it2 = this->PlotObjectsNumber.find(id);
    if(it2 != this->PlotObjectsNumber.end())
    {
        auto number = PlotObjectsNumber[id];
        this->PlotObjectsNumber.erase(it2);
        auto itNumber_vec = find (PlotObjectsNumbers.begin(), PlotObjectsNumbers.end(), number);
        this->PlotObjectsNumbers.erase(itNumber_vec);
    }

}

QObject* DataManagementClass::GetPlotByName(QString Name)
{

    //Itterate over all map elements
    for(auto e : PlotObjects)
            if(e.second->objectName().compare(Name) == 0)
                return e.second;

    return NULL;
}

void DataManagementClass::DeletePlotWindow(QString id )
{
    //Check if map element exists
    auto it = this->PlotWindows.find(id);
    if(it != this->PlotWindows.end())
        this->PlotWindows.erase (it);

    auto it2 = this->PlotWindowNumber.find(id);
    if(it2 != this->PlotWindowNumber.end())
    {
        auto number = PlotWindowNumber[id];
        this->PlotWindowNumber.erase(it2);
        auto itNumber_vec = find (PlotWindowNumbers.begin(), PlotWindowNumbers.end(), number);
        this->PlotWindowNumbers.erase(itNumber_vec);
    }

}

void DataManagementClass::AddFormFile(std::pair<QString, QString> Filename )
{
   this->FormFiles.push_back(Filename);
}

void DataManagementClass::AddSkipFormFile(QString Filename, bool skip )
{
   this->SkipFormFiles[Filename] = skip;
}

bool DataManagementClass::GetSkipFormFile(QString Filename)
{
    //Check if map element exists
    auto itt = this->SkipFormFiles.find(Filename);
    if(itt != this->SkipFormFiles.end())
    {
       return  this->SkipFormFiles[Filename];
    }
    return false;
}

void DataManagementClass::RemoveFormFile(QString Filename )
{
    //Search for the corresponding vector element
    int j=-1;
    for(int i = 0; i < this->FormFiles.size() && j ==-1; i++)
    {
        if(FormFiles[i].first.compare(Filename)==0)
            j = i;
    }
    //if found delete
    if(j != -1)
        FormFiles.erase(FormFiles.begin()+j);
}


bool DataManagementClass::ElementExists(QString ElementName)
{
    //Check if map element exists
    auto itt = this->Container.find(ElementName);
    if(itt != this->Container.end())
    {
        if(this->Container[ElementName] != NULL)
            return true;
    }
    return false;
}


void DataManagementClass::AddElementToContainerEntry(QString ElementName, QString ContainerID, QString ClassName, QObject* object)
{
    //Check if map element exists
    ToFormMapper* DataC = GetContainer(ContainerID);
    if(DataC)
    {
        for(auto el: DataC->Objects)
        {
            if(el.FormP == this)
                return;
        }
    }
    //Check if the widget is mapped to other element and delete this connection when exists and not a graph
    auto itt = this->ElementsToContainerID.find(ElementName);
    if(itt != this->ElementsToContainerID.end() && ClassName.compare("PlotWidget"))
    {
        DeleteEntryOfObject(object);
    }
    //Add connection between ElementName (widget name) and unique data id ContainerID
    this->ElementsToContainerID[ElementName] = ContainerID;
    ToFormMapper* Element = this->Container[ContainerID];
    //Add widget unique data id
    Element->Objects.push_back(ObjectStruct(ElementName,object,ClassName));
}

QString DataManagementClass::GetContainerID(QString ElementName)
{    
    return this->ElementsToContainerID[ElementName];
}

QString DataManagementClass::GetContainerID(QObject* Object)
{
    //Check if map element exists
    auto itt = this->ElementsToContainerID.find(Object->objectName());
    if( itt  != this->ElementsToContainerID.end())
        return this->ElementsToContainerID[Object->objectName()];

    return QString();
}

ToFormMapper* DataManagementClass::GetContainer(QObject* Object)
{
    return this->Container[GetContainerID(Object->objectName())];
}

ToFormMapper* DataManagementClass::GetContainer(QString ContainerID)
{
    //Check if map element exists
    if(!this->Container.size())
        return nullptr;
    auto itt = this->Container.find(ContainerID);
    if( itt  == this->Container.end())
        return nullptr;
    return this->Container[ContainerID];
}

InterfaceData DataManagementClass::GetInterfaceData(QObject* Object)
{
    ToFormMapper* Element = Container[GetContainerID(Object->objectName())];
    InterfaceData Data(Element->GetDataType(),Element->GetType());
    Data.SetDataRaw(Element->GetData());

    return Data;
}

QString DataManagementClass::GetObjectType(QObject* Object)
{
    ToFormMapper* Element = Container[GetContainerID(Object->objectName())];
    QString FormType;
    for(int i = 0; i <Element->Objects.size();i++)
    {
        if(Element->Objects[i].FormName.compare(Object->objectName())==0)
           return FormType.append(Element->Objects[i].FormType);
    }
      return FormType;
}

void DataManagementClass::DeleteEntryOfObject(QString id, QObject* Object)
{
    ToFormMapper* DataC = Container[id];
    if(DataC)
    {
        for(int i = 0; i < DataC->Objects.size();i++)
        {
            if(DataC->Objects[i].FormP == Object || DataC->Objects[i].FormName.compare(Object->objectName())==0)
            {
                DataC->Objects.erase(DataC->Objects.begin()+i);
            }
        }
    }
}

void DataManagementClass::DeleteEntryOfObject(QObject* Object)
{
    //Check if map element exists
    auto itt = this->ElementsToContainerID.find(Object->objectName());
    if(itt != this->ElementsToContainerID.end())
    {
        QString id = this->ElementsToContainerID[Object->objectName()];
        this->ElementsToContainerID.erase(this->ElementsToContainerID.find(Object->objectName()));
        DeleteEntryOfObject(id,Object);
    }
 }

void DataManagementClass::AddContainerElement(QString ID,QString DataType, QString Type,QString StateDependency )
{
    if(!this->GetContainer(ID))
    {
        this->Container[ID] = new ToFormMapper(DataType,Type);
        ((ToFormMapper*) this->Container[ID])->SetStateDependency(StateDependency);
    }
    else
    {

        ToFormMapper *P =  this->Container[ID];
        this->Container[ID] = new ToFormMapper(DataType,Type);
        ((ToFormMapper*) this->Container[ID])->SetStateDependency(StateDependency);
        this->Container[ID]->Objects = P->Objects;
        this->Container[ID]->MaxValue = P->MaxValue;
        this->Container[ID]->MinValue =  P->MinValue;
        delete P;
    }
}

int DataManagementClass::GetFormFileCount(void)
{
    return (int) this->FormFiles.size();
}

std::pair<QString, QString> DataManagementClass::GetFormFileEntry(int i)
{
    return this->FormFiles[i];
}

Platform_Interface* DataManagementClass::GetDevice(QString Filename)
{
    auto itt = this->_Devices.find(Filename);
    if(itt != this->_Devices.end())
       return  this->_Devices[Filename];
    else
        return nullptr;
}

QString DataManagementClass::GetDevicePath(QString Name)
{
    auto itt = this->_Devicepaths.find(Name);
    if(itt != this->_Devicepaths.end())
       return  this->_Devicepaths[Name];
    else
        return nullptr;
}


void DataManagementClass::AddDevice(QString Filename, QString Filepath, Platform_Interface* Device)
{
    if(!this->_Devices[Filename])
    {
        this->_Devices[Filename] = Device;
        this->_Devicepaths[Filename] = Filepath;
    }
}

void DataManagementClass::RemoveDevices(void)
{
    for(auto el : _Devices)
    {
        if(el.second)
            delete el.second;
    }
}

void DataManagementClass::CloseDevice(QString dev)
{
    auto itt = this->_Devices.find(dev);
    if(itt != this->_Devices.end())
    {
        delete this->_Devices[dev];
        this->_Devices.erase(itt);

        auto itt = this->_Devicepaths.find(dev);
        this->_Devicepaths.erase(itt);
    }
}


void DataManagementClass::CloseProjectLogic(void)
{
    //Clean up
    FormFiles.clear();
    SkipFormFiles.clear();

    for(auto itt : _Devices)
        if(itt.second)
            delete itt.second;
    _Devices.clear();
    _Devicepaths.clear();

    for(auto itt : Container)
        if(itt.second)
            delete itt.second;
    Container.clear();

    this->PlotWindowsIncrementer = 0;
    this->PlotWindows.clear();
    this->PlotWindowNumber.clear();
    this->PlotWindowNumbers.clear();
    this->PlotObjectsNumber.clear();
    this->PlotObjectsNumbers.clear();
    this->PlotObjects.clear();
    this->PlotWindowsIncrementer = 0;

    this->ElementsToContainerID.clear();
    this->AliasMap.clear();
}


QList<QString> DataManagementClass::GetDevices()
{
    QList<QString> Liste;
    for(auto itt: this->_Devicepaths)
        Liste.push_back(itt.first);
    return Liste;
}

QList<QString> DataManagementClass::GetDevicePaths()
{
    QList<QString> Liste;
    for(auto itt: this->_Devicepaths)
        Liste.push_back(itt.second);
    return Liste;
}

std::pair<int,int>  DataManagementClass::GetPlotWindowRowsCols(QString Name)
{
    return this->PlotWindows[Name];
}

int DataManagementClass::GetContainerCount(void)
{
    return (int) Container.size();
}

std::pair<QString, std::vector<QString>> DataManagementClass::GetContainerElementForms(int i)
{
    int r = 0;
    std::vector<QString> Elements;
    //itterate ober all elements
    for(auto itt :this->Container)
    {
        //return the i-th itterator
        if(r == i)
        {
             for(auto el : itt.second->Objects)
             {
                 Elements.push_back(el.FormName);

             }
                 return std::pair<QString, std::vector<QString>>(itt.first, Elements) ;
        }
        r++;
    }
    return std::pair<QString, std::vector<QString>>();
}

int DataManagementClass::GetPlotWindowsIncrementer()
{

    int LastNumber = 0;
    if(!PlotWindowNumbers.size())
        return 0;
    if(PlotWindowNumbers[0] != 0)
        return 0;
    for(auto index = 1; index < PlotWindowNumbers.size(); index++)
    {
        if (PlotWindowNumbers[index] - LastNumber > 1)
        {
            break;
        }
        LastNumber= PlotWindowNumbers[index];
    }
    return LastNumber+1;
}

void DataManagementClass::SetMinMaxValue(QString ID, double Min, double Max)
{   
    this->GetContainer(ID)->MaxValue  = Max;
    this->GetContainer(ID)->MinValue  = Min;
}

std::pair<double, double> DataManagementClass::MinMaxValue(QString ID)
{
    return std::pair<double, double>(this->GetContainer(ID)->MinValue,this->GetContainer(ID)->MaxValue);
}

bool DataManagementClass::IsObjectLinked(QObject* Obj)
{
    //Check if the object is linked
    auto itt = this->ElementsToContainerID.find(Obj->objectName());
    if(itt != this->ElementsToContainerID.end())
    {
       return true;
    }
    return false;
}

void DataManagementClass::SetAlias(QString ID, QString Alias)
{
    AliasMap[ID ] = Alias;
}

QString DataManagementClass::GetAlias(QString ID)
{
    //Check if map element exists
    auto it = this->AliasMap.find(ID);
    if(it != this->AliasMap.end())
    {
        return AliasMap[ID ] ;
    }
    else
        return ID;
}

void DataManagementClass::SetData(const QString &ID, InterfaceData Data)
{
    if(this->GetContainer(ID))
        this->GetContainer(ID)->SetDataRaw(Data.GetData());
}
