#pragma once
#include <QObject>
#include <QFile>
#include <QXmlStreamReader>
#include "../DataManagement/DataManagementClass.h"
#include "../DataManagement/DataMessengerClass.h"
#include <QDir>
class xmlexperimentwriter: public QObject
{
   Q_OBJECT
public:
    xmlexperimentwriter(QObject *parent = nullptr, MessengerClass* Messenger_ = nullptr );
    bool write(QIODevice *device, QDir T);
private:
    MessengerClass* Messenger = nullptr;
    DataManagementClass* Manager = nullptr;
    QXmlStreamWriter xmlWriter;
    QDir T;


    void writeExperiment();
    void writeTabs();
    void writeDevices();
    void writeFigureWindows();
    void writeWidgets();
    void writeConnections();
    void writeState();
};

