/* 
 * File:   RectangleItem.h
 * Author: blueck
 *
 * Created on 29. Juni 2010, 10:34
 */

#include "FillItem.h"

#ifndef RECTANGLEITEM_H
#define	RECTANGLEITEM_H

/**
 * Rectangle shape extending the fill item
 */
class RectangleItem : public FillItem
{
public:
    /**
     * Creates a rectangle item
     * @param posX the starting x position
     * @param posY the starting y position
     * @param groupNr the group number
     * @param color the border color
     * @param width the width of the rectangle
     * @param height the height of the rectangle
     */
    RectangleItem(int posX, int posY, int groupNr, const QColor &color, int width, int height);

    /**
     * Destructor
     */
    virtual ~RectangleItem(){};

    /**
     * Returns the width of the rectangle
     * @return the width of the rectangle
     */
    int getWidth(){return width;};

    /**
     * Returns the height of the rectangle
     * @return the height of the ractangle
     */
    int getHeight(){return height;};

    /**
     * Returns DrawType#Rectangle
     * @return DrawType#Rectangle
     */
    DrawType getType(){return Rectangle;};

    /**
     * Sets the width of the rectangle
     * @param width the width of the rectangle
     */
    void setWidth(int width) {this->width = width;};

    /**
     * Sets the height of the rectangle
     * @param height the height of the rectangle
     */
    void setHeight(int height) {this->height = height;};

    /**
     * @see DrawItem#draw
     */
    void draw(QPainter* painter, QRectF &source, QRectF &target);
protected:
    /** The width of the Rectangle*/
    int width;
    /** The height of the Rectangle*/
    int height;
    
};

#endif	/* RECTANGLEITEM_H */

