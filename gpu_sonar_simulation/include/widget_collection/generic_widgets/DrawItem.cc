/* 
 * File:   DrawItem.cc
 * Author: blueck
 * 
 * Created on 24. Juni 2010, 13:10
 */

#include "DrawItem.h"
int DrawItem::_ID = 0;

DrawItem::DrawItem(int posX, int posY, int groupNr, const QColor &color):
groupNr(groupNr),
posX(posX),
posY(posY),
color(color),
lineWidth(0),
penCapStyle(Qt::SquareCap),
penStyle(Qt::SolidLine),
m_id(_ID++),
brender_on_opengl(false),
position_factor_x(0),
position_factor_y(0)
{
 
}

void DrawItem::addPenStyle(QPainter* painter)
{
    QPen pen;
    pen.setCapStyle(penCapStyle);
    pen.setStyle(penStyle);
    pen.setWidth(lineWidth);
    pen.setColor(color);
    painter->setPen(pen);
}


