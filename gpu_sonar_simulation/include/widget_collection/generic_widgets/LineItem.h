/* 
 * File:   LineItem.h
 * Author: blueck
 *
 * Created on 24. Juni 2010, 14:24
 */

#include "DrawItem.h"
#include <QtGui/QPen>

#ifndef LINEITEM_H
#define	LINEITEM_H

/**
 * LineItem for drawing a line from one point to another
 */
class LineItem : public DrawItem
{
    Q_OBJECT
public:
    /**
     * Creates a LIneItem
     * @param posX the x position of the starting point
     * @param posY teh y position of the starting point
     * @param groupNr teh group number
     * @param color the color
     * @param endX the x position of the endpoint
     * @param endY the y position of the end point
     */
    LineItem(int posX, int posY, int groupNr, const QColor &color, int endX, int endY);

    /**
     * Destructor
     */
    virtual ~LineItem(){};
public Q_SLOTS:
    /**
     * Returns the x position of the end point
     * @return the x position of the end point
     */
    int getEndX(){return endX;};

    /**
     * Retuns the y position of the end point
     * @return  the y poistion of the end point
     */
    int getEndY(){return endY;};

    /**
     * Returns DrawType#Line
     * @return DrawType#Line
     */
    DrawType getType(){return Line;};

    /**
     * Sets the x position of the end point
     * @param endX the x position of the end point
     */
    void setEndX(int endX) {this->endX = endX;};

    /**
     * Sets the y position the end point
     * @param endY the y position of the end point
     */
    void setEndY(int endY) {this->endY = endY;};

    /**
     * @see DrawItem#draw
     */
    void renderOnGl(QGLWidget &widget,QRectF &source,QRectF &target);
    void draw(QPainter* painter, QRectF &source, QRectF &target);
private:
    /** The x position of the end point*/
    int endX;
    /** The y poistion of the end point*/
    int endY;
    
};

#endif	/* LINEITEM_H */

