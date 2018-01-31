/* 
 * File:   PolygonItem.cc
 * Author: blueck
 * 
 * Created on 29. Juni 2010, 12:06
 */

#include <QtGui/qpainter.h>


#include "PolygonItem.h"
#include "FillItem.h"

PolygonItem::PolygonItem(const QColor &color, int groupNr,const QList<QPoint> &points)
    : PolylineItem(color, groupNr, points)
{
}

PolygonItem::~PolygonItem()
{
}

void PolygonItem::draw(QPainter* painter, QRectF &source, QRectF &target)
{
    FillItem::draw(painter,source,target);
    painter->drawPolygon(getAllPoints(), getNumberOfPoints());
}

