/* 
 * File:   TextItem.cc
 * Author: blueck
 * 
 * Created on 24. Juni 2010, 13:14
 */

#include "TextItem.h"
#include <iostream>


TextItem::TextItem(int posX, int posY, int groupNr,const QString &text) :
DrawItem(posX, posY, groupNr, QColor(0,0,0)),text(text),font(QFont("Serif")),background(false)
{
  original_point_size = font.pointSizeF();
}

void TextItem::draw(QPainter* painter, QRectF &source, QRectF &target)
{
    QRect rect = painter->viewport();
    painter->setFont(font);
    painter->setPen(color);
    if(background)
    {
      painter->setBackgroundMode(Qt::OpaqueMode);
      painter->setBackground(QBrush(background_color));
    }
    painter->drawText(position_factor_x*rect.width()+posX,position_factor_y*rect.height(), text);
}

void TextItem::renderOnGl(QGLWidget &widget,QRectF &source, QRectF &target)
{
  widget.qglColor(color);
  widget.renderText(position_factor_x*widget.width()+posX,position_factor_y*widget.height()+posY,text,font);
}
