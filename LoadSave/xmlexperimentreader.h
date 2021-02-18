#pragma once
#include <QObject>
#include <QFile>
#include <QXmlStreamReader>
#include "../DataManagement/DataMessengerClass.h"

class XmlExperimentReader: public QObject
{
   Q_OBJECT
public:
    XmlExperimentReader(QObject *parent = nullptr, MessengerClass* Messenger_ = nullptr );
    bool read(QIODevice *device);
    QString errorString() { return QString();};
private:
    QXmlStreamReader reader;
    void readExperiment();
    void readTab();
    void LoadForm(QString Name);
    void readDevices();
    void LoadDevice();
    void readWidgets();
    void readWidget();
    void ReadState();
    void CreateFigureWindows();
    void CreateFigureWindow();
    void CreateConnections();
    void CreateConnection();

    MessengerClass* Messenger = nullptr;

signals:
    void NewDeviceRegistration(QObject *NewDevice);
    void LoadFormFromXML(QString UiFileName, QString LastFormName, bool skip);

};

