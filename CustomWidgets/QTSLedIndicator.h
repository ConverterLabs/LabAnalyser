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

class QTSLedIndicator : public QAbstractButton
{
    Q_PROPERTY(QColor onColor1      WRITE setOnColor1     READ getOnColor1   )
    Q_PROPERTY(QColor onColor2      WRITE setOnColor2     READ getOnColor2   )
    Q_PROPERTY(QColor offColor1     WRITE setOffColor1    READ getOffColor1  )
    Q_PROPERTY(QColor offColor2     WRITE setOffColor2    READ getOffColor2  )
    Q_OBJECT
    public:
        QTSLedIndicator(QWidget *parent);

        void setOnColor1(QColor c)  { onColor1  = c;    }
        void setOffColor1(QColor c) { offColor1 = c;    }
        void setOnColor2(QColor c)  { onColor2  = c;    }
        void setOffColor2(QColor c) { offColor2 = c;    }

        QColor getOnColor1(void)    { return onColor1;  }
        QColor getOffColor1(void)   { return offColor1; }
        QColor getOnColor2(void)    { return onColor2;  }
        QColor getOffColor2(void)   { return offColor2; }

        void SetState(int Input) { IsOn = Input; repaint(); }
        QString GetColor() { return Color; }
        void SetColor(QString Color);



    protected:
        virtual void paintEvent (QPaintEvent *event);
        virtual void resizeEvent(QResizeEvent *event);

    public slots:
         void GreenColor();
         void RedColor();
         void YellowColor();

    private:
        static const qreal scaledSize;  /* init in cpp */
        QColor  onColor1, offColor1;
        QColor  onColor2, offColor2;
        QPixmap ledBuffer;
        int IsOn =0;
        QString Color = "Green";

};

