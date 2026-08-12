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

namespace {
bool matchesWildcard(const QString& value, const QString& pattern)
{
    qsizetype valueIndex = 0;
    qsizetype patternIndex = 0;
    qsizetype starIndex = -1;
    qsizetype starValueIndex = 0;

    while (valueIndex < value.size())
    {
        if (patternIndex < pattern.size() && pattern.at(patternIndex) == value.at(valueIndex))
        {
            ++patternIndex;
            ++valueIndex;
        }
        else if (patternIndex < pattern.size() && pattern.at(patternIndex) == QLatin1Char('*'))
        {
            starIndex = patternIndex++;
            starValueIndex = valueIndex;
        }
        else if (starIndex >= 0)
        {
            patternIndex = starIndex + 1;
            valueIndex = ++starValueIndex;
        }
        else
        {
            return false;
        }
    }

    while (patternIndex < pattern.size() && pattern.at(patternIndex) == QLatin1Char('*'))
        ++patternIndex;
    return patternIndex == pattern.size();
}
}

RemoteControlServer::RemoteControlServer(std::map<QString, ToFormMapper *> *DataContainerI)
{
    this->DataContainer = DataContainerI;

    connect(&tcpServer, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));

    m_port = 4080;
    while (!(tcpServer.listen(QHostAddress::LocalHost, m_port++)))
        ;
}

RemoteControlServer::~RemoteControlServer()
{
    ConnectionState.ResetCurrentSocket();

    const QList<QTcpSocket*> sockets = tcpServer.findChildren<QTcpSocket*>(QString(), Qt::FindDirectChildrenOnly);
    for (QTcpSocket* socket : sockets)
    {
        disconnect(socket, nullptr, this, nullptr);
        socket->abort();
        socket->deleteLater();
        QCoreApplication::sendPostedEvents(socket, QEvent::DeferredDelete);
    }
    tcpServer.close();
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
                bool replyEncodable = true;

                if (this->DataContainer)
                {
                    auto it = this->DataContainer->find(ReceivedID);
                    const bool wildcardQuery = ReceivedID.contains(QLatin1Char('*'));
                    if (!wildcardQuery && it != this->DataContainer->end())
                    {
                        InterfaceData Data_ = *((*(this->DataContainer))[ReceivedID]);
                        if (Data_.IsNumeric())
                        {
                            DataOut = RemoteControlProtocol::EncodeNumericReply(Data_.GetAsDouble());
                        }
                        else if (Data_.IsString() || Data_.IsStringList())
                        {
                            replyEncodable = RemoteControlProtocol::TryEncodeStringReply(Data_.GetString(), &DataOut);
                        }
                        else if (Data_.IsGuiSelection())
                        {
                            replyEncodable = RemoteControlProtocol::TryEncodeStringReply(Data_.GetGuiSelection().first, &DataOut);
                        }
                        else if (Data_.IsPairOfVectorOfDoubles())
                        {
                            auto pointerPair = Data_.GetPointerPair();  // Store result to ensure consistency
                            if (pointerPair.first && pointerPair.second) {
                                auto TP = pointerPair.first;
                                auto DP = pointerPair.second;
                                const quint64 timeElements = TP->empty() ? 1 : TP->size();
                                const quint64 dataElements = DP->empty() ? 1 : DP->size();
                                if (!RemoteControlProtocol::CanEncodeVectorReplyElements(timeElements, dataElements))
                                {
                                    replyEncodable = false;
                                }
                                else
                                {
                                    std::vector<double> Time = TP->empty() ? std::vector<double>{0.0} : *TP;
                                    std::vector<double> MeasuredDataOut = DP->empty() ? std::vector<double>{0.0} : *DP;
                                    replyEncodable = RemoteControlProtocol::TryEncodeVectorReply(Time, MeasuredDataOut, &DataOut);
                                }
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
                            const bool matches = wildcardQuery
                                    ? matchesWildcard(it->first, ReceivedID)
                                    : it->first.contains(ReceivedID);
                            if (matches)
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
                            replyEncodable = RemoteControlProtocol::TryEncodeStringReply(Keys, &DataOut);
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
                if (!replyEncodable)
                {
                    currentSocket->abort();
                    return;
                }
                currentSocket->write(DataOut);
            }
        }
    }
}

void RemoteControlServer::displayError(QAbstractSocket::SocketError socketError)
{
    return;
}
