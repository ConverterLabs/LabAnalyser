#include "exportinputs2xml.h"
#include <QFile>
#include <QDir>


ExportInputs2Xml::ExportInputs2Xml(DataManagementSetClass* Manager_)
{
    Manager = Manager_;

}
bool ExportInputs2Xml::Export2XML(QString Filename_,  QStringList Ids_)
{


    ExportCounter = 0;
    Filename = Filename_;
    Ids = Ids_;
    //create a backup

    QFile file(Filename);
    QDir T(QFileInfo(file).absoluteDir());
    if(!file.open(QIODevice::WriteOnly)){
        Manager->GetMessenger()->SendError("Can't open file: " + file.errorString());
        return true;
    }
    xmlWriter.setDevice(&file);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    WriteParameterSet();

    xmlWriter.writeEndDocument();



}

void ExportInputs2Xml::WriteParameterSet()
{

    xmlWriter.writeStartElement("ParameterSet");
    for(ExportCounter = 0; ExportCounter < Ids.size(); ExportCounter++)
    {
     if(Manager->GetContainer(Ids[ExportCounter]))
         WriteEntry();
    }
    xmlWriter.writeEndElement();

}

void ExportInputs2Xml::WriteEntry()
{
    xmlWriter.writeStartElement("entry");
        WriteParameter();
        WriteValue();
    xmlWriter.writeEndElement();
}

void ExportInputs2Xml::WriteParameter()
{
    xmlWriter.writeStartElement("Parameter");
         xmlWriter.writeCharacters(Ids[ExportCounter]);
    xmlWriter.writeEndElement();
}

void ExportInputs2Xml::WriteValue()
{
    xmlWriter.writeStartElement("Value");
         xmlWriter.writeCharacters(Manager->GetContainer(Ids[ExportCounter])->GetString());
    xmlWriter.writeEndElement();
}
