/* 
 * File:   SonarViewGL.cc
 * Author: Matthias Goldhoorn (matthias.goldhoorn@dfki.de) 
 * 
 */

#include <stdexcept>
#include "SonarView.h"
#include "SonarViewGL.h"
#include <QtCore/QtPlugin>
#include "SonarViewGL.h"

SonarView::SonarView(QWidget *parent,bool use_openGL):
ImageViewOld(parent,false),
img(10, 10, QImage::Format_RGB888)
{
	resize(width(),height());	
	image_view_gl = 0;
	use_openGL=false;
	setOpenGL(use_openGL);
	lastScale=0;
	
}


SonarView::~SonarView()
{
}

void SonarView::setOpenGL(bool flag)
{
  if(flag)
  {
    //prevent activating if it is already activated
    if(image_view_gl)
      return;

    image_view_gl = new SonarViewGL(*this);
    if (!image_view_gl)
      return;
    image_view_gl->resize(width(),height());
    //image_view_gl->setAspectRatio(aspect_ratio);
    image_view_gl->show();
    //SonarViewGL *window = dynamic_cast<SonarViewGL*>(image_view_gl);
    //window->reset(0.024); //TODO hardcoded value
  }
  else
  {
    delete image_view_gl;
    image_view_gl = NULL;
  }
   
}

void SonarView::setDistance(double distance, double angle){
	SonarViewGL *window = dynamic_cast<SonarViewGL*>(image_view_gl);
	double bearing = angle/(M_PI*2.0)*6399.0;
	if(window){
		window->setWallDist(bearing,distance,0);
	}else{
		//printf("scale: %f, distance: %f\n",lastScale,distance);
		paintReference(angle,distance/lastScale);
	}
}

void SonarView::setSonarScan(const char *data_, int size, double angle, double timeBetweenBins,bool fromBearing){
	SonarViewGL *window = dynamic_cast<SonarViewGL*>(image_view_gl);
	double bearing = angle;
	double newScale = ((timeBetweenBins*640.0)*1e-9);
	if(!fromBearing){
		bearing = angle/(M_PI*2.0)*6399.0;
		newScale = timeBetweenBins*size/2.0;
	}
	newScale = (newScale*1500.0)/size;
	
	if(window){
		if(newScale != lastScale){
			lastScale = newScale;
			printf("new Scale: %f\n",newScale);	
			window->reset(newScale);
		}
	 
		std::vector<uint8_t> data;
		for(int i=0;i<size;i++){
			data.push_back(data_[i]);
		}
		window->setData(data,bearing);
	}else{
		lastScale =  newScale;
		if(img.size().width() != size*2.0){
			//img = QImage(size*2.0,size*2.0,QImage::Format_Mono);
			img = QImage(size*2.0,size*2.0,QImage::Format_RGB888);
			img.fill(0);
			printf("Creating new Image with: %i,%i\n",img.size().width(),img.size().height());
		}
		int begin,end;
		if(lastBearing-bearing>0 || lastBearing-bearing < -3000){
			begin = lastBearing;
			end = lastBearing-bearing;
		}else{
			begin = bearing;
			end = bearing-lastBearing;
		}

		for(int j=0;j<end;j++){
			int i = ((begin+j));
			if(i < 0 || i > 6399){
				break;
			}
			paintLine(((double)i)/6399*2.0*M_PI,(const uint8_t*)data_,(size_t)size);
		}
	}
	lastBearing = bearing;
        addImage(img);
	resize(width(),height());	
	//ImageView::update();
}

void SonarView::paintReference(double bearing, int distance){
	if(distance > img.size().width()){
		return;
	}
	QPainter painter(&img);
	double s = sin(bearing);
	double c = cos(bearing);
	int x = (img.size().width()/2.0)+(c*distance);
	int y = (img.size().width()/2.0)+(s*distance);	

	painter.setPen(QColor(255,255,0));
	painter.drawEllipse(QPoint(x,y),5,5);
        addImage(img);
	//painter.drawPoint(x,y);
}

void SonarView::paintPos(int posX, int posY, int sizeX, int sizeY){
	QPainter painter(&img);
	painter.setPen(QColor(255,0,0));
	painter.drawEllipse(QPoint(posX,posY),sizeX,sizeY);
        addImage(img);
}

void SonarView::paintLine(double bearing, const uint8_t *data, size_t len){
	double s = sin(bearing);
	double c = cos(bearing);
    for (size_t i = 0; i < len; i++)
        img.setPixel((len) + (c * i), (len) + (s * i), qRgb(data[i], data[i], data[i]));
}


void SonarView::keyPressEvent ( QKeyEvent * event ){
	SonarViewGL *window = dynamic_cast<SonarViewGL*>(image_view_gl);
	if(window) window->keyPressEvent(event);	
}


void SonarView::setPosition(double posX, double posY, double sigmaX, double sigmaY){
	SonarViewGL *window = dynamic_cast<SonarViewGL*>(image_view_gl);
	if(window)window->setPosition(posX,posY,sigmaX,sigmaY);	
}
  
void SonarView::setOrientation(const double orientation){
	SonarViewGL *window = dynamic_cast<SonarViewGL*>(image_view_gl);
	if(window)window->setOrientation(orientation);
}


void SonarView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(0, 0, img);
}

/*
void SonarView::resizeEvent ( QResizeEvent * event )
{
  ImageView::resizeEvent(event);
  int x_offset = 0;
  int y_offset = 0;
  if(aspect_ratio)
  {
    float x =1; 
    float y =1; 
    if (image.width() && image.height())
    {
      x = ((float)width())/ image.width();
      y = ((float)height())/ image.height();
    }
    if(x < y)
      y_offset =  0.5f*(height()-x*image.height());
    else
      x_offset =  0.5f*(width()-y*image.width());
  }
  target.setRect(x_offset, y_offset, width()-x_offset*2, height()-y_offset*2);
  source.setRect(0.0, 0.0, image.width(), image.height());
}
*/
