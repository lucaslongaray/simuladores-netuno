/* 
 * File:   LineItem.cc
 * Author: blueck
 * 
 * Created on 24. Juni 2010, 14:24
 */

#include <QtGui/qpainter.h>

#include "LineItem.h"

LineItem::LineItem(int posX, int posY, int groupNr, const QColor &color, int endX, int endY)
    : DrawItem(posX, posY, groupNr, color)
{
    this->endX = endX;
    this->endY = endY;
}

void LineItem::draw(QPainter* painter, QRectF &source, QRectF &target)
{
  float factor_x = ((float)target.width())/source.width();
  float factor_y = ((float)target.height())/source.height();
  addPenStyle(painter);
  painter->drawLine(factor_x*posX+target.x(),
                    posY*factor_y+target.y(),
                    target.x()+factor_x*endX,
                    target.y()+factor_y*endY);
}


void LineItem::renderOnGl(QGLWidget &widget,QRectF &source,QRectF &target)
{
  float factor_x = ((float)target.width())/source.width();
  float factor_y = ((float)target.height())/source.height();
  glColor4ub(color.red(),color.green(),color.blue(),color.alpha());
  glBegin(GL_LINES);
  glVertex3f(factor_x*posX+target.x(), widget.height()-(posY*factor_y+target.y()), 0.0f); // ending point of the line
  glVertex3f(target.x()+factor_x*endX, widget.height()-(target.y()+factor_y*endY), 0.0f); // ending point of the line
  glEnd( );
}
