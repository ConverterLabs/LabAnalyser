#pragma once
#include <QObject>
#include <QFile>
#include <QXmlStreamReader>
#include "../plugins/platforminterface.h"
#include "../DataManagement/DataMessengerClass.h"

class LoadPlugin: public QObject
{
   Q_OBJECT
public:
    LoadPlugin(QObject *parent, MessengerClass* Messenger_);
    bool read(QIODevice *device, QString devFileName);
    QString errorString() { return QString();};
    Platform_Interface* GetNewDevice(){return NewDeviceReg;};

private:
    QXmlStreamReader reader;
    void readDevice();
    Platform_Interface *NewDeviceReg = nullptr;
    QString m_devFileName;
    MessengerClass* Messenger;


};

