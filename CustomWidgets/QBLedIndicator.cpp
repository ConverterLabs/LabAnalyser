/***************************************************************************
 *   Copyright (C) 2010 by Tn                                              *
 *   thenobody@poczta.fm                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QPainter>
#include <QTimer>
#include "QBLedIndicator.h"


const qreal QBLedIndicator::scaledSize = 1000; /* Visual Studio static const mess */

QBLedIndicator::QBLedIndicator(QWidget *parent) : QAbstractButton(parent)
{
    setMinimumSize(15,15);
//    setCheckable(true);
    onColor =  QColor(255,0,0);
    offColor = QColor(128,0,0);
}

void QBLedIndicator::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    update();
}

void QBLedIndicator::paintEvent(QPaintEvent *event) {
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
    if( IsOn ) {
        gradient = QRadialGradient (QPointF(-500,-500), 1500, QPointF(-500,-500));
        gradient.setColorAt(0, onColor);
        gradient.setColorAt(1, QColor(onColor.red()*0.75,onColor.green()*0.75,onColor.blue()*0.75));
    } else {
        gradient = QRadialGradient (QPointF(500,500), 1500, QPointF(500,500));
        gradient.setColorAt(0, offColor);
        gradient.setColorAt(1, QColor(0,0,0));
    }

    painter.setBrush(gradient);
    painter.drawEllipse(QPointF(0,0), 400, 400);
}


void QBLedIndicator::SetState(bool Input)
{
    if(!this->running)
    {
        QTimer::singleShot(500, this, SLOT(TimeOut()));
    }
    running = Input;
    IsOn = Input;
}


void QBLedIndicator::TimeOut()
{

    if(this->running == 1)
    {
           QTimer::singleShot(500, this, SLOT(TimeOut()));
           IsOn = !IsOn;
    }
    repaint();

}

