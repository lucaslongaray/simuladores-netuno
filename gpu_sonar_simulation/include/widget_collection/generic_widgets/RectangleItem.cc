/* 
 * File:   RectangleItem.cc
 * Author: blueck
 * 
 * Created on 29. Juni 2010, 10:34
 */

#include <QtGui/qpainter.h>

#include "RectangleItem.h"

RectangleItem::RectangleItem(int posX, int posY, int groupNr, const QColor &color, int width, int height)
    : FillItem(posX, posY, groupNr, color)
{
    this->width = width;
    this->height = height;
}

void RectangleItem::draw(QPainter* painter, QRectF &source, QRectF &target)
{
    FillItem::draw(painter,source,target);
    painter->drawRect(posX, posY, width, height);    
}


