/* 
 * File:   PolygonItem.h
 * Author: blueck
 *
 * Created on 29. Juni 2010, 12:06
 */

#include "PolylineItem.h"

#ifndef POLYGONITEM_H
#define	POLYGONITEM_H

/**
 * Class whioch implements a polygon shape
 */
class PolygonItem : public PolylineItem
{
public:
    /**
     * Creates a polygon
     * @param color the color of the border
     * @param groupNr the group number
     * @param points array of Qpoint creating of the polygons edges
     * @param numberOfpoints the number of points in the points array
     */
    PolygonItem(const QColor &color, int groupNr, const QList<QPoint> &points);

    /**
     * Destructor
     */
    virtual ~PolygonItem();

    /**
     * Returns DrawType#Polygon
     * @return DrawType#Polygon
     */
    DrawType getType() {return Polygon;};

    /**
     * @see DrawItem#draw
     */
    void draw(QPainter* painter, QRectF &source, QRectF &target);

};

#endif	/* POLYGONITEM_H */

