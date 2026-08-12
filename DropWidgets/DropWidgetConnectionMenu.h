#ifndef DROPWIDGETCONNECTIONMENU_H
#define DROPWIDGETCONNECTIONMENU_H

#include <QtCore/qpoint.h>

class QWidget;

namespace DropWidgetConnectionMenu
{
struct Options
{
    bool separatorBeforeRemove;
    bool deleteOnClose;
    bool markChanged;
    bool standardLineEditMenu;
};

void Show(QWidget* widget, const QPoint& position, const Options& options);
}

#endif // DROPWIDGETCONNECTIONMENU_H
