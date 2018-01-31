/* 
 * File:   PointItem.h
 * Author: blueck
 *
 * Created on 24. Juni 2010, 14:24
 */

#include "DrawItem.h"
#include <QtGui/QPen>
#include <stdexcept>

#ifndef POINTIMTEM_H
#define	POINTIMTEM_H

/**
 * PointItem for drawing a line from one point to another
 */
class PointItem : public DrawItem
{
    Q_OBJECT
public:
    PointItem(const QList<int> &points_x,const QList<int> &points_y, int groupNr, const QColor &color);
    virtual ~PointItem(){};

public Q_SLOTS:
    void setPoints(const QList<int> &points_x,const QList<int> &points_y)
    {
      if(points_x.size() != points_y.size())
        throw std::runtime_error("size of points_x differs from the size of points_y");

      this->points_x = points_x;
      this->points_y = points_y;
      
      if(points_x.size() > 0)
      {
        setPosX(points_x.first());
        setPosY(points_y.first());
      }
    }

    DrawType getType(){return Point;};

    /**
     * @see DrawItem#draw
     */
    void renderOnGl(QGLWidget &widget,QRectF &source,QRectF &target);
    void draw(QPainter* painter, QRectF &source, QRectF &target);
private:
    QList<int> points_x;
    QList<int> points_y;
};

#endif	/*POINTIMTEM_H*/

