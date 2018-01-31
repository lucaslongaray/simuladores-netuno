/* 
 * File:   PolylineItem.cc
 * Author: blueck
 * 
 * Created on 29. Juni 2010, 11:32
 */

#include <QtGui/qpainter.h>


#include <QtCore/qvector.h>

#include "PolylineItem.h"

PolylineItem::PolylineItem(const QColor &color, int groupNr, const QList<QPoint> &points)
    : points(points),FillItem(0, 0, groupNr, color)
{
}

PolylineItem::~PolylineItem()
{
}

void PolylineItem::draw(QPainter* painter, QRectF &source, QRectF &target)
{
    addPenStyle(painter);
    painter->drawPolyline(getAllPoints(), getNumberOfPoints());
}

void PolylineItem::addPoint(QPoint point)
{
    points.push_back(point);
}

void PolylineItem::removeAllPoints()
{
    points.clear();
}

void PolylineItem::removePoint(QPoint point)
{
    points.removeAll(point);
}

int PolylineItem::getNumberOfPoints()
{
    return points.size();
}

QPoint* PolylineItem::getAllPoints()
{
    return points.toVector().data();
}

