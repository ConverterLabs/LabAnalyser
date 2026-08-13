#include "MatDataImport.h"

#include "DataManagement/DataManagementSetClass.h"
#include "DataManagement/DataMessengerClass.h"

#include <QDateTime>
#include <QFileInfo>
#include <matio.h>

#include <limits>
#include <vector>

namespace {
QString readText(const matvar_t* variable)
{
    if (!variable || variable->class_type != MAT_C_CHAR || !variable->data)
        return QString();
    const size_t size = variable->rank == 2 ? variable->dims[0] * variable->dims[1] : Mat_VarGetSize(variable);
    if (size > size_t(std::numeric_limits<int>::max()))
        return QString();
    QString result = QString::fromUtf8(static_cast<const char*>(variable->data), int(size));
    while (result.endsWith(QChar::Null))
        result.chop(1);
    return result;
}

bool readDoubles(const matvar_t* variable, std::vector<double>* values)
{
    if (!variable || variable->class_type != MAT_C_DOUBLE || variable->data_type != MAT_T_DOUBLE)
        return false;
    const size_t count = variable->rank == 2 ? variable->dims[0] * variable->dims[1] : Mat_VarGetSize(variable) / sizeof(double);
    if (count > 1000000000ULL)
        return false;
    const double* data = static_cast<const double*>(variable->data);
    values->assign(data, data + count);
    return true;
}
}

namespace MatDataImport {
bool Import(DataManagementSetClass& manager, const QString& path, QString* datasetRoot, QString* error)
{
    mat_t* file = Mat_Open(path.toUtf8().constData(), MAT_ACC_RDONLY);
    if (!file) { if (error) *error = QStringLiteral("Could not open MAT file"); return false; }
    matvar_t* channels = Mat_VarRead(file, "ExportedChannels");
    if (!channels || channels->class_type != MAT_C_STRUCT || channels->rank != 2) {
        if (channels) Mat_VarFree(channels); Mat_Close(file);
        if (error) *error = QStringLiteral("MAT file has no ExportedChannels struct"); return false;
    }

    const size_t count = channels->dims[0] * channels->dims[1];
    const QString base = QFileInfo(path).completeBaseName().replace(QStringLiteral("::"), QStringLiteral("_"));
    const QString root = QStringLiteral("Export_%1_%2").arg(base, QDateTime::currentDateTimeUtc().toString("yyyyMMdd_HHmmss"));
    for (size_t index = 0; index < count; ++index) {
        const QString originalId = readText(Mat_VarGetStructFieldByName(channels, "ID", index));
        if (originalId.isEmpty())
            continue;
        const matvar_t* time = Mat_VarGetStructFieldByName(channels, "Time", index);
        const matvar_t* data = Mat_VarGetStructFieldByName(channels, "Data", index);
        InterfaceData value;
        std::vector<double> timeValues;
        std::vector<double> dataValues;
        const bool hasTime = readDoubles(time, &timeValues);
        const bool hasData = readDoubles(data, &dataValues);
        if (hasTime && hasData) {
            const double t0 = timeValues.empty() ? 0.0 : timeValues.front();
            value.SetData(DataPair(boost::shared_ptr<std::vector<double>>(new std::vector<double>(timeValues)),
                                   boost::shared_ptr<std::vector<double>>(new std::vector<double>(dataValues)), t0));
        } else if (hasData && dataValues.size() == 1) {
            value.SetData(dataValues.front());
        } else if (data && data->class_type == MAT_C_CHAR) {
            value.SetData(readText(data));
        } else {
            continue;
        }
        // MAT exports do not carry the LabAnalyser category.  Imported values
        // are data channels, so set it before publishing; otherwise the
        // MainWindow has no explorer tree to which it can add the channel.
        value.SetType(QStringLiteral("Data"));
        manager.GetMessenger()->MessageReceiver(QStringLiteral("publish"),
                                                root + QStringLiteral("::") + originalId, value);
    }
    Mat_VarFree(channels); Mat_Close(file);
    if (datasetRoot) *datasetRoot = root;
    return true;
}
}
