#include <PaintWidget.h>


#include<ArrowItem.h>
#include<DrawItem.h>
#include<EllipseItem.h>
#include<FillItem.h>
#include<LineItem.h>
#include<PointItem.h>
#include<PolygonItem.h>
#include<PolylineItem.h>
#include<RectangleItem.h>
#include<TextItem.h>


PaintWidget::PaintWidget(QWidget *parent):
QWidget(parent),
isOpenGL(false)
//image(0, 0, QImage::Format_RGB888)
{
};

PaintWidget::~PaintWidget()
{
};

QObject* PaintWidget::addText(int xPos, int yPos, int groupNr, const QString &text)
{
    TextItem* textItem = new TextItem(xPos, yPos, groupNr, text);
    if(isOpenGL)
      textItem->openGL(true);
    items.push_back(textItem);
    return textItem;
}

QObject* PaintWidget::addLine(int xPos, int yPos, int groupNr, const QColor &color, int endX, int endY)
{
    LineItem* item = new LineItem(xPos, yPos, groupNr, color, endX, endY);
    if(isOpenGL)
      item->openGL(true);

    items.push_back(item);
    return item;
}

QObject* PaintWidget::addEllipse(int xPos, int yPos, int groupNr, const QColor &color, int width, int height)
{
    EllipseItem* item = new EllipseItem(xPos, yPos, groupNr, color, width, height);
    if(isOpenGL)
      item->openGL(true);

    items.push_back(item);
    return item;
}

QObject* PaintWidget::addRectangle(int xPos, int yPos, int groupNr, const QColor &color, int width, int height)
{
    RectangleItem* item = new RectangleItem(xPos, yPos, groupNr, color, width, height);
    if(isOpenGL)
      item->openGL(true);

    items.push_back(item);
    return item;
}
QObject* PaintWidget::addPolyline(int groupNr, const QColor &color, const QList<QPoint> &points)
{
    PolylineItem* item = new PolylineItem(color, groupNr, points);
    if(isOpenGL)
      item->openGL(true);

    items.push_back(item);
    return item;
}

QObject* PaintWidget::addPolygon(int groupNr, const QColor &color, const QList<QPoint> &points)
{
    PolygonItem* item = new PolygonItem(color, groupNr, points);
    if(isOpenGL)
      item->openGL(true);

    items.push_back(item);
    return item;
}

QObject* PaintWidget::addItem(QObject* object)
{
    DrawItem* drawItem = dynamic_cast<DrawItem*>(object);
    if(isOpenGL)
      drawItem->openGL(true);

    items.push_back(drawItem);
    return object;
}

QObject* PaintWidget::addPoints(const QList<int> &points_x,const QList<int> &points_y,int groupNr, const QColor &color)
{
    DrawItem* drawItem = new PointItem(points_x,points_y,groupNr,color);
    if(isOpenGL)
      drawItem->openGL(true);
    items.push_back(drawItem);
    return drawItem;
}

 QObject* PaintWidget::removeItem(QObject* object,bool delete_object)
{
    DrawItem *draw_item = dynamic_cast<DrawItem*>(object);
    items.removeAll(draw_item);
    if(delete_object)
    {
      delete object;
      object = NULL;
    }
    return object;
}

void PaintWidget::removeAllItems(bool delete_objects)
{
   if(delete_objects)
   {
      QList<DrawItem*>::iterator iter = items.begin();
      for(;iter != items.end();++iter)
          delete(*iter);
   }
   items.clear();
}


void PaintWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  drawDrawItemsToPainter(painter,true);
}

void PaintWidget::drawDrawItemsToPainter(QPainter &painter,bool all)
{
    if(!isOpenGL)
      all = true;

    QRectF target = painter.viewport();

    if(!items.empty())
    {
        painter.setRenderHint(QPainter::TextAntialiasing, true);
	QList<DrawItem*>::iterator iter = items.begin();
        for(;iter != items.end();++iter)
        {
	  if(!disabledGroups.contains((*iter)->getGroupNr()))
	    if(!(*iter)->onOpenGL() || all == true)
                (*iter)->draw(&painter,target,target);
        }
    }
}

