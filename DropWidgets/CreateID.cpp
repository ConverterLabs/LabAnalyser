#include "CreateID.h"


MainWindow* GetMainWindow()
{
    int i = 0;
    MainWindow *MW = qobject_cast<MainWindow*>( QApplication::topLevelWidgets().at(i++));
    while(!MW)
         MW = qobject_cast<MainWindow*>( QApplication::topLevelWidgets().at(i++));
    return MW;
}

//Create ID of an element
QString CreateID(QObject *Tree)
{
    QTreeWidget * treeWidget = qobject_cast<QTreeWidget*>(Tree);
    QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
    QString ToolTip;
    for(auto si : selectedItems)
    {
        if(si->childCount() == 0)
        {
            auto sit = si;
            while(sit)
            {
                ToolTip.insert(0,sit->text( 0 ));
                sit = sit->parent();
                if(sit)
                    ToolTip.insert(0,"::");
            }
        }
        return ToolTip;
    }
    return ToolTip;
}

