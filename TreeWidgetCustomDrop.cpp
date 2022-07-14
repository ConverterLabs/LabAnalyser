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

#include "TreeWidgetCustomDrop.h"
#include <QMouseEvent>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QDebug>

TreeWidgetCustomDrop::TreeWidgetCustomDrop(QWidget *parent): QTreeWidget(parent)
{

}

QMimeData *	TreeWidgetCustomDrop::mimeData(const QList<QTreeWidgetItem *> items) const
{

    QMimeData *mimeData = new QMimeData;
    QString Ids;
    for(int i = 0; i < items.size(); i++)
    {
        if(items.at(i)->childCount()==0)
        {
            auto sit = items.at(i);
            QString Name;
            while(sit)
            {
                Name.insert(0,sit->text( 0 ));
                sit = sit->parent();
                if(sit)
                    Name.insert(0,"::");
            }
            Ids.push_back(Name);
            if(items.size()>i+1)
                Ids.push_back("\r\n");
        }

    }
    mimeData->setText(Ids.toUtf8());
    return mimeData;
}

/*void TreeWidgetCustomDrop::startDrag(Qt::DropActions supportedActions)
{
    QTreeWidget::startDrag(Qt::CopyAction );
}
*/
