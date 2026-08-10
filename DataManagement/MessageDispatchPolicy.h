#ifndef MESSAGEDISPATCHPOLICY_H
#define MESSAGEDISPATCHPOLICY_H

#include <QString>
#include <QVector>

// Internal value-only classification for MessengerClass.  It owns no payload
// and deliberately has no QObject or infrastructure dependency.
enum class MessageDispatchIntent
{
    AddContainerElement,
    SetData,
    AddElementToWidget,
    NewDataReceived,
    WriteStatusMessage,
    CloseProject,
    WriteError,
    WriteInfo,
    WriteNotification,
    WriteCloseProjectNotification,
    PublishFinished,
    PublishStart
};

class MessageDispatchPolicy
{
public:
    static QVector<MessageDispatchIntent> ReceiverIntents(const QString& command);
};

#endif // MESSAGEDISPATCHPOLICY_H
