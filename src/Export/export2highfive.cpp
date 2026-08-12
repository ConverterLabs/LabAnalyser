#include "export2highfive.h"
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Easy.hpp>
#include <QTime>
#include <QDate>
#include <windows.h>

Export2HDF5::Export2HDF5(DataManagementClass* Manager_)
{
    Manager = Manager_;
}


bool Export2HDF5::Export(QString Filename_,  QStringList Ids_)
{

    Filename = Filename_;
    Ids = Ids_;
    //QString ID = "nsad";




    // we create a new hdf5 file
    HighFive::File file(Filename.toStdString(), HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate);
    //m_file = (void*) file;

    //Alle Elemente in ein matfile entsprechend
    QTime time = QTime::currentTime();
    QDate Date = QDate::currentDate();
    QString DateTime;
    DateTime.append("Measurement_");
    DateTime.append(Date.toString("yyyy-MM-dd "));
    DateTime.append(time.toString("hh:mm:ss"));

    //H5Easy::dumpAttribute(*file, "/", "description", DateTime.toStdString());
    H5Easy::dump(file, "/Timestamp", DateTime.toStdString());


    for(int i = 0; i < Ids.size(); i++)
    {
        QString ID = Ids[i];
        auto C = this->Manager->GetContainer(ID);
        if(C)
        {
           if(C->IsNumeric())
            {
               H5Easy::dump(file, (ID.replace("::","/")).toStdString(), C->GetAsDouble());

            }
            else if(C->IsPairOfVectorOfDoubles())
            {
               if(C->GetPointerPair().first != nullptr)
               {
                    H5Easy::dump(file, (ID.replace("::","/") + "/Time").toStdString(), *(C->GetPointerPair().first));
                    H5Easy::dump(file, (ID.replace("::","/") + "/Data").toStdString(), *(C->GetPointerPair().second));
               }
            }
            else if(C->IsString() || C->IsStringList() || C->IsGuiSelection())
            {
               H5Easy::dump(file, (ID.replace("::","/")).toStdString(), C->GetString().toStdString());
            }
            IDcounter++;
        }
    }

    return false;


}

