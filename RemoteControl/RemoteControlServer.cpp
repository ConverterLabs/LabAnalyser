/***************************************************************************
**                                                                        **
**  LabAnlyser, a plugin based data modification and visualization tool   **
**  Copyright (C) 2015-2021 Andreas Hoffmann                              **
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

#include "RemoteControlServer.h"
#include "RemoteControlProtocol.h"

#include <cstring>

RemoteControlServer::RemoteControlServer(std::map<QString, ToFormMapper *> *DataContainerI)
{
    this->DataContainer = DataContainerI;

    connect(&tcpServer, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));

    m_port = 4080;
    while (!(tcpServer.listen(QHostAddress::LocalHost, m_port++)))
        ;
}

void RemoteControlServer::acceptConnection()
{
    // if(ConnectionState.GetCurrentSocket())
    //     ConnectionState.GetCurrentSocket()->close();
    QTcpSocket* socket = tcpServer.nextPendingConnection();
    ConnectionState.SetCurrentSocket(socket);

    connect(socket, SIGNAL(readyRead()),
            this, SLOT(HeaderReceived()));
    connect(socket, &QAbstractSocket::errorOccurred,
            this, &RemoteControlServer::displayError);
    connect(socket, &QAbstractSocket::disconnected, this, [this, socket]() {
        ConnectionState.ResetIfCurrent(socket);
        socket->deleteLater();
    });
}

void RemoteControlServer::HeaderReceived()
{
    QTcpSocket* socket = ConnectionState.GetCurrentSocket();
    if (!socket)
        return;

    while (socket->bytesAvailable())
    {
        ConnectionState.GetFrameSplitter().Append(socket->readAll());
        QByteArray Data;
        while (true)
        {
            const RemoteControlFrameSplitter::FrameResult frameResult = ConnectionState.GetFrameSplitter().TakeFrame(&Data);
            if (frameResult == RemoteControlFrameSplitter::FrameResult::Incomplete)
                return;
            if (frameResult == RemoteControlFrameSplitter::FrameResult::InvalidPrefix)
            {
                QTcpSocket* currentSocket = ConnectionState.GetCurrentSocket();
                if (currentSocket)
                    currentSocket->abort();
                return;
            }

            const RemoteControlProtocol::DecodedFrame decoded = RemoteControlProtocol::DecodeValidatedFrame(Data);
            if (decoded.Status == RemoteControlProtocol::DecodeStatus::Invalid)
                continue;

            ReceivedID = decoded.Id;

            if (decoded.CommandType == RemoteControlProtocol::Command::Set)
            {
                if (this->DataContainer)
                {
                    auto it = this->DataContainer->find(ReceivedID);
                    if (it != this->DataContainer->end())
                    {
                        InterfaceData Data_ = *((*(this->DataContainer))[ReceivedID]);
                        if (Data_.IsNumeric())
                        {
                            if (!RemoteControlProtocol::HasNumericSetPayload(decoded))
                                continue;
                            double value = 0.0;
                            std::memcpy(&value, decoded.Payload.constData(), sizeof(value));
                            Data_.SetDataKeepType(value);
                        }
                        else if (Data_.IsString())
                        {
                            QString TS = QString::fromLatin1(RemoteControlProtocol::RemoveOptionalTrailingNul(decoded.Payload));
                            Data_.SetData(TS);
                        }
                        else if (Data_.IsStringList())
                        {
                            QString TS = QString::fromLatin1(RemoteControlProtocol::RemoveOptionalTrailingNul(decoded.Payload));
                            QStringList SL;
                            SL.append(TS);
                            Data_.SetData(SL);
                        }
                        else if (Data_.IsGuiSelection())
                        {
                            auto Sel = Data_.GetGuiSelection();
                            QString TS = QString::fromLatin1(RemoteControlProtocol::RemoveOptionalTrailingNul(decoded.Payload));
                            Sel.first = TS;
                            if (Sel.second.contains(TS))
                                Data_.SetData(Sel);
                        }
                        else if (Data_.IsStringList())
                        {
                        }
                        emit MessageSender("set", ReceivedID, Data_);
                    }
                }
            }
            if (decoded.CommandType == RemoteControlProtocol::Command::Get)
            {
                QByteArray DataOut = RemoteControlProtocol::EncodeEmptyReply();

                if (this->DataContainer)
                {
                    auto it = this->DataContainer->find(ReceivedID);
                    if (it != this->DataContainer->end())
                    {
                        InterfaceData Data_ = *((*(this->DataContainer))[ReceivedID]);
                        if (Data_.IsNumeric())
                        {
                            DataOut = RemoteControlProtocol::EncodeNumericReply(Data_.GetAsDouble());
                        }
                        else if (Data_.IsString() || Data_.IsStringList())
                        {
                            DataOut = RemoteControlProtocol::EncodeStringReply(Data_.GetString());
                        }
                        else if (Data_.IsGuiSelection())
                        {
                            DataOut = RemoteControlProtocol::EncodeStringReply(Data_.GetGuiSelection().first);
                        }
                        else if (Data_.IsPairOfVectorOfDoubles())
                        {
                            auto pointerPair = Data_.GetPointerPair();  // Store result to ensure consistency
                            if (pointerPair.first && pointerPair.second) {
                                std::vector<double> Time;
                                std::vector<double> MeasuredDataOut;

                                auto TP = pointerPair.first;
                                auto DP = pointerPair.second;

                                if (TP && !TP->empty()) {
                                    Time.insert(Time.end(), TP->begin(), TP->end());
                                } else {
                                    Time.push_back(0.0);
                                }

                                if (DP && !DP->empty()) {
                                    MeasuredDataOut.insert(MeasuredDataOut.end(), DP->begin(), DP->end());
                                } else {
                                    MeasuredDataOut.push_back(0.0);
                                }

                                DataOut = RemoteControlProtocol::EncodeVectorReply(Time, MeasuredDataOut);
                            }
                            else
                            {
                                DataOut = RemoteControlProtocol::EncodeEmptyReply();
                            }
                        }
                    }
                    else
                    {

                        // create a string  with all keys that conaint ReceivedID and separate them by |
                        QString Keys;
                        for (auto it = this->DataContainer->begin(); it != this->DataContainer->end(); it++)
                        {
                            if (it->first.contains(ReceivedID))
                            {
                                Keys.append(it->first);
                                Keys.append("|");
                            }

                        }
                        // if string != empty remove last |
                        if (Keys.size())
                            Keys.remove(Keys.size() - 1, 1);

                        // check if there are any keys
                        if (Keys.size())
                        {
                            DataOut = RemoteControlProtocol::EncodeStringReply(Keys);
                        }
                        else
                        {
                            DataOut = RemoteControlProtocol::EncodeEmptyReply();
                        }
                    }
                }
                QTcpSocket* currentSocket = ConnectionState.GetCurrentSocket();
                if (!currentSocket)
                    return;
                currentSocket->write(DataOut);
            }
        }
    }
}

void RemoteControlServer::displayError(QAbstractSocket::SocketError socketError)
{
    return;
}
