/* 
 * File:   PolylineItem.h
 * Author: blueck
 *
 * Created on 29. Juni 2010, 11:32
 */

#include "FillItem.h"
#include <QtCore/QPoint>
#include <QtCore/QList>

#ifndef POLYLINEITEM_H
#define	POLYLINEITEM_H

/**
 * Polyline shape which draws multiple lines without filling.
 */
class PolylineItem : public FillItem
{
public:
    /**
     * Creates a PolyLine Item
     * @param color the line color
     * @param groupNr the group number
     * @param points array of QPoint
     * @param numberOfpoints the number of QPoint objects in the points array
     */
    PolylineItem(const QColor &color, int groupNr, const QList<QPoint> &points);

    /**
     * Destructor
     */
    virtual ~PolylineItem();

    /**
     * removes all points
     */
    void removeAllPoints();

    /**
     * Adds a point to the polyline
     * @param point the point to add
     */
    void addPoint(QPoint point);

    /**
     * Removes a point
     * @param point the point to remove
     */
    void removePoint(QPoint point);

    /**
     * Returns the number of points
     * @return  the number of points
     */
    int getNumberOfPoints();

    /**
     * Returns an array of Qpoint
     * @return a QPoint array
     */
    QPoint* getAllPoints();

    /**
     * Returns DrawType#Polyline
     * @return DrawType#Polyline
     */
    DrawType getType() {return Polyline;};

    /**
     * @see DrawItem#draw
     */
    void draw(QPainter* painter, QRectF &source, QRectF &target);
protected:
    /** List of points for the polyline*/
    QList<QPoint> points;
};

#endif	/* POLYLINEITEM_H */

