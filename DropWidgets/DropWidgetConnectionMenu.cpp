#include "DropWidgetConnectionMenu.h"

#include "CreateID.h"
#include "../mainwindow.h"

#include <QtWidgets/qaction.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmenu.h>

namespace DropWidgetConnectionMenu
{
void Show(QWidget* widget, const QPoint& position, const Options& options)
{
    QMenu* menu = options.standardLineEditMenu
            ? static_cast<QLineEdit*>(widget)->createStandardContextMenu()
            : new QMenu(widget);
    if (options.deleteOnClose)
        menu->setAttribute(Qt::WA_DeleteOnClose);

    const QString connection = GetMainWindow()->GetLogic()->GetContainerID(widget);
    if (!connection.isEmpty())
    {
        MainWindow* mainWindow = GetMainWindow();
        if (options.separatorBeforeHighlight)
            menu->addSeparator();
        QAction* highlight = new QAction;
        QObject::connect(highlight, &QAction::triggered, [=] {
            mainWindow->HighLightConnection(connection);
        });
        highlight->setText("Highlight Connection");
        menu->addAction(highlight);

        if (options.separatorBeforeRemove)
            menu->addSeparator();
        menu->addAction("Remove Connection", widget, SLOT(RemoveConnection()));
    }

    menu->popup(widget->mapToGlobal(position));
    if (options.markChanged)
        GetMainWindow()->ChangeForSaveDetected = true;
}
}
