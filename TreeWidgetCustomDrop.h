/***************************************************************************
**                                                                        **
**  This file is part of LabAnlyser.                                      **
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

#ifndef TREEWIDGETCUSTOMDROP_H
#define TREEWIDGETCUSTOMDROP_H

#include <QObject>
#include <QTreeWidget>
#include <QMimeData>
#include <QDrag>

class TreeWidgetCustomDrop : public QTreeWidget
{
public:
    TreeWidgetCustomDrop(QWidget *parent);


protected:
    QStringList mimeTypes() const
           {
               QStringList qstrList;
               qstrList.append("text/uri-list");
               return qstrList;
           }
           QMimeData * mimeData( const QList<QTreeWidgetItem *> items ) const;
           void startDrag(Qt::DropActions supportedActions) override;

private:

     void performDrag(QMouseEvent *event);




    QPoint startPos;
};

#endif // TREEWIDGETCUSTOMDROP_H
