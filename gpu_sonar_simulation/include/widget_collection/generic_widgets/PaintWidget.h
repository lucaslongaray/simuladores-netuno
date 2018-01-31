#ifndef PAINT_WIDGET_H
#define PAINT_WIDGET_H

#include <QWidget>
#include <widget_collection/generic_widgets/DrawItem.h>

class PaintWidget : public QWidget{
	Q_OBJECT

public:
		PaintWidget(QWidget *parent);
		~PaintWidget();


public Q_SLOTS:

    /**
     * Adds a shape to the image
     * @param drawItem teh itrem which to draw. keep the item given to remove it afterwards
     */
    QObject* addItem(QObject* object);
    QObject* addPoints(const QList<int> &points_x,const QList<int> &points_y,int groupNr, const QColor &color);
    /**
     * Adds a Text to the image and all successive images
     * @param xPos the starting x position
     * @param yPos the starting y position
     * @param groupNr the group number
     * @param text the text to be displayed
     * @return pointer to a #seeDrawItem to remove the text afterwards.
     */
    QObject* addText(int xPos, int yPos, int groupNr, const QString &text);

    /**
     * Adds a line to the images and all successive images
     * @param xPos the starting x position
     * @param yPos the starting y position
     * @param groupNr the group number
     * @param color the color of the line
     * @param endX the ending x position
     * @param endY the ending y position
     * @return pointer to a #seeDrawItem to remove the line afterwards
     */
    QObject* addLine(int xPos, int yPos, int groupNr, const QColor &color, int endX, int endY);

    /**
     * Adds an ellipse to the image and all successive images. the elipse will be drawn
     * within an outlying rectangle specified.<br>
     * The ellipse will not be filles and the border will have the minimum width.
     * @param xPos the top left x position of the rectangle
     * @param yPos the top left y position of the rectangle
     * @param groupNr teh group number
     * @param color the color of the ellipses border
     * @param width the width of the rectangle
     * @param height the height of the rectangle
     * @return pointer to a #seeDrawItem to remove the ellipse afterwards
     */
    QObject* addEllipse(int xPos, int yPos, int groupNr, const QColor &color, int width, int height);

    /**
     * Adds a rectangle to the image and all successive images.
     * The rectangle will not be filles and the border will have the minimum width.
     * @param xPos the top left x position of the rectangle
     * @param yPos the top left y position of the rectangle
     * @param groupNr the group number
     * @param color the color of the rectangles border
     * @param width the width of the rectangle
		*/
    
		/**
     * Adds a rectangle to the image and all successive images.
     * The rectangle will not be filles and the border will have the minimum width.
     * @param xPos the top left x position of the rectangle
     * @param yPos the top left y position of the rectangle
     * @param groupNr the group number
     * @param color the color of the rectangles border
     * @param width the width of the rectangle
     * @param height the height of the rectangle
     * @return pointer to a #seeDrawItem to remove the ellipse afterwards
     */
    QObject* addRectangle(int xPos, int yPos, int groupNr, const QColor &color, int width, int height);

    /**
     * Adds multiple lines
     * @param groupNr the number of the group
     * @param color the color of the lines
     * @param points the points weher a line starts/ends
     * @param numberOfPoints the number of points in the points array
     * @return pointer to a DrawItem
     */
    QObject* addPolyline(int groupNr, const QColor &color, const QList<QPoint> &points);

    /**
     * Adds a Polygon with the given points. An additional line will be drawn between
     * the last and the first poin t given. Polygons can be filles unlike Polylines
     * @param groupNr the group number
     * @param color the color of the lines
     * @param points the points of the polygon
     * @param numberOfPoints the number of points in the points array
     * @return a DrawItem containing the Polygon
     */
    QObject* addPolygon(int groupNr, const QColor &color, const QList<QPoint> &points);

    /**
     * Remobves a shape from the image
     * @param drawItem the shape to remove
     */
    QObject* removeItem(QObject* drawItem,bool delete_object);
    
     /**
     * Removes all shapes from the iamges
     */
    void removeAllItems(bool delete_objects);
    
		
protected:
    /**
     * Adds all shapes to the image
     * @param shownImage teh iamge to add the shapes to
     */
    void drawDrawItemsToImage(QImage &image,bool all=false);
    void drawDrawItemsToPainter(QPainter &painter,bool all=false);
    void paintEvent(QPaintEvent *event);
    
		
		/** The format used in the widget*/
    /** List of all Draw Items*/
    QList<DrawItem*> items;
    /** List of group numbers currently disabled*/
    QList<int> disabledGroups;
		bool isOpenGL;

};

#endif
