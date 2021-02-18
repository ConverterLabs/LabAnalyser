/***************************************************************************
**                                                                        **
**  This file is part of LabAnlyser.                                      **
**                                                                        **
**  LabAnlyser is free software: you can redistribute it and/or modify ´  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
****************************************************************************/

#ifndef REMOTECONTROLSERVER_H
#define REMOTECONTROLSERVER_H
#include <QtWidgets>
#include <QtNetwork>
#include <QObject>
#include "../DataManagement/mapper.h"
#include <map>

class QTcpServer;
class QNetworkSession;

class RemoteControlServer : public QWidget
{
    Q_OBJECT
public:
    explicit RemoteControlServer( std::map<QString, ToFormMapper*> *DataContainerI = 0);

private slots:
    void acceptConnection();
   // void startTransfer();
   // void updateServerProgress();
   void HeaderReceived();
   // void updateClientProgress(qint64 numBytes);
    void displayError(QAbstractSocket::SocketError socketError);

signals:
    void MessageSender(const QString &Command, const QString &ID, InterfaceData Data);

public slots:

private:
    QTcpServer tcpServer;
    QTcpSocket *tcpServerConnection = NULL;
    int bytesToWrite;
    int bytesWritten;
    int bytesReceived;

    bool IsHeaderReceived = false;
    std::map<QString, ToFormMapper*> *DataContainer;
    QString ReceivedID;
    QByteArray DataBuffer;
    QByteArray NextDataBuffer;
    int debugCounter = 0;


};

#endif // REMOTECONTROLSERVER_H
