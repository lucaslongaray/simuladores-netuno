/* 
 * File:   ImageViewOld.h
 * Author: blueck
 *
 * Created on 17. Juni 2010, 14:14
 */

#ifndef IMAGEVIEWOLD_H
#define	IMAGEVIEWOLD_H

#include <widget_collection/multi_view/MultiWidget.h>
#include <QtGui/QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QPen>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtGui/qfiledialog.h>
#include <QtGui/QMenu>
#include <QtCore/QString>
#include <QtDesigner/QDesignerExportWidget>

#include <iostream>
#include <frame_helper/src/FrameQImageConverter.h>

#include "ImageViewOldGL.h"

/**
 * Widget which displays an image. The dimensions and the format needs
 * to be given and images can then be added an will be shown immidiately.
 * <h1>Imaging</h1>
 * <p>
 * If an image which is larger than the specified dimension is added, only
 * the images visible parts (within the dimension specified) will be shown.<br>
 * Alternatively one can add an image and scale it. No scaling will be
 * done if the image already has the proportions given at start. Scaling is
 * costy though. If more images of a different size shall be added the best way
 * is to change the format via changeFormat. This will change the
 * format according to the new dimensions. This should be the default behaviour
 * e.g. if a cameras resolution was changed so is the widgets. Scaling
 * should only be done if a few images are changed or if the images are all quite
 * large and should be scaled to enhance the viewing. Note though, again, that
 * scaling <b>does</b> cost performance.
 * <p>
 * <h1>Drawing shapes</h1>
 * One can draw shapes on top of the image. Currently these are:
 * <ul>
 * <li>Texts</li>
 * <li>Lines</li>
 * <li>Ellipses</li>
 * <li>Rectangles</li>
 * <li>Polylines</li>
 * <li>Polygones</li>
 * </ul>
 * One can add the shapes via addItem or a specific convenience method like
 * addText, addLine etc. Added Items will be displayed on top of the image currently displayed and all
 * successive images. RemoveItems, removes the given item. Alternatively all specific shapes 
 * can be removed or all Items.<br>
 * Items can have a group number and all items with a specific group number can be shown or hidden.
 * Note that hidden is <b>NOT</b> the same as removed. Hidden just does not draw them, removed
 * items can not be retrieved by the widget (even though they might be from outside the class).<br>
 * <b>Note:</b>
 * After adding any shape no repaint will be done. You need to either explicitly call
 * repaint on the widget or add a different image will will automatically have the shapes added
 * ontop of it. Otherwise, if adding multiple shapes, repaint would be called very often which
 * especially when working with video would be a performance waste.
 * 
 * @author Bjoern Lueck, Alexander Duda
 * @version 0.1
 * @deprecated This class is deprecated and will no longer be maintained. You should use the new ImageView widget.
 */

class QDESIGNER_WIDGET_EXPORT ImageViewOld : public MultiWidget 
{
    Q_OBJECT
    Q_CLASSINFO("Author", "Alexander Duda")
    Q_PROPERTY(bool Use_OpenGL READ onOpenGL WRITE openGL USER false)
    Q_PROPERTY(bool Aspect_Ratio READ getAspectRatio WRITE setAspectRatio USER false)

public:
    friend class ImageViewOldGL;
    /**
     * Initializing the widget with the given format and dimensions
     * For format parameters @seeQImage
     * @param width the width of images put onto the widget
     * @param height the height of images put onto the widget
     */
    ImageViewOld(QWidget *parent = NULL,bool use_openGL = false);

    /**
     * Destructor cleaning up
     */
    virtual ~ImageViewOld();
    void contextMenuEvent ( QContextMenuEvent * event );
    int heightForWidth( int w ) {return w*image.height()/image.width(); };
    void mouseDoubleClickEvent ( QMouseEvent * event );
Q_SIGNALS:
    void clickImage(int x,int y);
    void userExport();

public Q_SLOTS:
    void emitUserExport();
    void update();
    void update2();
    void setDefaultImage();

    void setZoomEnabled(bool enabled);

    virtual void openGL(bool flag);
    bool onOpenGL(){return image_view_gl;};
    void setAspectRatio(bool value)
    {
      aspect_ratio=value;
      if(image_view_gl)
        image_view_gl->setAspectRatio(value);
    };
    bool getAspectRatio(){return aspect_ratio;};

    //for saving displayed frames
    void saveImage(bool overlay=true);
    bool saveImage2(QString path,bool overlay=true);
    bool saveImage3(const QString &mode, int pixel_size,  int width,  int height,const char* pbuffer, QString path,QString format);

    /** Removes any cropping previously set with crop(x, y, w, h) */
    void resetCrop();

    /** Crops the current image */
    void crop(int x, int y, int w, int h);
    

    /** Adds an image to the display, given raw data in \c pbuffer
     *
     * @arg mode the mode as a string. This maps to the values in the base::samples::frame::frame_mode_t enumeration (see below)
     * @arg pixel_size the size of a complete pixel, in bytes
     * @arg with the width of the image, in pixels
     * @arg height the height of the image, in pixels
     * @arg pbuffer the raw data
     *
     * The following modes are understood in \c mode:
     *
     * <ul>
     * <li>MODE_GRAYSCALE
     * <li>MODE_RGB
     * <li>MODE_UYVY
     * </ul>
     */
    void addRawImage(const QString &mode, int pixel_size, int width, int height,const char* pbuffer, const int size);
    void addFrame(const base::samples::frame::Frame &frame);

    /**
     * Adds a QImage to the widget.
     * @param image the image to be added
     */
     void addImage(const QImage &image);

    void setGroupStatus(int groupNr, bool enable);

    /**
     * Removes all groups stati. If there are still shapes with group numbers
     * they will all be visible afterwards
     */
    void clearGroups();

    int getHeight()const {return image.height();};
    int getWidth()const {return image.width();};
    int getFormat()const {return image.format();};

protected:
    /**
     * Adds all shapes to the image
     * @param shownImage teh iamge to add the shapes to
     */
    void drawDrawItemsToImage(QImage &image,bool all=false);
    void drawDrawItemsToPainter(QPainter &painter,bool all=false);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void calculateRects();
    std::pair<QPoint, bool> toImage(QPoint const& p);

    QMenu contextMenu;
    QAction *save_image_act;
    QAction *user_export_act;
    QString save_path;
    
    QImage originalImage; // holds the original image
    QImage image;
    QRect currentCrop;
    bool no_input;
    bool aspect_ratio;

    frame_helper::FrameQImageConverter frame_converter;
    ImageViewOldGL *image_view_gl;

    QRectF target;
    QRectF source;

    bool zoomEnabled;
    QPoint pressPoint;
    bool pressValid;
};

#endif	/* IMAGEVIEWOLD_H */

