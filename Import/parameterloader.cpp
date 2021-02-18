#include "parameterloader.h"
#include <QFile>
#include <QDir>


ParameterLoader::ParameterLoader( DataManagementSetClass* Manager_)
{
    Manager= Manager_;

}

bool ParameterLoader::Load(QString Path)
{
    bool Error = false;
    //create a backup
    QFile file(Path);
    QDir T(QFileInfo(file).absoluteDir());
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        Manager->GetMessenger()->SendError("Can't open file: " + file.errorString());
        return true;
    }
    reader.setDevice(&file);

    if (reader.readNextStartElement()) {
        if (reader.name() == "ParameterSet")
            readParameterSets();
        else
        {
            reader.raiseError(QObject::tr("Not a Parameter file!"));
            Error = true;
        }
    }
    file.close();


    if(reader.error())
        Manager->GetMessenger()->SendError(reader.errorString());

    return Error||reader.error();

}

void ParameterLoader::readParameterSets()
{
    while(reader.readNextStartElement()){
       if(reader.name() == "entry")
            readEntry();
       else
           reader.skipCurrentElement();
   }
}

void ParameterLoader::readEntry()
{
    QString Id;
    while(reader.readNextStartElement())
    {
       if(reader.name() == "Parameter")
       {
           Id =  reader.readElementText().trimmed();
       }
       else if(reader.name() == "Value")
       {
           if(!Id.isEmpty())
           {
                auto con = Manager->GetContainer(Id);
                if(con)
                {
                    InterfaceData tmp = *(Manager->GetContainer(Id));
                    tmp.SetDataKeepType(reader.readElementText().trimmed());
                    Manager->GetMessenger()->MessageTransmitter("set", Id, tmp);
                }
                else
                {
                    Manager->GetMessenger()->SendInfo("Parameter " + Id + " unkown!");
                    reader.skipCurrentElement();
                }
           }
       }
       else
           reader.skipCurrentElement();
   }
}

