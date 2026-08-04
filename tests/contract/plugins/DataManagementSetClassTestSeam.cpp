// Test-only: LoadPlugin requires the concrete DataManagementSetClass only for
// its DataManagementClass registry and Messenger access. Avoid its unrelated
// widget/plot implementation; this file is not part of LabAnalyser.pro.
#include "DataManagement/DataManagementSetClass.h"
DataManagementSetClass::DataManagementSetClass(QObject *parent) : DataManagementClass(parent) {
    Messenger = new MessengerClass(this);
}
void DataManagementSetClass::SetData(const QString&) {}
void DataManagementSetClass::SendNewValue() {}
void DataManagementSetClass::UpdateRequest() {}
void DataManagementSetClass::UpdateRequest(QString) {}
