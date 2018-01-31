/* 
 * File:   PointItem.cc
 * Author: blueck
 * 
 * Created on 24. Juni 2010, 14:24
 */

#include <QtGui/qpainter.h>

#include "PointItem.h"
#include <iostream>

PointItem::PointItem(const QList<int> &points_x,const QList<int> &points_y, int groupNr, const QColor &color)
    : DrawItem(0, 0, groupNr, color)
{
    this->points_x = points_x;
    this->points_y = points_y;
    
}

void PointItem::draw(QPainter* painter, QRectF &source, QRectF &target)
{
  //drawing on non opengl contexts is not supported at the moment
}


void PointItem::renderOnGl(QGLWidget &widget,QRectF &source,QRectF &target)
{
  float factor_x = ((float)target.width())/source.width();
  float factor_y = ((float)target.height())/source.height();
  glColor4ub(color.red(),color.green(),color.blue(),color.alpha());
  QList<int>::iterator iter_x = points_x.begin();
  QList<int>::iterator iter_y = points_y.begin();
  
  glBegin(GL_POINTS);
  while(iter_x != points_x.end())
  {
//    std::cout << *iter_x << std::endl;
    glVertex3f(factor_x*(*(iter_x++))+target.x(), widget.height()-(factor_y*(*(iter_y++))+target.y()), 0.0f);
  }
  glEnd( );
}
