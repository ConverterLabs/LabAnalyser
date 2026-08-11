#ifndef REMOTECONTROLCONNECTIONSTATE_H
#define REMOTECONTROLCONNECTIONSTATE_H

#include <QPointer>
#include <QTcpSocket>

#include "RemoteControlFrameSplitter.h"

// Private non-owning state for exactly the latest accepted connection. It does
// not create, delete, reparent or otherwise manage the socket QObject.
class RemoteControlConnectionState
{
public:
    void SetCurrentSocket(QTcpSocket* socket);
    void ResetCurrentSocket();
    bool ResetIfCurrent(QTcpSocket* socket);

    QTcpSocket* GetCurrentSocket() const;
    RemoteControlFrameSplitter& GetFrameSplitter();

private:
    QPointer<QTcpSocket> CurrentSocket;
    RemoteControlFrameSplitter FrameSplitter;
};

#endif // REMOTECONTROLCONNECTIONSTATE_H
