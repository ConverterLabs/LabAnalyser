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

#include "InterfaceDataType.h"
#include <qdebug.h>

QString InterfaceData::GetType()
{
    return this->Type;
}

void InterfaceData::SetDataRaw(boost::variant<DataTypes> Data)
{
    m_Data = Data;
}

bool InterfaceData::IsEditable()
{
    if(GetType().compare("Parameter")==0)
    {
        return true;
    }
    return false;
}


uint64_t InterfaceData::GetUnsignedData()
{
    const std::type_info& type = m_Data.type();
    bool ret = false;
    if(type == typeid(uint8_t))
      return static_cast<uint64_t>(boost::get<uint8_t>(m_Data));
    else if(type == typeid(uint16_t))
        return static_cast<uint64_t>(boost::get<uint16_t>(m_Data));
    else if(type == typeid(uint32_t))
        return static_cast<uint64_t>(boost::get<uint32_t>(m_Data));
    else if(type == typeid(uint64_t))
        return static_cast<uint64_t>(boost::get<uint64_t>(m_Data));

    return 0;
}

int64_t InterfaceData::GetSignedData()
{
    const std::type_info& type = m_Data.type();
    bool ret = false;
    if(type == typeid(int8_t))
      return static_cast<int64_t>(boost::get<int8_t>(m_Data));
    else if(type == typeid(int16_t))
        return static_cast<int64_t>(boost::get<int16_t>(m_Data));
    else if(type == typeid(int32_t))
        return static_cast<int64_t>(boost::get<int32_t>(m_Data));
    else if(type == typeid(int64_t))
        return static_cast<int64_t>(boost::get<int64_t>(m_Data));

    return 0;
}

double InterfaceData::GetFloatingPointData()
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(double))
      return static_cast<double>(boost::get<double>(m_Data));
    else if(type == typeid(float))
        return static_cast<double>(boost::get<float>(m_Data));

    return 0.0;
}

bool InterfaceData::IsNumeric()
{
    return IsBool() || IsSigedNumber() || IsUnsigedNumber() || IsFloatingPointNumber();
}

double InterfaceData::GetAsDouble()
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(double))
        return static_cast<double>(boost::get<double>(m_Data));
    else if(type == typeid(float))
        return static_cast<double>(boost::get<float>(m_Data));
    else if(type == typeid(int8_t))
        return static_cast<double>(boost::get<int8_t>(m_Data));
    else if(type == typeid(int16_t))
        return static_cast<double>(boost::get<int16_t>(m_Data));
    else if(type == typeid(int32_t))
        return static_cast<double>(boost::get<int32_t>(m_Data));
    else if(type == typeid(int64_t))
        return static_cast<double>(boost::get<int64_t>(m_Data));
    else if(type == typeid(uint8_t))
        return static_cast<double>(boost::get<uint8_t>(m_Data));
    else if(type == typeid(uint16_t))
        return static_cast<double>(boost::get<uint16_t>(m_Data));
    else if(type == typeid(uint32_t))
        return static_cast<double>(boost::get<uint32_t>(m_Data));
    else if(type == typeid(uint64_t))
        return static_cast<double>(boost::get<uint64_t>(m_Data));
    else if(type == typeid(bool))
        return static_cast<double>(boost::get<bool>(m_Data));

}


bool InterfaceData::IsUnsigedNumber()
{
    const std::type_info& type = m_Data.type();
    bool ret = false;
    if(type == typeid(uint8_t))
        ret = true;
    else if(type == typeid(uint16_t))
        ret = true;
    else if(type == typeid(uint32_t))
        ret = true;
    else if(type == typeid(uint64_t))
        ret = true;
    return ret;
}

bool InterfaceData::IsString()
{
     const std::type_info& type = m_Data.type();
    if(type == typeid(QString))
        return true;
    return false;
}

bool InterfaceData::IsGuiSelection()
{
    const std::type_info& type = m_Data.type();
   if(type == typeid(GuiSelection))
       return true;
   return false;
}

bool InterfaceData::IsPairOfVectorOfDoubles()
{
   const std::type_info& type = m_Data.type();
   if(type == typeid(DataPair))
       return true;
   return false;
}

bool InterfaceData::IsSigedNumber()
{
    const std::type_info& type = m_Data.type();
    bool ret = false;
    if(type == typeid(int8_t))
        ret = true;
    else if(type == typeid(int16_t))
        ret = true;
    else if(type == typeid(int32_t))
        ret = true;
    else if(type == typeid(int64_t))
        ret = true;


    return ret;
}
bool InterfaceData::IsBool()
{
    const std::type_info& type = m_Data.type();
    bool ret = false;
    if(type == typeid(bool))
        ret = true;

    return ret;
}
bool InterfaceData::IsFloatingPointNumber()
{

    const std::type_info& type = m_Data.type();
    bool ret = false;
    if(type == typeid(float))
        ret = true;
    else if(type == typeid(double))
        ret = true;

    return ret;
}
bool InterfaceData::IsStringList()
{
    const std::type_info& type = m_Data.type();
    bool ret = false;
    if(type == typeid(QStringList))
        ret = true;


    return ret;

}

QString InterfaceData::GetTypeInfo()
{
    const std::type_info& type = m_Data.type();

    if(type == typeid(QString))
            return QString("QString");
    else if(type == typeid(int8_t))
        return QString("int8_t");
    else if(type == typeid(int16_t))
        return QString("int16_t");
    else if(type == typeid(int32_t))
        return QString("int32_t");
    else if(type == typeid(int64_t))
        return QString("int64_t");
    else if(type == typeid(uint8_t))
        return QString("uint8_t");
    else if(type == typeid(uint16_t))
        return QString("uint16_t");
    else if(type == typeid(uint32_t))
        return QString("uint32_t");
    else if(type == typeid(uint64_t))
        return QString("uint64_t");
    else if(type == typeid(bool))
        return QString("bool");
    else if(type == typeid(float))
        return QString("float");
    else if(type == typeid(double))
        return QString("double");
    else if(type == typeid(GuiSelection))
        return QString("GuiSelection");
    else if(type == typeid(QStringList))
        return QString("QStringList");

}


void InterfaceData::SetDataKeepType(int8_t data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );

}
void InterfaceData::SetDataKeepType(int16_t data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(int32_t data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(int64_t data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(uint8_t data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(uint16_t data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(uint32_t data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(uint64_t data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(float data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(bool data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(QString data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in.toInt();
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in.toInt();
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in.toInt();
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in.toInt();
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in.toUInt();
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in.toUInt();
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in.toUInt();
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in.toUInt();
    else if(type == typeid(bool))
        m_Data = (bool) data_in.toUInt();
    else if(type == typeid(float))
        m_Data = (float) data_in.toFloat();
    else if(type == typeid(double))
        m_Data = (double) data_in.toDouble();
    else if(type == typeid(QString))
         m_Data = data_in;
    else if(type == typeid(QStringList))
    {
            QStringList tmp =  boost::get<QStringList>(m_Data);
            tmp.append(data_in);
            m_Data = tmp;
    }
    else if(type == typeid(GuiSelection))
    {
            GuiSelection tmp =  boost::get<GuiSelection>(m_Data);
            m_Data = GuiSelection(data_in, tmp.second);
    }
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}
void InterfaceData::SetDataKeepType(double data_in)
{
    const std::type_info& type = m_Data.type();
    if(type == typeid(int8_t))
        m_Data = (int8_t) data_in;
    else if(type == typeid(int16_t))
        m_Data = (int16_t) data_in;
    else if(type == typeid(int32_t))
        m_Data = (int32_t) data_in;
    else if(type == typeid(int64_t))
        m_Data = (int64_t) data_in;
    else if(type == typeid(uint8_t))
        m_Data = (uint8_t) data_in;
    else if(type == typeid(uint16_t))
        m_Data = (uint16_t) data_in;
    else if(type == typeid(uint32_t))
        m_Data = (uint32_t) data_in;
    else if(type == typeid(uint64_t))
        m_Data = (uint64_t) data_in;
    else if(type == typeid(bool))
        m_Data = (bool) data_in;
    else if(type == typeid(float))
        m_Data = (float) data_in;
    else if(type == typeid(double))
        m_Data = (double) data_in;
    else if(type == typeid(QString))
         m_Data = QString::number(data_in);
    else
         throw std::invalid_argument( "Unknown cast neccessary" );
}



void InterfaceData::SetData(DataPair Data)
{
    DataType = "vector<double>";
    m_Data = Data;
}

void InterfaceData::SetData(GuiSelection input)
{
   DataType = "GuiSelection";
   this->m_Data  = input;
}

void InterfaceData::SetData(bool input)
{    
   DataType = "boolean";
   this->m_Data  = input;
}
void InterfaceData::SetData(uint64_t input)
{
    DataType = "uint64_t";
    this->m_Data  = input;
}
void InterfaceData::SetData(uint32_t input)
{
    DataType = "uint32_t";
    this->m_Data  = input;
}
void InterfaceData::SetData(uint16_t input)
{
    DataType = "uint16_t";
    this->m_Data  = input;
}
void InterfaceData::SetData(uint8_t input)
{
    DataType = "uint8_t";
    this->m_Data  = input;
}

void InterfaceData::SetData(int64_t input)
{
    DataType = "int64_t";
    this->m_Data  = input;
}
void InterfaceData::SetData(int32_t input)
{
    DataType = "int32_t";
    this->m_Data  = input;
}
void InterfaceData::SetData(int16_t input)
{
    DataType = "int16_t";
    this->m_Data  = input;
}
void InterfaceData::SetData(int8_t input)
{
    DataType = "int8_t";
    this->m_Data  = input;
}


void InterfaceData::SetData(double input)
{
   DataType = "double";
   this->m_Data  = input;
}

void InterfaceData::SetData(float input)
{
  DataType = "float";
  this->m_Data  = input;
}

void InterfaceData::SetData(QStringList input)
{
   DataType = "QStringList";
   this->m_Data  = input;
}



void InterfaceData::SetData(QString input)
{

    const std::type_info& type = m_Data.type();
    if(type == typeid(boost::blank))
    {
        this->DataType = "string";
        this->m_Data  = input;
    }
    else
        SetDataKeepType(input);

    return;

    if(this->DataType.compare("boolean") == 0)
    {
       this->DataType = "boolean";
       this->m_Data  = (bool) input.toInt();
    }
    else if(this->DataType.compare("int") == 0)
    {
       this->DataType = "int";
       this->m_Data  = input.toInt();
    }
    else if(this->DataType.compare("float") == 0)
    {
        this->DataType = "float";
        this->m_Data  = input.toFloat();
    }
    else if(this->DataType.compare("double") == 0)
    {
       this->DataType = "double";
       this->m_Data  = input.toDouble();
    }
    else if(this->DataType.compare("string") == 0)
    {
       this->DataType = "string";
       this->m_Data  = input;
    }
    else if(this->DataType.compare("QStringList") == 0)
    {
       this->DataType = "QStringList";
        if(m_Data.which() != 0)
        {
            QStringList tmp =  boost::get<QStringList>(m_Data);
            tmp.append(input);
            m_Data = tmp;
        }
        else
            this->m_Data  = QStringList(input);
    }
    else if(this->DataType.compare("GuiSelection") == 0)
    {
        this->DataType = "GuiSelection";
        if(m_Data.which() == 7)
        {
            GuiSelection tmp =  boost::get<GuiSelection>(m_Data);
            m_Data = GuiSelection(input, tmp.second);

        }
        else
            m_Data = GuiSelection(input,QStringList(input));
    }
    else
    {
       this->DataType = "string";
       this->m_Data  = input;
    }
}

void InterfaceData::SetStateDependency(QString input)
{
    this->StateDependency = input;
}
QString InterfaceData::GetStateDependency()
{
    return this->StateDependency;
}

QString InterfaceData::GetDataType()
{

    const std::type_info& type = m_Data.type();
    QString ret;
    if(type == typeid(QString))
            ret = "QString";
    else if(type == typeid(int8_t))
            ret = "int8_t";
    else if(type == typeid(int16_t))
             ret = "int16_t";
    else if(type == typeid(int32_t))
            ret = "int32_t";
    else if(type == typeid(int64_t))
            ret = "int64_t";
    else if(type == typeid(uint8_t))
             ret = "uint8_t";
    else if(type == typeid(uint16_t))
             ret = "uint16_t";
    else if(type == typeid(uint32_t))
             ret = "uint32_t";
    else if(type == typeid(uint64_t))
             ret = "uint64_t";
    else if(type == typeid(bool))
             ret = "bool";
    else if(type == typeid(float))
             ret = "float";
    else if(type == typeid(double))
             ret = "double";
    else if(type == typeid(GuiSelection))
            ret = "GuiSelection";
    else if(type == typeid(QStringList))
            ret = "QStringList";
    else if(type == typeid(DataPair))
            ret = "vector<double>";
    else
         return ret = "";

    return ret;
    return this->DataType;
}

int64_t InterfaceData::GetInt64_tData(){
    return boost::get<int64_t>(m_Data);
}

int32_t InterfaceData::GetInt32_tData(){
    return boost::get<int32_t>(m_Data);
}

int16_t InterfaceData::GetInt16_tData(){
    return boost::get<int16_t>(m_Data);
}

int8_t InterfaceData::GetInt8_tData(){
    return boost::get<int8_t>(m_Data);
}

uint64_t InterfaceData::GetUInt64_tData(){
    return boost::get<uint64_t>(m_Data);
}

uint32_t InterfaceData::GetUInt32_tData(){
    return boost::get<uint32_t>(m_Data);
}

uint16_t InterfaceData::GetUInt16_tData(){
    return boost::get<uint16_t>(m_Data);
}

uint8_t InterfaceData::GetUInt8_tData(){
    return boost::get<uint8_t>(m_Data);
}



bool InterfaceData::GetBool()
{
        return boost::get<bool>(m_Data);
}

float InterfaceData::GetFloat()
{
    return boost::get<float>(m_Data);
}

double InterfaceData::GetDouble()
{
    return boost::get<double>(m_Data);
}

DataPair InterfaceData::GetPointerPair()
{
    return boost::get<DataPair>(m_Data);
}

GuiSelection InterfaceData::GetGuiSelection()
{
    return boost::get<GuiSelection>(m_Data);
}

QStringList InterfaceData::GetStringList()
{
    return boost::get<QStringList>(m_Data);
}

QString InterfaceData::GetString()
{

    /*define DataTypes
        boost::blank,
        int8_t,
        int16_t,
        int32_t,
        int64_t,
        uint8_t,
        uint16_t,
        uint32_t,
        uint64_t,
        float,
        bool,
        QString,
        double,
        QStringList,
        GuiSelection,
        DataPair*/

        const std::type_info& type = m_Data.type();

        if(type == typeid(QString))
                return boost::get<QString>(m_Data);
        else if(type == typeid(int8_t))
                return QString::number((int8_t)boost::get<int8_t>(m_Data));
        else if(type == typeid(int16_t))
                return QString::number((int16_t)boost::get<int16_t>(m_Data));
        else if(type == typeid(int32_t))
                return QString::number((int32_t)boost::get<int32_t>(m_Data));
        else if(type == typeid(int64_t))
                return QString::number((int64_t)boost::get<int64_t>(m_Data));
        else if(type == typeid(uint8_t))
                return QString::number((uint8_t)boost::get<uint8_t>(m_Data));
        else if(type == typeid(uint16_t))
                return QString::number((uint16_t)boost::get<uint16_t>(m_Data));
        else if(type == typeid(uint32_t))
                return QString::number((uint32_t)boost::get<uint32_t>(m_Data));
        else if(type == typeid(uint64_t))
                return QString::number((uint64_t)boost::get<uint64_t>(m_Data));
        else if(type == typeid(bool))
                return QString::number((uint32_t)boost::get<bool>(m_Data));
        else if(type == typeid(float))
                return QString::number((float)boost::get<float>(m_Data));
        else if(type == typeid(double))
                return QString::number((double)boost::get<double>(m_Data));
        else if(type == typeid(GuiSelection))
                  return boost::get<GuiSelection>(m_Data).first;
        else if(type == typeid(QStringList)){
            if(boost::get<QStringList>(m_Data).size())
                 return boost::get<QStringList>(m_Data).at(0);
            else
                return QString();
         }
        else
             return QString();


//    if(this->DataType.compare("boolean") == 0)
//    {
//       return QString::number((int)(boost::get<bool>(m_Data)));
//    }
//    else if(this->DataType.compare("int") == 0)
//    {
//        return QString::number(boost::get<int>(m_Data));
//    }
//    else if(this->DataType.compare("float") == 0)
//    {
//        return QString::number(boost::get<float>(m_Data));
//    }
//    else if(this->DataType.compare("double") == 0)
//    {
//       return QString::number(boost::get<double>(m_Data));
//    }
//    else if(this->DataType.compare("string") == 0)
//    {
//       return boost::get<QString>(m_Data);
//    }
//    else if(this->DataType.compare("GuiSelection") == 0)
//    {
//       return boost::get<GuiSelection>(m_Data).first;
//    }
//    else if(this->DataType.compare("QStringList") == 0)
//    {
//       if(boost::get<QStringList>(m_Data).size())
//            return boost::get<QStringList>(m_Data).at(0);
//       else
//           return QString();
//    }
//    else
//        return boost::get<QString>(m_Data);


    return NULL;
}

void InterfaceData::SetDataType(QString DT)
{
    this->DataType =  DT;
}

void InterfaceData::SetType(QString T)
{
    this->Type = T;
}

