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
#ifndef CUSTOMUILOADER
#define CUSTOMUILOADER


#include <QUiLoader>


/** //Override QUILoader create Widget to create the Drag and Drop widgets
 *
 *  @author Andreas Hoffmann
 *  @date 16.03.2018
 *
 *  @version 0.1
 *  Kommentare hinzugefügt (Doxygen).
 */

class DropWidgetsUiLoader : public QUiLoader
{
   Q_OBJECT
public:
   DropWidgetsUiLoader(QObject *parent = 0) : QUiLoader(parent) { }
   virtual QWidget* createWidget(const QString &className,
   QWidget *parent = 0, const QString &name = QString());
private:

QString FileName;
};

#endif // CUSTOMUILOADER

