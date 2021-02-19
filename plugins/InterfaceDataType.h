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

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
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
#include "boost/variant.hpp"
#include <boost\shared_ptr.hpp>

#ifndef INTERFACEFATA_H
#define INTERFACEFATA_H

#define DataPair std::pair<boost::shared_ptr<std::vector<double>>, boost::shared_ptr<std::vector<double>>>
#define GuiSelection std::pair<QString, QStringList>
#define DataTypes boost::blank, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, bool, QString, double,  QStringList, GuiSelection, DataPair


class InterfaceData{
    public:
    InterfaceData(){}
    InterfaceData(QString DataTypeI, QString TypeI)
    {
        DataType = DataTypeI;
        Type = TypeI;
    }
    // Copy constructor
    InterfaceData(const InterfaceData &Obj) { DataType = Obj.DataType;
                                              StateDependency = Obj.StateDependency;
                                              Type = Obj.Type;
                                              m_Data = Obj.m_Data; };

    bool IsEditable();

    void SetDataType(QString);
    void SetType(QString);

    bool IsUnsigedNumber();
    bool IsSigedNumber();
    bool IsBool();
    bool IsFloatingPointNumber();

    bool IsNumeric();

    bool IsStringList();
    bool IsString();
    bool IsGuiSelection();

    bool IsPairOfVectorOfDoubles();

    void SetDataKeepType(int8_t);
    void SetDataKeepType(int16_t);
    void SetDataKeepType(int32_t);
    void SetDataKeepType(int64_t);
    void SetDataKeepType(uint8_t);
    void SetDataKeepType(uint16_t);
    void SetDataKeepType(uint32_t);
    void SetDataKeepType(uint64_t);
    void SetDataKeepType(float);
    void SetDataKeepType(bool);
    void SetDataKeepType(QString);
    void SetDataKeepType(double);
    void SetDataKeepType(GuiSelection);
    void SetDataKeepType(QStringList);

    uint64_t GetUnsignedData();
    int64_t GetSignedData();
    double GetFloatingPointData();
    int64_t GetInt64_tData();
    int32_t GetInt32_tData();
    int16_t GetInt16_tData();
    int8_t GetInt8_tData();
    uint64_t GetUInt64_tData();
    uint32_t GetUInt32_tData();
    uint16_t GetUInt16_tData();
    uint8_t GetUInt8_tData();
    int32_t GetInt(){ return GetInt32_tData(); };

    bool GetBool();
    float GetFloat();
    double GetDouble();

    double GetAsDouble();


    void SetData(DataPair Data);
    void SetData(QString);
    void SetData(uint64_t);
    void SetData(uint32_t);
    void SetData(uint16_t);
    void SetData(uint8_t);
    void SetData(int64_t);
    void SetData(int32_t);
    void SetData(int16_t);
    void SetData(int8_t);
    void SetData(double);
    void SetData(float);
    void SetData(bool);
    void SetData(QStringList);
    void SetData(GuiSelection);

    void SetDataRaw( boost::variant<DataTypes>);



    QString GetString();
    DataPair GetPointerPair();
    void SetStateDependency(QString);
    QString GetStateDependency();
    QString GetDataType();
    QString GetType();
    QString GetTypeInfo();
    QStringList GetStringList();
    GuiSelection GetGuiSelection();

    boost::variant<DataTypes> GetData(){return m_Data;}
    boost::variant<DataTypes> GetDataRaw(){return m_Data;}

private:
    QString DataType; //double, int, boolean, GuiSelection ...
    QString StateDependency;
    QString Type;
    boost::variant<DataTypes> m_Data;


};


#endif // MAPPER

