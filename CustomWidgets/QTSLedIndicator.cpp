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


#include <QPainter>

#include "QTSLedIndicator.h"

const qreal QTSLedIndicator::scaledSize = 1000; /* Visual Studio static const mess */

QTSLedIndicator::QTSLedIndicator(QWidget *parent) : QAbstractButton(parent)
{
    setMinimumSize(15,15);
//    setCheckable(true);
    onColor1 =  QColor(255,0,0);
    onColor2 =  QColor(192,0,0);
    offColor1 = QColor(28,0,0);
    offColor2 = QColor(128,0,0);
    Color     = "Red";
}

void QTSLedIndicator::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    update();
}

void QTSLedIndicator::paintEvent(QPaintEvent *event) {
    qreal realSize = qMin(width(), height());
     Q_UNUSED(event);

    QRadialGradient gradient;
    QPainter painter(this);
    QPen     pen(Qt::black);
             pen.setWidth(1);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width()/2, height()/2);
    painter.scale(realSize/scaledSize, realSize/scaledSize);

    gradient = QRadialGradient (QPointF(-500,-500), 1500, QPointF(-500,-500));
    gradient.setColorAt(0, QColor(224,224,224));
    gradient.setColorAt(1, QColor(28,28,28));
    painter.setPen(pen);
    painter.setBrush(QBrush(gradient));
    painter.drawEllipse(QPointF(0,0), 500, 500);

    gradient = QRadialGradient (QPointF(500,500), 1500, QPointF(500,500));
    gradient.setColorAt(0, QColor(224,224,224));
    gradient.setColorAt(1, QColor(28,28,28));
    painter.setPen(pen);
    painter.setBrush(QBrush(gradient));
    painter.drawEllipse(QPointF(0,0), 450, 450);

    painter.setPen(pen);
    if( IsOn == -1 ) {
        RedColor();
        gradient = QRadialGradient (QPointF(-500,-500), 1500, QPointF(-500,-500));
        gradient.setColorAt(0, onColor1);
        gradient.setColorAt(1, onColor2);
    }
    else if(( IsOn == 0 )) {
        YellowColor();
        gradient = QRadialGradient (QPointF(-500,-500), 1500, QPointF(-500,-500));
        gradient.setColorAt(0, onColor1);
        gradient.setColorAt(1, onColor2);
    }
    else if(( IsOn == 1 ))
    {
        GreenColor();
        gradient = QRadialGradient (QPointF(-500,-500), 1500, QPointF(-500,-500));
        gradient.setColorAt(0, onColor1);
        gradient.setColorAt(1, onColor2);
    }
    else
    {
        gradient = QRadialGradient (QPointF(500,500), 1500, QPointF(500,500));
        gradient.setColorAt(0, offColor1);
        gradient.setColorAt(1, offColor2);
    }
    painter.setBrush(gradient);
    painter.drawEllipse(QPointF(0,0), 400, 400);
}



void QTSLedIndicator::GreenColor()
{
    onColor1 =  QColor(0,255,0);
    onColor2 =  QColor(0,192,0);
    offColor1 = QColor(0,28,0);
    offColor2 = QColor(0,128,0);
    Color = "Green";
    //this->repaint();
}

void QTSLedIndicator::RedColor()
{
    onColor1 =  QColor(255,0,0);
    onColor2 =  QColor(192,0,0);
    offColor1 = QColor(28,0,0);
    offColor2 = QColor(128,0,0);
    Color = "Red";
    //this->repaint();
}

void QTSLedIndicator::YellowColor()
{
    onColor1 =  QColor(255,255,0);
    onColor2 =  QColor(192,192,0);
    offColor1 = QColor(28,28,0);
    offColor2 = QColor(128,128,0);
    Color = "";
    //this->repaint();
}

void QTSLedIndicator::SetColor(QString Color)
{
    if(Color.compare("Red") == 0)
        RedColor();
    else if(Color.compare("Green") == 0)
        GreenColor();
}


