/* 
 * File:   FillItem.h
 * Author: blueck
 *
 * Created on 29. Juni 2010, 14:25
 */

#include "DrawItem.h"

#ifndef FILLITEM_H
#define	FILLITEM_H

/**
 * Abstract class which extends the DrawItem with fillable functionality.
 * All shapes which need support for filling should subclass FillItem instead of
 * DrawItem. Note that if no border is drawn and no interiorColor is given nothing will
 * be drawn.
 */
class FillItem : public DrawItem
{
public:
    /**
     * Generates a FillItem
     * @param posX the xposition
     * @param posY the y poistion
     * @param groupNr the group number
     * @param color the border color
     */
    FillItem(int posX, int posY, int groupNr, const QColor &color);

    /**
     * Returns whether the border is drawn
     * @return if the border shall be drawn
     */
    bool isDrawBorder() {return drawBorder;};

    /**
     * Retunrs the fill color or NULL if the shape shall not be filles
     * @return teh fill color or NULL
     */
    QColor* getInteriorColor() {return interiorColor;};

    /**
     * Sets whether a border shall be drawn
     * @param drawBorder if a border shall be drawn
     */
    void setDrawBorder(bool drawBorder) {this->drawBorder = drawBorder;};

    /**
     * Sets the color to fill the shape
     * @param interiorColor the color to fill the shape or a NULL pointer if no filling is available
     */
    void setInteriorColor(QColor* interiorColor) {this->interiorColor = interiorColor;};

    /**
     * @see DrawItem#draw(QPainter*)
     */
    void draw(QPainter* painter, QRectF &source, QRectF &target);
protected:
    /**
     * Adds the brush style handling filling the shape
     * @param painter the apinter with which to draw the shape afterwards
     */
    void addBrushStyle(QPainter* painter);
    /** Determines whether the border is drawn*/
    bool drawBorder;
    /**The interior color if a fill shoud take place or NULL otherwise*/
    QColor* interiorColor;
};

#endif	/* FILLITEM_H */

