#include "LabDataArchive.h"

#include "DataManagement/DataManagementSetClass.h"
#include "DataManagement/DataMessengerClass.h"
#include "DataManagement/mapper.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <cstring>
#include <limits>

namespace {
constexpr char Magic[] = "LABANALYSER-LADAT-1\n";
constexpr quint64 PrefixBytes = sizeof(Magic) - 1 + sizeof(quint64);

bool writeBytes(QIODevice& file, const char* data, qint64 size) { return size >= 0 && file.write(data, size) == size; }
bool writeU64(QIODevice& file, quint64 value) {
    char bytes[8]; for (int i = 0; i < 8; ++i) bytes[i] = char((value >> (i * 8)) & 0xff);
    return writeBytes(file, bytes, sizeof(bytes));
}
bool readU64(QIODevice& file, quint64* value) {
    char bytes[8]; if (file.read(bytes, sizeof(bytes)) != sizeof(bytes)) return false;
    *value = 0; for (int i = 0; i < 8; ++i) *value |= quint64(uchar(bytes[i])) << (i * 8); return true;
}
bool writeText(QIODevice& file, const QString& text) {
    const QByteArray utf8 = text.toUtf8(); return writeU64(file, utf8.size()) && writeBytes(file, utf8.constData(), utf8.size());
}
bool readText(QIODevice& file, QString* text) {
    quint64 length = 0; if (!readU64(file, &length) || length > quint64(std::numeric_limits<int>::max())) return false;
    QByteArray bytes(static_cast<int>(length), Qt::Uninitialized);
    if (length && file.read(bytes.data(), bytes.size()) != bytes.size()) return false;
    *text = QString::fromUtf8(bytes); return true;
}
bool writeDouble(QIODevice& file, double value) { quint64 bits = 0; static_assert(sizeof(bits) == sizeof(value)); std::memcpy(&bits, &value, sizeof(bits)); return writeU64(file, bits); }
bool readDouble(QIODevice& file, double* value) { quint64 bits = 0; if (!readU64(file, &bits)) return false; std::memcpy(value, &bits, sizeof(bits)); return true; }

quint64 textBytes(const QString& text) { return 8 + quint64(text.toUtf8().size()); }
QString storedType(ToFormMapper& value) { return value.IsPairOfVectorOfDoubles() ? QStringLiteral("DataPair") : value.GetTypeInfo(); }
quint64 payloadBytes(ToFormMapper& value) {
    if (value.IsPairOfVectorOfDoubles()) { const DataPair pair = value.GetPointerPair(); const quint64 t = pair.first ? pair.first->size() : 0; const quint64 d = pair.second ? pair.second->size() : 0; return 24 + 8 * (t + d); }
    if (value.IsString()) return textBytes(value.GetString());
    if (value.IsStringList()) { quint64 n = 8; for (const QString& item : value.GetStringList()) n += textBytes(item); return n; }
    if (value.IsGuiSelection()) { const GuiSelection s = value.GetGuiSelection(); quint64 n = textBytes(s.first) + 8; for (const QString& item : s.second) n += textBytes(item); return n; }
    if (value.IsBool()) return 1;
    if (value.GetTypeInfo() == QStringLiteral("int8_t") || value.GetTypeInfo() == QStringLiteral("uint8_t")) return 1;
    if (value.GetTypeInfo() == QStringLiteral("int16_t") || value.GetTypeInfo() == QStringLiteral("uint16_t")) return 2;
    if (value.GetTypeInfo() == QStringLiteral("int32_t") || value.GetTypeInfo() == QStringLiteral("uint32_t") || value.GetTypeInfo() == QStringLiteral("float")) return 4;
    return 8;
}

bool writePayload(QIODevice& file, ToFormMapper& value) {
    const QString type = value.GetTypeInfo();
    if (value.IsPairOfVectorOfDoubles()) {
        const DataPair pair = value.GetPointerPair(); const auto time = pair.first; const auto data = pair.second;
        if (!writeU64(file, time ? time->size() : 0) || !writeU64(file, data ? data->size() : 0) || !writeDouble(file, pair.third ? *pair.third : 0.0)) return false;
        if (time) for (double x : *time) if (!writeDouble(file, x)) return false;
        if (data) for (double x : *data) if (!writeDouble(file, x)) return false;
        return true;
    }
    if (value.IsString()) return writeText(file, value.GetString());
    if (value.IsStringList()) { const QStringList list = value.GetStringList(); if (!writeU64(file, list.size())) return false; for (const QString& item : list) if (!writeText(file, item)) return false; return true; }
    if (value.IsGuiSelection()) { const GuiSelection selection = value.GetGuiSelection(); if (!writeText(file, selection.first) || !writeU64(file, selection.second.size())) return false; for (const QString& item : selection.second) if (!writeText(file, item)) return false; return true; }
    if (value.IsBool()) { const char v = value.GetBool() ? 1 : 0; return writeBytes(file, &v, 1); }
    if (type == QStringLiteral("int8_t")) { const qint8 v = value.GetInt8_tData(); return writeBytes(file, reinterpret_cast<const char*>(&v), 1); }
    if (type == QStringLiteral("uint8_t")) { const quint8 v = value.GetUInt8_tData(); return writeBytes(file, reinterpret_cast<const char*>(&v), 1); }
    if (type == QStringLiteral("int16_t")) { const qint16 v = value.GetInt16_tData(); return writeBytes(file, reinterpret_cast<const char*>(&v), 2); }
    if (type == QStringLiteral("uint16_t")) { const quint16 v = value.GetUInt16_tData(); return writeBytes(file, reinterpret_cast<const char*>(&v), 2); }
    if (type == QStringLiteral("int32_t")) { const qint32 v = value.GetInt32_tData(); return writeBytes(file, reinterpret_cast<const char*>(&v), 4); }
    if (type == QStringLiteral("uint32_t")) { const quint32 v = value.GetUInt32_tData(); return writeBytes(file, reinterpret_cast<const char*>(&v), 4); }
    if (type == QStringLiteral("int64_t")) return writeU64(file, static_cast<quint64>(value.GetInt64_tData()));
    if (type == QStringLiteral("uint64_t")) return writeU64(file, value.GetUInt64_tData());
    if (type == QStringLiteral("float")) { const float v = value.GetFloat(); quint32 bits = 0; std::memcpy(&bits, &v, 4); char b[4]; for (int i=0;i<4;++i) b[i]=char((bits>>(i*8))&0xff); return writeBytes(file,b,4); }
    return writeDouble(file, value.GetDouble());
}

bool readPayload(QIODevice& file, const QString& type, InterfaceData* value) {
    if (type == QStringLiteral("DataPair")) { quint64 tc=0, dc=0; double offset=0; if (!readU64(file,&tc)||!readU64(file,&dc)||!readDouble(file,&offset)||tc>1000000000ULL||dc>1000000000ULL) return false; auto time=boost::shared_ptr<std::vector<double>>(new std::vector<double>); auto data=boost::shared_ptr<std::vector<double>>(new std::vector<double>); time->reserve(size_t(tc)); data->reserve(size_t(dc)); double x=0; for(quint64 i=0;i<tc;++i){if(!readDouble(file,&x))return false;time->push_back(x);} for(quint64 i=0;i<dc;++i){if(!readDouble(file,&x))return false;data->push_back(x);} value->SetData(DataPair(time,data,offset)); return true; }
    if (type == QStringLiteral("QString")) { QString v; if(!readText(file,&v))return false; value->SetData(v); return true; }
    if (type == QStringLiteral("QStringList")) { quint64 n=0; if(!readU64(file,&n)||n>1000000)return false; QStringList list; for(quint64 i=0;i<n;++i){QString v;if(!readText(file,&v))return false;list.push_back(v);} value->SetData(list); return true; }
    if (type == QStringLiteral("GuiSelection")) { QString selected;quint64 n=0;if(!readText(file,&selected)||!readU64(file,&n)||n>1000000)return false;QStringList list;for(quint64 i=0;i<n;++i){QString v;if(!readText(file,&v))return false;list.push_back(v);}value->SetData(GuiSelection(selected,list));return true; }
    char b[8]{}; const int n=(type.endsWith("8_t")||type==QStringLiteral("bool"))?1:(type.endsWith("16_t"))?2:(type.endsWith("32_t")||type==QStringLiteral("float"))?4:8; if(file.read(b,n)!=n)return false;
    quint64 raw=0;for(int i=0;i<n;++i)raw|=quint64(uchar(b[i]))<<(i*8);
    if(type==QStringLiteral("bool"))value->SetData(bool(raw)); else if(type==QStringLiteral("int8_t"))value->SetData(qint8(raw)); else if(type==QStringLiteral("uint8_t"))value->SetData(quint8(raw)); else if(type==QStringLiteral("int16_t"))value->SetData(qint16(raw)); else if(type==QStringLiteral("uint16_t"))value->SetData(quint16(raw)); else if(type==QStringLiteral("int32_t"))value->SetData(qint32(raw)); else if(type==QStringLiteral("uint32_t"))value->SetData(quint32(raw)); else if(type==QStringLiteral("int64_t"))value->SetData(qint64(raw)); else if(type==QStringLiteral("uint64_t"))value->SetData(raw); else if(type==QStringLiteral("float")){quint32 bits=raw;float v;std::memcpy(&v,&bits,4);value->SetData(v);}else{double v;std::memcpy(&v,&raw,8);value->SetData(v);} return true;
}
}

namespace LabDataArchive {
bool ExportAll(DataManagementSetClass& manager, const QString& path, QString* error) {
    QJsonArray channels; quint64 offset = 0;
    const auto* containers = manager.GetContainerPointer();
    for (const auto& entry : *containers) { if (!entry.second) continue; ToFormMapper& value=*entry.second; const quint64 bytes=payloadBytes(value); QJsonObject channel{{"id",entry.first},{"dataType",value.GetDataType()},{"category",value.GetType()},{"stateDependency",value.GetStateDependency()},{"alias",manager.GetAlias(entry.first)},{"min",value.MinValue},{"max",value.MaxValue},{"valueType",storedType(value)},{"offset",QString::number(offset)},{"bytes",QString::number(bytes)}}; channels.append(channel); offset += bytes; }
    const QJsonObject header{{"format",QStringLiteral("LabAnalyserData")},{"version",1},{"createdUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},{"byteOrder",QStringLiteral("littleEndian")},{"channels",channels}};
    const QByteArray json=QJsonDocument(header).toJson(QJsonDocument::Compact); QSaveFile file(path); if(!file.open(QIODevice::WriteOnly)){if(error)*error=file.errorString();return false;} if(!writeBytes(file,Magic,sizeof(Magic)-1)||!writeU64(file,json.size())||!writeBytes(file,json.constData(),json.size())){if(error)*error=file.errorString();return false;} for(const auto& entry:*containers) if(entry.second&&!writePayload(file,*entry.second)){if(error)*error=QStringLiteral("Could not write channel payload");return false;} if(!file.commit()){if(error)*error=file.errorString();return false;} return true;
}

bool Import(DataManagementSetClass& manager, const QString& path, QString* datasetRoot, QString* error) {
    QFile file(path); if(!file.open(QIODevice::ReadOnly)){if(error)*error=file.errorString();return false;} QByteArray magic(sizeof(Magic)-1,Qt::Uninitialized); if(file.read(magic.data(),magic.size())!=magic.size()||magic!=QByteArray(Magic,sizeof(Magic)-1)){if(error)*error=QStringLiteral("Not a LabAnalyser data archive");return false;} quint64 headerSize=0;if(!readU64(file,&headerSize)||headerSize>16*1024*1024){if(error)*error=QStringLiteral("Invalid archive header");return false;} const QByteArray json=file.read(qint64(headerSize));const QJsonDocument document=QJsonDocument::fromJson(json);if(!document.isObject()||document.object().value("format").toString()!=QStringLiteral("LabAnalyserData")||document.object().value("version").toInt()!=1){if(error)*error=QStringLiteral("Unsupported archive format");return false;} const quint64 payloadStart=PrefixBytes+headerSize; const QString base=QFileInfo(path).completeBaseName().replace(QStringLiteral("::"),QStringLiteral("_")); const QString root=QStringLiteral("Export_%1_%2").arg(base,QDateTime::currentDateTimeUtc().toString("yyyyMMdd_HHmmss")); for(const QJsonValue& item:document.object().value("channels").toArray()){const QJsonObject c=item.toObject();bool ok=false;const quint64 offset=c.value("offset").toString().toULongLong(&ok);const quint64 bytes=c.value("bytes").toString().toULongLong(&ok);if(!ok||offset>quint64(file.size())||bytes>quint64(file.size())||offset+bytes>quint64(file.size())-payloadStart||!file.seek(qint64(payloadStart+offset))){if(error)*error=QStringLiteral("Invalid channel offset");return false;} InterfaceData value(c.value("dataType").toString(),c.value("category").toString()); value.SetStateDependency(c.value("stateDependency").toString()); if(!readPayload(file,c.value("valueType").toString(),&value)){if(error)*error=QStringLiteral("Invalid channel payload");return false;} const QString id=root+QStringLiteral("::")+c.value("id").toString(); manager.GetMessenger()->MessageReceiver(QStringLiteral("publish"), id, value); manager.SetMinMaxValue(id,c.value("min").toDouble(),c.value("max").toDouble()); manager.SetAlias(id,c.value("alias").toString()); } if(datasetRoot)*datasetRoot=root;return true;
}
}
