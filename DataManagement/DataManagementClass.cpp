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
#include "ContainerStore.h"
#include "DataRegistry.h"
#include "WidgetBindingRegistry.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>
#include "../plugins/platforminterface.h"


DataManagementClass::DataManagementClass(QObject* parent): QObject(parent), Registry(new DataRegistry), Containers(new ContainerStore), Bindings(new WidgetBindingRegistry)
{
    connect(this, SIGNAL(CloseProject()),this->parent(),SLOT(CloseProject()));
}

void DataManagementClass::DataRegistryDeleter::operator()(DataRegistry* registry) const
{
    delete registry;
}

void DataManagementClass::ContainerStoreDeleter::operator()(ContainerStore* store) const
{
    delete store;
}

void DataManagementClass::WidgetBindingRegistryDeleter::operator()(WidgetBindingRegistry* registry) const
{
    delete registry;
}

int DataManagementClass::PlotCount()
{
    return Registry->PlotCount();
}

int DataManagementClass::GetUniquePlotNumber()
{
    return Registry->GetUniquePlotNumber();

}

void DataManagementClass::AddPlotPointer(QString id, QObject* pointer)
{
    Registry->AddPlotPointer(id, pointer);
}

void DataManagementClass::AddPlotPointer(QString id, QObject* pointer, int number)
{
    Registry->AddPlotPointer(id, pointer, number);

}

void DataManagementClass::RenamePlotPointer(QString IdOld, QString IdNew)
{
    Registry->RenamePlotPointer(IdOld, IdNew);
}

void DataManagementClass::AddPlotWindow(QString id, int rows, int cols, int number )
{
    Registry->AddPlotWindow(id, rows, cols, number);
}


void DataManagementClass::AddPlotWindow(QString id, int rows, int cols )
{
    Registry->AddPlotWindow(id, rows, cols);
}

void DataManagementClass::DeletePlotPointer(QString id )
{
    Registry->DeletePlotPointer(id);

}

QObject* DataManagementClass::GetPlotByName(QString Name)
{

    return Registry->GetPlotByName(Name);
}

void DataManagementClass::DeletePlotWindow(QString id )
{
    Registry->DeletePlotWindow(id);

}

void DataManagementClass::AddFormFile(std::pair<QString, QString> Filename )
{
   Registry->AddFormFile(Filename);
}

void DataManagementClass::AddSkipFormFile(QString Filename, bool skip )
{
   Registry->AddSkipFormFile(Filename, skip);
}

bool DataManagementClass::GetSkipFormFile(QString Filename)
{
    return Registry->GetSkipFormFile(Filename);
}

void DataManagementClass::RemoveFormFile(QString Filename )
{
    Registry->RemoveFormFile(Filename);
}


bool DataManagementClass::ElementExists(QString ElementName)
{
    //Check if map element exists
    auto itt = Containers->Map().find(ElementName);
    if(itt != Containers->Map().end())
    {
        if(Containers->LookupOrInsert(ElementName) != NULL)
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
    if(Bindings->Contains(ElementName) && ClassName.compare("PlotWidget"))
    {
        DeleteEntryOfObject(object);
    }
    //Add connection between ElementName (widget name) and unique data id ContainerID
    Bindings->Set(ElementName, ContainerID);
    ToFormMapper* Element = Containers->LookupOrInsert(ContainerID);
    //Add widget unique data id
    Element->Objects.push_back(ObjectStruct(ElementName,object,ClassName));
}

QString DataManagementClass::GetContainerID(QString ElementName)
{    
    return Bindings->LookupOrInsert(ElementName);
}

QString DataManagementClass::GetContainerID(QObject* Object)
{
    //Check if map element exists
    return Bindings->Find(Object->objectName());
}

ToFormMapper* DataManagementClass::GetContainer(QObject* Object)
{
    return Containers->LookupOrInsert(GetContainerID(Object->objectName()));
}

ToFormMapper* DataManagementClass::GetContainer(QString ContainerID)
{
    return Containers->Find(ContainerID);
}

InterfaceData DataManagementClass::GetInterfaceData(QObject* Object)
{
    ToFormMapper* Element = Containers->LookupOrInsert(GetContainerID(Object->objectName()));
    InterfaceData Data(Element->GetDataType(),Element->GetType());
    Data.SetDataRaw(Element->GetData());

    return Data;
}

QString DataManagementClass::GetObjectType(QObject* Object)
{
    ToFormMapper* Element = Containers->LookupOrInsert(GetContainerID(Object->objectName()));
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
    ToFormMapper* DataC = Containers->LookupOrInsert(id);
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
    QString id;
    if(Bindings->Take(Object->objectName(), &id))
    {
        DeleteEntryOfObject(id,Object);
    }
 }

void DataManagementClass::AddContainerElement(QString ID,QString DataType, QString Type,QString StateDependency )
{
    Containers->AddOrReplace(ID, DataType, Type, StateDependency);
}

int DataManagementClass::GetFormFileCount(void)
{
    return Registry->GetFormFileCount();
}

std::pair<QString, QString> DataManagementClass::GetFormFileEntry(int i)
{
    return Registry->GetFormFileEntry(i);
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
    _Devices.clear();

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
    Registry->Clear();

    for(auto itt : _Devices)
        if(itt.second)
            delete itt.second;
    _Devices.clear();
    _Devicepaths.clear();

    Containers->Clear();

    Bindings->Clear();
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
    return Registry->GetPlotWindowRowsCols(Name);
}

int DataManagementClass::GetContainerCount(void)
{
    return Containers->Count();
}

std::pair<QString, std::vector<QString>> DataManagementClass::GetContainerElementForms(int i)
{
    int r = 0;
    std::vector<QString> Elements;
    //itterate ober all elements
    for(auto itt : Containers->Map())
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

std::map<QString, ToFormMapper*>* DataManagementClass::GetContainerPointer()
{
    return Containers->MapAddress();
}

int DataManagementClass::GetPlotWindowsIncrementer()
{

    return Registry->GetPlotWindowsIncrementer();
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
    return Bindings->Contains(Obj->objectName());
}

void DataManagementClass::SetAlias(QString ID, QString Alias)
{
    Registry->SetAlias(ID, Alias);
}

QString DataManagementClass::GetAlias(QString ID)
{
    return Registry->GetAlias(ID);
}

void DataManagementClass::SetData(const QString &ID, InterfaceData Data)
{
    if(this->GetContainer(ID))
        this->GetContainer(ID)->SetDataRaw(Data.GetData());
}
