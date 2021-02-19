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

