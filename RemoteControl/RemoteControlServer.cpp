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
    // if(tcpServerConnection)
    //     tcpServerConnection->close();
    tcpServerConnection = tcpServer.nextPendingConnection();
    FrameSplitter.Clear();

    connect(tcpServerConnection, SIGNAL(readyRead()),
            this, SLOT(HeaderReceived()));
    connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
}

void RemoteControlServer::HeaderReceived()
{
    while (tcpServerConnection->bytesAvailable())
    {
        FrameSplitter.Append(tcpServerConnection->readAll());
        QByteArray Data;
        while (FrameSplitter.TakeCompleteFrame(&Data))
        {
            const RemoteControlProtocol::DecodedFrame decoded = RemoteControlProtocol::DecodeCompleteFrame(Data);
            uint32_t DataSize = decoded.TotalSize;

            int DataPointer = 0;
            while (DataPointer < DataSize)
            {
                char *DataArrayStart = &((Data.data())[DataPointer]);
                uint32_t LengthID = *((uint32_t *)&DataArrayStart[4 + 3]);
                uint32_t LengthofData = *((uint32_t *)&DataArrayStart[3 + 8]);
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
                                Data_.SetDataKeepType(*((double *)&DataArrayStart[15 + LengthID]));
                            else if (Data_.IsString())
                            {
                                QString TS = QString::fromLatin1(decoded.Payload.left(decoded.Payload.size() - 1));
                                Data_.SetData(TS);
                            }
                            else if (Data_.IsStringList())
                            {
                                QString TS = QString::fromLatin1(decoded.Payload.left(decoded.Payload.size() - 1));
                                QStringList SL;
                                SL.append(TS);
                                Data_.SetData(SL);
                            }
                            else if (Data_.IsGuiSelection())
                            {
                                auto Sel = Data_.GetGuiSelection();
                                QString TS = QString::fromLatin1(decoded.Payload.left(decoded.Payload.size() - 1));
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
                            tcpServerConnection->write(DataOut);
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
                            tcpServerConnection->write(DataOut);
                        }
                    }
                    else
                    {
                        tcpServerConnection->write(DataOut);
                    }
                }
                DataPointer += 15 + LengthID + LengthofData;
            }
        }
    }
}

void RemoteControlServer::displayError(QAbstractSocket::SocketError socketError)
{
    return;
}
