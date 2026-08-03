#pragma once

#include "DataManagement/mapper.h"

class DataManagementSetClass;

// Minimal test seam for the only DropWidgets type used by
// DataManagementSetClass.cpp.  Production widget implementations are not part
// of this component test and remain covered at their own boundary.
class VariantDropWidget
{
public:
    virtual ~VariantDropWidget() {}
    virtual void SetVariantData(ToFormMapper data) = 0;
    virtual void GetVariantData(ToFormMapper* data) = 0;
    virtual bool LoadFromXML(const std::vector<std::pair<QString, QString>>& attributes, const QString& text) = 0;
    virtual bool SaveToXML(std::vector<std::pair<QString, QString>>& attributes, QString& text) = 0;
    virtual void ConnectToID(DataManagementSetClass* manager, QString id) = 0;
};
