#ifndef DROPWIDGETDATAACCESS_H
#define DROPWIDGETDATAACCESS_H

#include "../DataManagement/mapper.h"

#include <QtCore/qstring.h>

#include <utility>
#include <vector>

// Stateless conversion/XML helpers shared by the Designer compatibility
// widgets.  They intentionally preserve the legacy type admission rules; the
// concrete widgets keep their individual range, rounding and presentation.
namespace DropWidgetDataAccess
{
inline bool TryReadNumeric(ToFormMapper& data, double* value)
{
    if (data.IsFloatingPointNumber())
    {
        *value = data.GetFloatingPointData();
        return true;
    }
    if (data.IsSigedNumber())
    {
        *value = static_cast<double>(data.GetSignedData());
        return true;
    }
    if (data.IsUnsigedNumber())
    {
        *value = static_cast<double>(data.GetUnsignedData());
        return true;
    }
    return false;
}

inline bool TryReadIndicatorState(ToFormMapper& data, uint32_t bit, bool* state)
{
    if (data.IsBool())
    {
        *state = data.GetBool();
        return true;
    }
    if (data.IsUnsigedNumber())
    {
        *state = (data.GetUnsignedData() & (1ULL << bit)) != 0;
        return true;
    }
    return false;
}

inline bool LoadBitAttribute(const std::vector<std::pair<QString, QString>>& attributes, uint32_t* bit)
{
    for (const auto& attribute : attributes)
    {
        if (attribute.first == QString("Bit"))
        {
            *bit = attribute.second.toUInt();
            return true;
        }
    }
    return false;
}

inline void SaveBitAttribute(std::vector<std::pair<QString, QString>>& attributes, uint32_t bit)
{
    attributes.push_back(std::make_pair(QString("Bit"), QString::number(bit)));
}
}

#endif // DROPWIDGETDATAACCESS_H
