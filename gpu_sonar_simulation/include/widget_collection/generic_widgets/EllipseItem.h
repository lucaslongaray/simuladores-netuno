/* 
 * File:   EllipseItem.h
 * Author: blueck
 *
 * Created on 24. Juni 2010, 14:42
 */

#include "DrawItem.h"
#include "RectangleItem.h"

#ifndef ELLIPSEITEM_H
#define	ELLIPSEITEM_H

/**
 * Ellipse Item which subclasses a Rectangle item and currently does not add any functionality.
 * Ellipses will be drawn within an oulying rectangle.
 */
class EllipseItem : public RectangleItem
{
public:
    /**
     * Creates an Ellipseitem
     * @param posX the x position of the outlying rectangle
     * @param posY the y position of the outlying rectangle
     * @param groupNr teh group number
     * @param color the color of the border
     * @param width the width of the outlying rectangle
     * @param height the height of the oulying rectangle
     */
    EllipseItem(int posX, int posY, int groupNr,const QColor &color, int width, int height);

    /**
     * Destructor
     */
    virtual ~EllipseItem(){};

    /**
     * Returns DrawType::Ellipse
     * @return DrawType::Ellipse
     */
    DrawType getType(){return Ellipse;};

    /**
     * @see DrawItem#draw
     */
    void draw(QPainter* painter, QRectF &source, QRectF &target);
    
};

#endif	/* ELLIPSEITEM_H */

