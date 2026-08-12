#include "MessageDispatchPolicy.h"

QVector<MessageDispatchIntent> MessageDispatchPolicy::ReceiverIntents(const QString& command)
{
    if (command == "publish") {
        return {MessageDispatchIntent::AddContainerElement,
                MessageDispatchIntent::SetData,
                MessageDispatchIntent::AddElementToWidget,
                MessageDispatchIntent::SetData,
                MessageDispatchIntent::NewDataReceived};
    }
    if (command == "StatusMessage")
        return {MessageDispatchIntent::WriteStatusMessage};
    if (command == "CloseProject")
        return {MessageDispatchIntent::WriteCloseProjectNotification, MessageDispatchIntent::CloseProject};
    if (command == "error")
        return {MessageDispatchIntent::WriteError};
    if (command == "info")
        return {MessageDispatchIntent::WriteInfo};
    if (command == "notification")
        return {MessageDispatchIntent::WriteNotification};
    if (command == "set")
        return {MessageDispatchIntent::SetData, MessageDispatchIntent::NewDataReceived};
    if (command == "publish_finished")
        return {MessageDispatchIntent::PublishFinished};
    if (command == "publish_start")
        return {MessageDispatchIntent::PublishStart};
    return {};
}
