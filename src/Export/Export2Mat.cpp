/***************************************************************************
**  MATLAB MAT-file export using the matio library                         **
****************************************************************************/

#include "Export2Mat.h"
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QTime>

#include <limits>

namespace {

matvar_t* CreateString(const char* name, const QByteArray& value)
{
    const size_t dims[2] = {1, static_cast<size_t>(value.size())};
    return Mat_VarCreate(name, MAT_C_CHAR, MAT_T_UTF8, 2, dims,
                         value.isEmpty() ? nullptr : const_cast<char*>(value.constData()), 0);
}

matvar_t* CreateDouble(const char* name, const double* values, size_t count,
                       bool copyData = true)
{
    const size_t dims[2] = {count, 1};
    return Mat_VarCreate(name, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims,
                         count == 0 ? nullptr : const_cast<double*>(values),
                         copyData ? 0 : MAT_F_DONT_COPY_DATA);
}

constexpr size_t MaximumMatPayloadBytes = size_t(1024) * 1024 * 1024;

bool AddPayloadBytes(size_t count, size_t* total)
{
    if (count > std::numeric_limits<size_t>::max() / sizeof(double))
        return false;
    const size_t bytes = count * sizeof(double);
    if (*total > MaximumMatPayloadBytes - bytes)
        return false;
    *total += bytes;
    return true;
}

}

MatExporter::MatExporter(DataManagementClass* Manager_)
    : Manager(Manager_)
{
}

bool MatExporter::WriteTimeStamp()
{
    const QString timestamp = QStringLiteral("Measurement_%1 %2")
        .arg(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")))
        .arg(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
    matvar_t* variable = CreateString("Timestamp", timestamp.toUtf8());
    if (!variable)
        return false;

    const bool written = Mat_VarWrite(matfile, variable, MAT_COMPRESSION_NONE) == 0;
    Mat_VarFree(variable);
    return written;
}

bool MatExporter::HasSafePayloadSize() const
{
    size_t total = 0;
    for (const QString& ID : Ids) {
        ToFormMapper* container = Manager->GetContainer(ID);
        if (!container || !container->IsPairOfVectorOfDoubles())
            continue;

        const DataPair pair = container->GetPointerPair();
        if ((pair.first && !AddPayloadBytes(pair.first->size(), &total))
            || (pair.second && !AddPayloadBytes(pair.second->size(), &total)))
            return false;
    }
    return true;
}

bool MatExporter::ExportChannels()
{
    const char* fields[] = {"ID", "Time", "Data"};
    const size_t dims[2] = {static_cast<size_t>(Ids.size()), 1};
    exportedChannels = Mat_VarCreateStruct("ExportedChannels", 2, dims, fields, 3);
    if (!exportedChannels)
        return false;

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
        } else if (container->IsPairOfVectorOfDoubles()) {
            const DataPair pair = container->GetPointerPair();
            if (pair.first && pair.second) {
                const auto& time = *pair.first;
                const auto& data = *pair.second;
                // The manager owns both vectors throughout Mat_VarWrite.
                // Avoid a second full in-memory copy for large recordings.
                timeVariable = CreateDouble("Time", time.data(), time.size(), false);
                dataVariable = CreateDouble("Data", data.data(), data.size(), false);
            }
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

        if (!idVariable || !timeVariable || !dataVariable) {
            Mat_VarFree(idVariable);
            Mat_VarFree(timeVariable);
            Mat_VarFree(dataVariable);
            Mat_VarFree(exportedChannels);
            exportedChannels = nullptr;
            return false;
        }

        Mat_VarSetStructFieldByName(exportedChannels, "ID", index, idVariable);
        Mat_VarSetStructFieldByName(exportedChannels, "Time", index, timeVariable);
        Mat_VarSetStructFieldByName(exportedChannels, "Data", index, dataVariable);
        ++index;
    }

    const bool written = Mat_VarWrite(matfile, exportedChannels, MAT_COMPRESSION_NONE) == 0;
    Mat_VarFree(exportedChannels);
    exportedChannels = nullptr;
    return written;
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

    bool success = false;
    try {
        if (!HasSafePayloadSize()) {
            qWarning() << "MAT export exceeds the 1 GiB uncompressed payload limit:" << Filename;
        } else if (!WriteTimeStamp()) {
            qWarning() << "Could not write MAT timestamp:" << Filename;
        } else if (!ExportChannels()) {
            qWarning() << "Could not write MAT channels:" << Filename;
        } else {
            success = true;
        }
    } catch (const std::bad_alloc&) {
        qWarning() << "MAT export ran out of memory:" << Filename;
    } catch (const std::exception& error) {
        qWarning() << "MAT export failed:" << Filename << error.what();
    }

    CloseFile();
    if (!success)
        QFile::remove(Filename);
    return !success;
}

void MatExporter::CloseFile()
{
    if (exportedChannels) {
        Mat_VarFree(exportedChannels);
        exportedChannels = nullptr;
    }
    if (matfile) {
        Mat_Close(matfile);
        matfile = nullptr;
    }
}
