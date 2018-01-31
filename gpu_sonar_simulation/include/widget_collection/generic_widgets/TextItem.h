/* 
 * File:   TextItem.h
 * Author: blueck
 *
 * Created on 24. Juni 2010, 13:14
 */

#include "DrawItem.h"
#include <QtGui/QFont>


#ifndef TEXTITEM_H
#define	TEXTITEM_H

/**
 * DrawItem that draws text.<br>
 * the standard font used is QFont("Serif", 24, QFont::Normal)
 */
class TextItem : public DrawItem
{
    Q_OBJECT
public:
    /**
     * Creates a Textitem
     * @param posX the x start position
     * @param posY the y start position
     * @param groupNr the group number
     * @param color the color of teh text
     * @param text the text to display
     */
    TextItem(int posX, int posY, int groupNr, const QString &text);

    /**
     * Destructor
     */
    virtual ~TextItem() {};
    
    /**
     * @see DrawItem#draw
     */
    void draw(QPainter* painter, QRectF &source, QRectF &target);
    virtual void renderOnGl(QGLWidget &widget,QRectF &source,QRectF &target);

  public Q_SLOTS:
    /**
     * Returns the text of the item
     * @return the text of the item
     */
    QString getText() {return text;};

    /**
     * Returns the font used
     * @return the font used.
     */
    QFont getFont(){return font;};

    /**
     * Returns DrawType#Text
     * @return DrawType#Text
     */
    DrawType getType(){return Text;};

    /**
     * Sets the text of the item
     * @param text the text of the item
     */
    void setText(const QString &text) {this->text = text;};

    /**
     * Sets the font of the item
     * @param font the font of the item
     */
    void setFont(const QFont &font) {this->font = font; original_point_size = font.pointSizeF();};
    void setBackgroundColor(QColor color) {this->background_color = color;background=true;};


private:
    /** The text of the item*/
    QString text;
    /** The font of the item*/
    QFont font;
    bool background;
    QColor background_color;
    float original_point_size;
};

#endif	/* TEXTITEM_H */

