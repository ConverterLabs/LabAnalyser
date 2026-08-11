#include "RemoteControlConnectionState.h"

void RemoteControlConnectionState::SetCurrentSocket(QTcpSocket* socket)
{
    CurrentSocket = socket;
    FrameSplitter.Clear();
}

void RemoteControlConnectionState::ResetCurrentSocket()
{
    CurrentSocket.clear();
    FrameSplitter.Clear();
}

bool RemoteControlConnectionState::ResetIfCurrent(QTcpSocket* socket)
{
    if (CurrentSocket.data() != socket)
        return false;

    ResetCurrentSocket();
    return true;
}

QTcpSocket* RemoteControlConnectionState::GetCurrentSocket() const
{
    return CurrentSocket.data();
}

RemoteControlFrameSplitter& RemoteControlConnectionState::GetFrameSplitter()
{
    return FrameSplitter;
}
