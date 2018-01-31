/* 
 * File:   ImageViewGL.cc
 * Author: blueck
 * 
 * Created on 17. Juni 2010, 14:14
 */
#include <iostream>
#include "ImageViewOldGL.h"
#include "ImageViewOld.h"

ImageViewOldGL::ImageViewOldGL(ImageViewOld &parent):
  QGLWidget(&parent),
  image_view(parent)
{
}

ImageViewOldGL::~ImageViewOldGL()
{
}

void ImageViewOldGL::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawPixels(image_view.image.width(), image_view.image.height(), GL_RGB, GL_UNSIGNED_BYTE, image_view.image.bits());
    QList<DrawItem*>::iterator iter = image_view.items.begin();
    for(;iter != image_view.items.end();++iter)
    {
      if(!image_view.disabledGroups.contains((*iter)->getGroupNr()))
        if((*iter)->onOpenGL())
          (*iter)->renderOnGl(*this,image_view.source,image_view.target);
    }
}

void ImageViewOldGL::setGLViewPoint(int display_width,int display_height)
{
  if(!display_width || !display_height)
  {
    display_width = width();
    display_height = height();
  }

  float x =1; 
  float y =1; 
 
  if (image_view.image.width() && image_view.image.height())
  {
    x = ((float)display_width)/ image_view.image.width();
    y = ((float)display_height)/ image_view.image.height();
  }

  int x_offset = 0;
  int y_offset = 0;
  if(aspect_ratio)
  {
    if(x < y)
    {
      y_offset =  -0.5f*(display_height-x*image_view.image.height());
      y = x;
    }
    else
    {
      x_offset =  0.5f*(display_width-y*image_view.image.width());
      x = y;
    }
  }
  glPixelZoom(x,-y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, display_width, 0, display_height, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0,0, display_width, display_height);
  glRasterPos2i(x_offset,display_height+y_offset);
  
 // target.setRect(x_offset,y_offset,display_width-x_offset,display_width-y_offset);
 // source.setRect(0,0,image->width(),image->height());
}

void ImageViewOldGL::resizeGL(int w, int h)
{
  setGLViewPoint(w,h);
  paintGL();
}
