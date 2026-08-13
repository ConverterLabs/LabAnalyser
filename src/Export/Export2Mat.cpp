/***************************************************************************
**  MATLAB MAT-file export using the matio library                         **
****************************************************************************/

#include "Export2Mat.h"
#include <QDate>
#include <QDebug>
#include <QTime>

namespace {

matvar_t* CreateString(const char* name, const QByteArray& value)
{
    const size_t dims[2] = {1, static_cast<size_t>(value.size())};
    return Mat_VarCreate(name, MAT_C_CHAR, MAT_T_UTF8, 2, dims,
                         value.isEmpty() ? nullptr : const_cast<char*>(value.constData()), 0);
}

matvar_t* CreateDouble(const char* name, const double* values, size_t count)
{
    const size_t dims[2] = {count, 1};
    return Mat_VarCreate(name, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims,
                         count == 0 ? nullptr : const_cast<double*>(values), 0);
}

}

MatExporter::MatExporter(DataManagementClass* Manager_)
    : Manager(Manager_)
{
}

void MatExporter::WriteTimeStamp()
{
    const QString timestamp = QStringLiteral("Measurement_%1 %2")
        .arg(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")))
        .arg(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
    matvar_t* variable = CreateString("Timestamp", timestamp.toUtf8());
    if (variable) {
        Mat_VarWrite(matfile, variable, MAT_COMPRESSION_NONE);
        Mat_VarFree(variable);
    }
}

void MatExporter::ExportChannels()
{
    const char* fields[] = {"ID", "Time", "Data"};
    const size_t dims[2] = {static_cast<size_t>(Ids.size()), 1};
    exportedChannels = Mat_VarCreateStruct("ExportedChannels", 2, dims, fields, 3);
    if (!exportedChannels)
        return;

    size_t index = 0;
    for (const QString& ID : Ids) {
        auto container = Manager->GetContainer(ID);
        if (!container)
            continue;

        matvar_t* idVariable = CreateString("ID", ID.toUtf8());
        matvar_t* timeVariable = nullptr;
        matvar_t* dataVariable = nullptr;

        if (container->IsNumeric()) {
            timeVariable = CreateString("Time", QByteArray());
            const double value = container->GetAsDouble();
            dataVariable = CreateDouble("Data", &value, 1);
        } else if (container->IsPairOfVectorOfDoubles()
                   && container->GetPointerPair().first
                   && container->GetPointerPair().second) {
            const auto& time = *container->GetPointerPair().first;
            const auto& data = *container->GetPointerPair().second;
            timeVariable = CreateDouble("Time", time.data(), time.size());
            dataVariable = CreateDouble("Data", data.data(), data.size());
        } else if (container->IsString() || container->IsStringList()
                   || container->IsGuiSelection()) {
            timeVariable = CreateString("Time", QByteArray());
            dataVariable = CreateString("Data", container->GetString().toUtf8());
        }

        // A declared container may not have received a value yet.  libmatio
        // struct fields must still be valid matvar_t instances; passing null
        // here causes an access violation in the writer.
        if (!timeVariable)
            timeVariable = CreateDouble("Time", nullptr, 0);
        if (!dataVariable)
            dataVariable = CreateDouble("Data", nullptr, 0);

        Mat_VarSetStructFieldByName(exportedChannels, "ID", index, idVariable);
        Mat_VarSetStructFieldByName(exportedChannels, "Time", index, timeVariable);
        Mat_VarSetStructFieldByName(exportedChannels, "Data", index, dataVariable);
        ++index;
    }

    Mat_VarWrite(matfile, exportedChannels, MAT_COMPRESSION_NONE);
    Mat_VarFree(exportedChannels);
    exportedChannels = nullptr;
}

bool MatExporter::Export2Mat(QString Filename_, QStringList Ids_)
{
    if (!Manager)
        return true;

    Filename = Filename_;
    Ids = Ids_;
    matfile = Mat_CreateVer(Filename.toUtf8().constData(), nullptr, MAT_FT_MAT5);
    if (!matfile) {
        qWarning() << "Could not create MAT file:" << Filename;
        return true;
    }

    WriteTimeStamp();
    ExportChannels();
    Mat_Close(matfile);
    matfile = nullptr;
    return false;
}
