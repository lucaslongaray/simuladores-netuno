/*
 * File:   SonarViewGL.h
 * Author: Matthias Goldhoorn (matthias.goldhoorn@dfki.de)
 *
 */

#ifndef SONARVIEWGL_H
#define	SONARVIEWGL_H

#include "image_view_old/ImageViewOldGL.h"
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <stdint.h>

class SonarViewGL : public ImageViewOldGL
{
    Q_OBJECT
    Q_CLASSINFO("Author", "Matthias Goldhoorn")

public:
    SonarViewGL(ImageViewOld &parent, unsigned int maximumBearings=6400);
    virtual ~SonarViewGL();
     QSize minimumSizeHint() const;
     QSize sizeHint() const;
     void setPosition(int xRot,int yRot, int zRot, int xMove, int yMove, int zMove, float zoom, int width, int height);
     void setData(const std::vector<uint8_t> data, int bearing);
     void reset(double currentScale);
     void setGroundTrue(double sizeX,double sizeY,double posX,double posY,double rot);
     void drawEllipse(float xradius, float yradius);

public Q_SLOTS:
     void repaintFunc();
     void setXRotation(int angle);
     void setYRotation(int angle);
     void setZRotation(int angle);
     void setxPosition(int value);
     void setyPosition(int value);
     void setzPosition(int value);
     void setWallDist(int bearing, int dist,int dist2);
     void keyPressEvent ( QKeyEvent * event );
     void setPosition(const double posX, const double posY, const double sigmaX=0, const double sigmaY=0);
     void setOrientation(const double orientation);
     void setZoomMin(float value);
     void setZoomMax(float value);

protected:
     void initializeGL();
     void paintGL();
     void resizeGL(int width, int height);
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);
     void wheelEvent(QWheelEvent *event);
     void checkGL(const char*);
     bool deleteSurfaces;
     void drawList();
     void drawFaces();
     void drawSurfaces();
     int cloudSize;
     void paintSonar(); 
     double factor;
     GLuint makeObject();
     GLuint show_pointcloud();
     GLuint groundPlane;
     void normalizeAngle(int *angle);
     void createAvalon(); 
     float zoom, zoom_min, zoom_max;
     int lastBearing;
     int lastWallBearing;
     double orientation;

     inline double max(double a,double b){
     	return (a>b)?a:b;
     }
     const unsigned int maximumBearings;

     std::vector<std::pair<GLuint, QColor> > colorList; 
     GLuint *wallDist, *bearingList;
     GLuint groundTrue,avalon;
     bool *bearingChanged, *listValid;
     bool dataChanged;     
     bool paintGroundTrue,paintWall;

     int xRot;
     int yRot;
     int zRot;
     int xShift;
     int yShift;
     int zShift;
     int medium_ableitung;
     int overlay_border;
     double currentScale;
     double pos[2];
     double sigmaPos[2];


     unsigned int oldSize;
     volatile GLuint *indices;
     volatile GLubyte *colors;
     volatile GLfloat *vertecies;
     QPoint lastPos;
     QPoint lastPosTrans;
     QTimer repaintTimer;
     QMutex mutex;

Q_SIGNALS:
     void xRotationChanged(int angle);
     void yRotationChanged(int angle);
     void zRotationChanged(int angle);
     void xPositionChanged(int value);
     void yPositionChanged(int value);
     void zPositionChanged(int value);

};

#endif	

