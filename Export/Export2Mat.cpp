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

#include "Export2Mat.h"
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "matOut.h"
#include "toolsMatlab.c"
#include <complex.h>
#include "toolsMisc.c"
#include <QTime>
#include <QDate>
#include <QDebug.h>
#include <qstring.h>


MatExporter::MatExporter(DataManagementClass* Manager_)
{
    Manager = Manager_;

    fieldnames_.push_back("ID");
    fieldnames_.push_back("Time");
    fieldnames_.push_back("Data");

    for (int r = 0; r < fieldnames_.size(); r++)
        fieldNames.push_back(fieldnames_[r].c_str());

}

void MatExporter::WriteTimeStamp()
{
    //Alle Elemente in ein matfile entsprechend
    QTime time = QTime::currentTime();
    QDate Date = QDate::currentDate();
    QString DateTime;
    DateTime.append("Measurement_");
    DateTime.append(Date.toString("yyyy-MM-dd "));
    DateTime.append(time.toString("hh:mm:ss"));

    auto mxString= mxCreateString(DateTime.toLatin1());
    matPutVariable(matfile, "Timestamp", mxString);

    mxDestroyArray(mxString);
}

void MatExporter::ExportChannel(QString ID)
{
    mxArray* mxStruct_ = static_cast<mxArray*>(mxStruct);
    //Search ID
    auto C = this->Manager->GetContainer(ID);
    if(C)
    {
        auto mxString= mxCreateString(ID.toLatin1());
        //Eintragen der ID an row = 0
        mxSetFieldByNumber( mxStruct_,   //pointer to the mxStruct
                            IDcounter,          //index of the element (linear)
                            0,          //position of the field (in this case, field 0 is "b"
                            mxString);       //the mxArray we want to add to the mxStruct

        if(C->IsNumeric())
        {
            auto mxString= mxCreateString("");
            mxSetFieldByNumber(mxStruct_, IDcounter, 1, mxString);
            auto mxData2 = mxCreateDoubleMatrix(1, 1, mxREAL);
            double * DatenPrt2;
            double tmp = C->GetAsDouble();
            DatenPrt2 = &tmp;
            double ** DatenPrtPtr2;
            DatenPrtPtr2 = &DatenPrt2;
            copyDoubleToMxArray2D(DatenPrtPtr2, mxData2, 1, 1);
            mxSetFieldByNumber(mxStruct_, IDcounter, 2, mxData2);
        }
        else if(C->IsPairOfVectorOfDoubles())
        {
            auto mxData = mxCreateDoubleMatrix(C->GetPointerPair().first->size(), 1, mxREAL);
            double * DatenPrt;
            DatenPrt = (&(C->GetPointerPair().first->at(0)));
            double ** DatenPrtPtr;
            DatenPrtPtr = &DatenPrt;
            copyDoubleToMxArray2D(DatenPrtPtr, mxData, C->GetPointerPair().first->size(), 1);
            mxSetFieldByNumber(mxStruct_, IDcounter, 1, mxData);
            auto mxData2 = mxCreateDoubleMatrix(C->GetPointerPair().second->size(), 1, mxREAL);
            double * DatenPrt2;
            DatenPrt2 = (&(C->GetPointerPair().second->at(0)));
            double ** DatenPrtPtr2;
            DatenPrtPtr2 = &DatenPrt2;
            copyDoubleToMxArray2D(DatenPrtPtr2, mxData2, C->GetPointerPair().second->size(), 1);
            mxSetFieldByNumber(mxStruct_, IDcounter, 2, mxData2);
        }
        else if(C->IsString() || C->IsStringList() || C->IsGuiSelection())
        {
            auto mxString= mxCreateString(C->GetString().toLatin1());
            mxSetFieldByNumber(mxStruct_, IDcounter, 2, mxString);
        }
        IDcounter++;
    }


    mxStruct = static_cast<void*>(mxStruct_);


}
void MatExporter::ExportChannels()
{

    auto  mxStruct_ = mxCreateStructMatrix(  Ids.size(),  //number of rows
                                            1,  //number of columns
                                            3,  //number of fields in each element
                                            &fieldNames[0]);   //list of field names

    mxStruct = static_cast<void*>(mxStruct_);

    for(int i = 0; i < Ids.size(); i++)
    {
        ExportChannel(Ids[i]);
    }

    mxStruct_ = static_cast<mxArray*>(mxStruct);
    matPutVariable(     matfile, "ExportedChannels", mxStruct_);

    //Here seems to be a bug in the library
    //mxDestroyArray(mxStruct_);
}

bool MatExporter::Export2Mat(QString Filename_, QStringList Ids_)
{
    if(!Manager)
        return true;
    Filename = Filename_;
    Ids = Ids_;

    qDebug() << "QString::fromStdString(Filename.toStdString())";
    qDebug() << QString::fromStdString(Filename.toStdString());
    qDebug() << QString::fromStdString(Filename.toStdString().c_str());

    matfile = matOpen(Filename.toStdString().c_str(), "w");

    WriteTimeStamp();

    //WriteData
    ExportChannels();

    matClose(matfile);
    return 0;
}


