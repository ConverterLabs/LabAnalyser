/***************************************************************************
**                                                                        **
**  All original material Copyright (C) :                                 **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2010 by Tn                                              **
**   thenobody@poczta.fm                                                   *
****************************************************************************
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
#pragma once

#include <QAbstractButton>
#include <QResizeEvent>
#include <QColor>
#include <QDebug>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>

class QLedIndicator : public QAbstractButton
{
    Q_PROPERTY(QColor onColor      WRITE setOnColor     READ getOnColor   )
    Q_PROPERTY(QColor offColor     WRITE setOffColor    READ getOffColor  )
    Q_OBJECT
    public:
        QLedIndicator(QWidget *parent);

        void setOnColor(QColor c)  { onColor  = c;    }
        void setOffColor(QColor c) { offColor = c;    }

        QColor getOnColor(void)    { return onColor;  }
        QColor getOffColor(void)   { return offColor; }

        void SetState(bool Input) { IsOn = Input; }


    protected:
        virtual void paintEvent (QPaintEvent *event);
        virtual void resizeEvent(QResizeEvent *event);

    private:
        static const qreal scaledSize;  /* init in cpp */
        QColor  onColor;
        QColor  offColor;
        QPixmap ledBuffer;
        bool IsOn =0;
};
