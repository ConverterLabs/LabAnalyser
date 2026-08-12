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
    ToFormMapper* container;

    bool IsValid() const { return mainWindow && manager && container; }
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
    const QString id = CreateID(event->source());
    MainWindow* mainWindow = GetMainWindow();
    DataManagementSetClass* manager = mainWindow ? mainWindow->GetLogic() : nullptr;
    ToFormMapper* container = manager ? manager->GetContainer(id) : nullptr;
    return { id, mainWindow, manager, container };
}

inline bool SupportsNumeric(const Context& context)
{
    return context.IsValid() && (context.container->IsFloatingPointNumber()
                                 || context.container->IsUnsigedNumber()
                                 || context.container->IsSigedNumber());
}

inline bool SupportsGuiSelection(const Context& context)
{
    return context.IsValid() && context.container->IsGuiSelection();
}

inline bool SupportsPushButton(const Context& context)
{
    return context.IsValid() && (context.container->GetType().compare("State") == 0
                                 || (context.container->GetType().compare("Parameter") == 0
                                     && (context.container->IsBool() || context.container->IsUnsigedNumber())));
}

inline void Activate(QWidget* widget, const Context& context)
{
    if (!context.IsValid())
        return;
    ResetConnections(widget);
    widget->setToolTip(context.id);
    widget->setToolTipDuration(2000);
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
