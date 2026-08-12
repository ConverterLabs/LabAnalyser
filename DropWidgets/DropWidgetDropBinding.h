#ifndef DROPWIDGETDROPBINDING_H
#define DROPWIDGETDROPBINDING_H

#include "CreateID.h"
#include "DropWidgetBinding.h"

#include <QtGui/qevent.h>
#include <QtWidgets/qwidget.h>

// Common legacy setup for adapters whose drop operation replaces all direct
// connections and then binds exactly one source ID.  Special multi-selection,
// label and checkbox contracts deliberately remain outside this helper.
namespace DropWidgetDropBinding
{
struct Context
{
    QString id;
    MainWindow* mainWindow;
    DataManagementSetClass* manager;

    bool IsValid() const { return mainWindow && manager; }
};

inline void ResetConnections(QObject* widget)
{
    widget->disconnect();
    QObject::connect(widget, SIGNAL(customContextMenuRequested(QPoint)), widget, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(widget, SIGNAL(RequestUpdate()));
}

inline void ResetContextConnections(QObject* widget, DataManagementSetClass* manager)
{
    QObject::disconnect(widget, SIGNAL(customContextMenuRequested(QPoint)), widget,
                        SLOT(contextMenu(QPoint)));
    QObject::disconnect(widget, SIGNAL(RequestUpdate()), manager, SLOT(UpdateRequest()));
    QObject::connect(widget, SIGNAL(customContextMenuRequested(QPoint)), widget, SLOT(contextMenu(QPoint)));
    DropWidgetBinding::ConnectRequestUpdate(widget, SIGNAL(RequestUpdate()));
}

inline void ClearConnectionPresentation(QWidget* widget)
{
    widget->setToolTip("");
    widget->setToolTipDuration(0);
}

inline void RemoveManagerBinding(QWidget* widget)
{
    MainWindow* mainWindow = GetMainWindow();
    if (mainWindow)
        mainWindow->GetLogic()->DeleteEntryOfObject(widget);
}

inline Context Prepare(QWidget* widget, QDropEvent* event)
{
    ResetConnections(widget);

    const QString id = CreateID(event->source());
    widget->setToolTip(id);
    widget->setToolTipDuration(2000);
    MainWindow* mainWindow = GetMainWindow();
    return { id, mainWindow, mainWindow ? mainWindow->GetLogic() : nullptr };
}

inline void Register(QWidget* widget, const Context& context)
{
    if (!context.IsValid())
        return;
    context.manager->AddElementToContainerEntry(widget->objectName(), context.id,
                                                widget->metaObject()->className(), widget);
    context.mainWindow->ChangeForSaveDetected = true;
}
}

#endif // DROPWIDGETDROPBINDING_H
