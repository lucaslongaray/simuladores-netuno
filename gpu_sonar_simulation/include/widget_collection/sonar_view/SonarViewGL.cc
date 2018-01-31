/* 
 * File:   SonarViewGL.cc
 * Author: Matthias Goldhoorn (matthias.goldhoorn@dfki.de) 
 * 
 */

#include <stdexcept>
#include "SonarViewGL.h"
#include <QtCore/QtPlugin>
#include <QtOpenGL/QtOpenGL>
#include <math.h>
#include <stdio.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif


SonarViewGL::SonarViewGL(ImageViewOld &parent,unsigned int maximumBearings):
ImageViewOldGL(parent),
maximumBearings(maximumBearings)
{
     colors = new GLubyte[maximumBearings];	
     vertecies = new GLfloat[maximumBearings];
     indices = new GLuint[maximumBearings];
     wallDist = new GLuint[maximumBearings];
     bearingList = new GLuint[maximumBearings];

     bearingChanged = new bool[maximumBearings];
     listValid = new bool[maximumBearings];

     xRot = 0;//180*16;
     yRot = 180*16;
     zRot = 0;
     xShift = 0;
     yShift = 0;
     zShift = 0;
     zoom = 40;
     zoom_min = -6000.0;
     zoom_max = 6000.0;
     overlay_border=0;
     medium_ableitung=1;
     factor=1;
	//repaintTimer.setSingleShot(false);
	//repaintTimer.setInterval(100);
	//connect(&repaintTimer,SIGNAL(timeout()),this,SLOT(repaintFunc()));
	//repaintTimer.start();
	
	for(int i=0;i<maximumBearings;i++){
		glDeleteLists(bearingList[i], 1);
		glDeleteLists(wallDist[i], 1);
		bearingList[i] = glGenLists( 1 );
		wallDist[i] = glGenLists( 1 );
		glNewList( wallDist[i], GL_COMPILE );
		glEndList();
		glNewList( bearingList[i], GL_COMPILE );
		glEndList();
//		listValid=false;
	}
     createAvalon();
     paintGroundTrue=true; 
	 paintWall=true;
     lastBearing=0;
     lastWallBearing=0;
}



void SonarViewGL::reset(double currentScale){
	this->currentScale = currentScale;
	createAvalon();
//	mutex.lock();
	for(int i=0;i<maximumBearings;i++){
		glDeleteLists(bearingList[i], 1);
		bearingList[i] = glGenLists( 1 );
		glNewList( bearingList[i], GL_COMPILE );
		glEndList();
//		listValid=false;
	}
//	listValid[i]=false;
//
//
	for(int i=0;i<maximumBearings;i++){
		glDeleteLists(wallDist[i], 1);
		wallDist[i] = glGenLists( 1 );
		glNewList( wallDist[i], GL_COMPILE );
		glEndList();
		lastBearing=0;
		lastWallBearing=0;
	}
//	mutex.unlock();
}




void SonarViewGL::setData(const std::vector<uint8_t> data,int bearing){
//        printf("Bearing %i\n",bearing);
        //for(int i=0;i<data.size();i++){
         // if(data[i]!= 0){
          //    printf("Got data value :%i\n",data[i]);
          //}
        //}
          
	if(bearing < 0 || bearing > maximumBearings){
		fprintf(stderr,"Cannot Set bearing is out of range\n");
	} 

	if(data.size() != oldSize){
		printf("deleteing lists new Size is: %i\n",(int)data.size());
		oldSize = data.size();
		delete colors;
		delete indices;
		delete vertecies;
     		colors = new GLubyte[data.size()*3*maximumBearings];	
		vertecies = new GLfloat[data.size()*3*maximumBearings];
		indices = new GLuint[data.size()*3*maximumBearings];
		//uint16_t c=0;
		for(unsigned int i=0;i<data.size()*3*maximumBearings; i++){
			colors[i]=0;
			vertecies[i]=0;
		
			//if((i%3)==0){
			//	vertecies[i]=0.001*(i%6399);
			//	colors[i]=++c;
			//}else{
			//	colors[i]=c;
			//	vertecies[i]=0;
			//}
			indices[i]=i;
		}
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
			fprintf(stderr,"Error in glwidet, please check value make no sense, prevent crash, not displaying data for index %i (%s:%i)\n",i,__FILE__,__LINE__);
			break;
		}

			
		for(unsigned int k=0; k<data.size(); k+=3){
			int value=data[k]*factor;
			if(value<0)value=0;
			if(value>255)value=255;
			colors[k+0 + i*data.size()*3] = value;
			colors[k+1 + i*data.size()*3 ] = value;
			colors[k+2 + i*data.size()*3 ] = value;
		}

		for(unsigned int k=0; k<data.size(); k+=3){
			vertecies[k+0 + i*data.size()*3] = k*currentScale;
			vertecies[k+1 + i*data.size()*3] = 0;
			vertecies[k+2 + i*data.size()*3] = 0; 
		}
		glDeleteLists(wallDist[i],1);
		wallDist[i] = glGenLists( 1 );
		glNewList( wallDist[i], GL_COMPILE );
		glEndList();

		glDeleteLists(bearingList[i], 1);
		bearingList[i] = glGenLists( 1 );
		glNewList( bearingList[i], GL_COMPILE );
			glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_VERTEX_ARRAY);
			glColorPointer(3,GL_UNSIGNED_BYTE,0,(void*)colors);
			glVertexPointer(3,GL_FLOAT,0,(void*)vertecies);
			glPushMatrix();
				double rotate = (double)i/6399.0*360.0;
				glRotated( rotate , 0.0, 0.0, 1.0);
				//if(i == 300){
			//		printf("Making for bearing: %i Color: %i, value: %f index: %i\n ",i, colors[300+i*data.size()],vertecies[300+i*data.size()],indices[300+i*data.size()]);
				if(i*data.size() > data.size()*3*maximumBearings){
					fprintf(stderr,"CRITIACAL ERROR\n");
				}
				//glPolygonOffset(i,1e-10);
				volatile GLuint *baseIndex = &indices[i*(data.size())];
				//glDrawElements(GL_LINES, data.size(), GL_UNSIGNED_INT, (void*)baseIndex);
				//glDrawElements(GL_LINES, data.size(), GL_UNSIGNED_INT, (void*)baseIndex);
				//glDrawElements(GL_LINE_STRIP, data.size(), GL_UNSIGNED_INT, (void*)baseIndex);
				glDrawElements(GL_POINTS, data.size(), GL_UNSIGNED_INT, (void*)baseIndex);
				//
/*
				glBegin(GL_LINE_STRIP);
				for(unsigned int k=0; k<data.size(); k++){
					double value=colors[k + i*data.size()];  //data[k]*factor;
					if(value<0)value=0;
					if(value>255)value=255;
					QColor color(value,value,value);
					qglColor(color);
					double pos = vertecies[k + i*data.size()];
					glVertex3f(k*currentScale,0,0);
				}
				glEnd();
				*/

				//}
				checkGL((const char*)"after Draw");
			glPopMatrix();
			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY); 
		glEndList();
	}



/*

		for(unsigned int k=0; k<data.size(); k++){
			double value=data[k]*factor;
			if(value<0)value=0;
			if(value>255)value=255;
			QColor color(value,value,value);
			qglColor(color);
			glVertex3f(k*currentScale,0,0);
		}
		glEnd();
		*/


#if 0
		if(false){ //Paint the overlay
			glBegin(GL_LINE_STRIP);
			int p_max = 0;
			int point;
			for(unsigned int k=medium_ableitung; k<data.size()-medium_ableitung; k++){
				int prevalues =0;
				int aftervalues=0;
				for(int p=1;p<medium_ableitung;p++){
					
					prevalues +=	max((data[k-p] - data[(int)max((k/2-p),0)]),0);
					aftervalues +=	max((data[k+p] - data[(k/2+p)]),0);
				}
				double value= (((prevalues - aftervalues)*data[k])) *factor;
				if(value<0)value=0;
				if(value>255)value=255.0;

				if(value > p_max && k >10){
					p_max=value;
					point = k*currentScale;
				}
				QColor color(value,value,value);
				qglColor(color);
				
				glVertex3f(k*currentScale,0,0);
				/*
				if(overlay_border >0){
					if(value >= overlay_border)
						glVertex3f(k*currentScale,0,0);
				}else{
					if(value < overlay_border)
						glVertex3f(k*currentScale,0,0);

				}
				*/
			}
			glEnd();

			glBegin(GL_LINE_STRIP);//;GL_POINTS);
			QColor color("red");
			qglColor(color);
			glVertex3f(0,0,0);
			glVertex3f(point,0,0);
			glEnd();
		}
#endif
//	}
		
	lastBearing = bearing;
//	mutex.unlock();
}

void SonarViewGL::paintSonar(){
}


void SonarViewGL::repaintFunc(){
   update();
   updateGL(); //TODO not calling this?

}

SonarViewGL::~SonarViewGL()
{
    //glDeleteLists(object, 1);
}

QSize SonarViewGL::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize SonarViewGL::sizeHint() const
{
    return QSize(400, 400);
}

void SonarViewGL::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        //updateGL();
    }
}

void SonarViewGL::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        //updateGL();
    }
}

void SonarViewGL::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        //updateGL();
    }
}

void SonarViewGL::setxPosition(int value)
{
       xShift = value;
       //printf("%d\n",xShift);
       emit xPositionChanged(value);
       //updateGL();
}
void SonarViewGL::setyPosition(int value)
{
       yShift = value;
       emit yPositionChanged(value);
       //updateGL();
}
void SonarViewGL::setzPosition(int value)
{
       zShift = value;
       emit zPositionChanged(value);
       //updateGL();
}

void SonarViewGL::initializeGL()
{
  //qglClearColor(QColor(0.1,0.1,0.0));
  //qglClearColor(purple.light());
  //qglClearColor(white);
    qglClearColor(QColor(0.0,0.0,0.0));
    glShadeModel(GL_FLAT);
    
    glEnable(GL_DEPTH_TEST);
    //glDisable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glDepthFunc(GL_ALWAYS);
    //glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);
//	glClearDepth(0.0);
    //glDepthFunc(GL_GEQUAL);
/*
    GLfloat fogColor[4] = {0.5,0.5,0.5,1.0};

    glFogi (GL_FOG_MODE, GL_LINEAR); //set the fog mode to GL_EXP2
    glFogfv (GL_FOG_COLOR, fogColor ); //set the fog color to our color chosen above
    glFogf (GL_FOG_DENSITY, 0.5); //set the density to the value above
    glHint (GL_FOG_HINT, GL_NICEST); // set the fog to look the nicest, may slow down on older cards

    glFogf(GL_FOG_START, 1.0f);             // Fog Start Depth
    glFogf(GL_FOG_END, 100.0f);               // Fog End Depth
    glDisable(GL_FOG);                   // Enables GL_FOG

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
*/
    
   groundPlane = glGenLists(1);
   glNewList(groundPlane, GL_COMPILE);

       #if 1 
       // draw the grid
       glDisable( GL_LIGHTING );
       glDisable( GL_COLOR_MATERIAL ) ;

       float size = 50.0f;
       float interval = 1.0f;


   glColor4f(0.5f,0.5f,1.0f,0.5f);
   glBegin(GL_LINES);
   float x = - size*0.5f;
   while( x <= size*0.5f ) {
       glVertex3f(-size/2.0f, x, 0.01f);
       glVertex3f(size/2.0f, x, 0.01f);
       glVertex3f(x, -size/2.0f, 0.01f);
       glVertex3f(x, size/2.0f, 0.01f);
       x += interval;
   }
   glEnd();

   // draw concentric circles
   float r;
   for(r=0;r<size/2;r+=interval) {
       glBegin(GL_LINES);
       for(x=0;x<(2*3.14152);x+=(2*3.14152)/(r*100)) {
           glVertex3f(cos(x)*r, sin(x)*r, 0.01f);
       }
       glEnd();
   }
   glEndList();
       #endif
}

void SonarViewGL::drawEllipse(float xradius, float yradius)
{
	glBegin(GL_LINE_LOOP);
	for(int i=0; i < 360; i++)
	{
		glVertex2f(cos(i/180.0*M_PI)*xradius,sin(i/180.0*M_PI)*yradius);
	} 
	glEnd();
}



void SonarViewGL::paintGL(){
//	printf("PaintGL\n");
     //set gui
//     mutex.lock();
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glLoadIdentity();
     glTranslated(0, 0, -zoom);
     
	//glMultMatrixf(camera);
     //set movments
     glTranslated(double(xShift)/100.0+pos[0], double(yShift)/100.0+pos[1], double(zShift)/100.0);
     glRotated(orientation, 1.0, 0.0, 0.0);
     glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
     glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
     glRotated(zRot / 16.0, 0.0, 0.0, 1.0);

     
     QColor color("yellow");
     qglColor(color);
     drawEllipse(sigmaPos[0],sigmaPos[1]);

    //gllDist
   // wlCallList(point_list);
    //glCallList(groundPlane);
     //Wegen Nebenläufigkeit die Punkte im GUI Thread zeichnen.
     //if(paintGroundTrue)
//	  if(glIsList(groundTrue))
//		glCallList(groundTrue);
	   //  else
	     //	printf("Error gorund true is not an list\n");
     if(glIsList(avalon))
	     glCallList(avalon);

     

     //for(unsigned int i=0;i<colorList.size();i++){
     //	glCallList(colorList[i].first);
     //}
//	 if(paintWall)
     for(int i=0;i<maximumBearings;i++){
     	if(glIsList(wallDist[i])){
			glCallList(wallDist[i]);
	 	}
     }
     for(int i=0;i<maximumBearings;i++){
     	if(glIsList(bearingList[i])){
		 glCallList(bearingList[i]);
		}
     }
    
  //   mutex.unlock();
  //printf("Paint gl end\n");
 }


void SonarViewGL::setWallDist(int bearing, int dist, int dist2){
	//printf("Set Wall Dist\n");	
//	mutex.lock();
	
	int begin,end;
	if(lastWallBearing-bearing>0 || lastWallBearing-bearing < -3000){
		begin = lastWallBearing;
		end = lastWallBearing-bearing;
	}else{
		begin = bearing;
		end = bearing-lastWallBearing;
	}
//	printf("Maximum distance: %f\n",currentScale*data.size());
	for(int j=0;j<end-2;j++){
		int i = (begin+j)%6400;
		if(i < 0 || i > 6399){
			fprintf(stderr,"Error in glwidet, please check value make no sense, prevent crash, not displaying data for index %i (%s:%i)\n",i,__FILE__,__LINE__);
			break;
		}
		//glDeleteLists(wallDist[i], 1);
	}
	lastWallBearing = bearing;

	glDeleteLists(wallDist[bearing], 1);
	//generate new one
	//checkGL("delete-list");
	wallDist[bearing] = glGenLists( 1 );
	//checkGL("generate-list");
	glNewList( wallDist[bearing], GL_COMPILE );
	//checkGL("new-list");
	//checkGL("push-Matrix");
	glPushMatrix();
	double rotate = (double)bearing/6399.0*360.0;
	glRotated( rotate, 0.0, 0.0, 1.0);
	glPointSize(2.0);
	glBegin(GL_POINTS);
	QColor color("yellow");
	qglColor(color);
	glVertex3f(dist*currentScale,0,-0.1);
	qglColor(QColor("green"));
	glVertex3f(dist2*currentScale,0,-0.1);
	glEnd();
	glPopMatrix();
	glEndList();
//	mutex.unlock(); 
}


 void SonarViewGL::resizeGL(int width, int height)
 {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 45.0, (float)width/(float)height, 1.0, 1000.0 );
    glMatrixMode(GL_MODELVIEW);
 }

 void SonarViewGL::mousePressEvent(QMouseEvent *event)
 {
     lastPos = event->pos();
     lastPosTrans = event->pos();
     repaintFunc();
 }

 void SonarViewGL::mouseMoveEvent(QMouseEvent *event)
 {
     //int dx = event->x() - lastPos.x();
     //int dy = event->y() - lastPos.y();

     int dxt = event->x() - lastPosTrans.x();
     int dyt = event->y() - lastPosTrans.y();

     if (event->buttons() & Qt::LeftButton) {
         //setXRotation(xRot + 8 * dy);
         //setZRotation(zRot + 8 * dx);
		setxPosition(xShift + dxt);
		setyPosition(yShift - dyt);
     }else if (event->buttons() & Qt::RightButton) {
    		zoom += (dyt+dxt)*0.05;
	        zoom = std::max<float>( std::min<float>( zoom, zoom_max ), zoom_min);
     }

     lastPos = event->pos();
     lastPosTrans = event->pos();

//     printf("Rot: %i,%i,%i, Pos, (%i,%i,%i), zoom: %f\n",xRot,yRot,zRot,xShift,yShift,zShift,zoom);

     repaintFunc();
 }

 //Werte für die Anzeige setzen.
 void SonarViewGL::setPosition(int xRot,int yRot, int zRot, int xMove, int yMove, int zMove, float zoom, int width, int height){
	 resize(width,height);
	 this->xRot = xRot;
	 this->yRot = yRot;
	 this->zRot = zRot;
	 this->xShift = xMove;
	 this->yShift = yMove;
	 this->zShift = zMove;
	 this->zoom = zoom;
	 //camera[2] = -zoom;

 }


void SonarViewGL::keyPressEvent ( QKeyEvent * event ){
	switch (event->key()) {
		case Qt::Key_W:
			paintWall=!paintWall;
		case Qt::Key_G:
			paintGroundTrue=!paintGroundTrue;
			break;
		case Qt::Key_Q:
			//Exit or do something
			break;
		case Qt::Key_I:
			factor+=0.02;
			printf("Gain %f\n",factor);
			//Exit or do something
			break;
		case Qt::Key_D:
			factor-=0.02;
			printf("Gain %f\n",factor);
			//Exit or do something
			break;
		case Qt::Key_O:
			overlay_border+=1;
			printf("Overlay border: %i\n",overlay_border);
			break;
		case Qt::Key_L:
			overlay_border-=1;
			printf("Overlay border: %i\n",overlay_border);
			break;
		case Qt::Key_A:
			medium_ableitung+=1;
			printf("Ableitungglaettung: %i\n",medium_ableitung);
			break;
		case Qt::Key_Y:
			medium_ableitung-=1;
			printf("Ableitungglaettung: %i\n",medium_ableitung);
			break;
		default:
			return;
	}
     repaintFunc();
 }

 void SonarViewGL::wheelEvent(QWheelEvent *event)
 {
    zoom -= (2.0*(float)event->delta()/120.0)*5;
    zoom = std::max<float>( std::min<float>( zoom, zoom_max ), zoom_min);
    event->accept();
   // updateGL();
     repaintFunc();
}

void SonarViewGL::setGroundTrue(double sizeX,double sizeY,double posX,double posY,double rotate){
				sizeX/=2.0;
				sizeY/=2.0;
				glDeleteLists(groundTrue, 1);
				groundTrue = glGenLists( 1 );
				glNewList( groundTrue, GL_COMPILE );
     				//checkGL("after list create");
				glPushMatrix();
				qglColor(Qt::red);
     			glTranslated(-posX, -posY, 0);
				glRotated( rotate, 0.0, 0.0, 1.0);
				glBegin(GL_LINE_LOOP);
				glVertex3d(-sizeX,sizeY,0);
				glVertex3d(sizeX,sizeY,0);
				glVertex3d(sizeX,-sizeY,0);
				glVertex3d(-sizeX,-sizeY,0);
				glEnd();
				glPopMatrix();
				glEndList();
     				//checkGL("end of set ground true:");
}

void SonarViewGL::createAvalon(){
				glDeleteLists(avalon, 1);
				avalon= glGenLists( 1 );
				glNewList( avalon, GL_COMPILE );
     				//checkGL("after list create");
				glPushMatrix();
				qglColor(Qt::cyan);
     				//glTranslated(posX, posY, 0);
				//glRotated( rotate, 0.0, 0.0, 1.0);
				glBegin(GL_LINE_LOOP);
				glVertex3d(-1,0.2,0);
				glVertex3d(1,0.2,0);
				glVertex3d(1,-0.2,0);
				glVertex3d(-1,-0.2,0);
				glEnd();
				glPopMatrix();
				glEndList();
     				//checkGL("end of set ground true:");
}


 void SonarViewGL::normalizeAngle(int *angle)
 {
     while (*angle < 0)
         *angle += 360 * 16;
     while (*angle > 360 * 16)
         *angle -= 360 * 16;
 }


void SonarViewGL::checkGL(const char* msg){
	GLenum err = glGetError();
	switch(err){
	case GL_NO_ERROR:
		break;
	case GL_INVALID_ENUM:
		printf("%s: Invalid enum\n",msg);
		break;
	case GL_INVALID_VALUE:
		printf("%s: invalid value\n",msg);
		break;
	case GL_INVALID_OPERATION:
		printf("%s: invalid operation\n",msg);
		break;
	case GL_STACK_OVERFLOW:
		printf("%s: stack overflow\n",msg);
		break;
	case GL_STACK_UNDERFLOW:
		printf("%s: stack underflow\n",msg);
		break;
	case GL_OUT_OF_MEMORY:
		printf("%s: OpenGL Put of memoty\n",msg);
		break;
	case GL_TABLE_TOO_LARGE:
		printf("%s: GL Table too large\n",msg);
		break;
	}
}

void SonarViewGL::setOrientation(const double orientation){
	this->orientation = orientation;
}
     
void SonarViewGL::setPosition(const double posX, const double posY, const double sigmaX, const double sigmaY){
	pos[0] = posX;
	pos[1] = posY;
	sigmaPos[0] = sigmaX;
	sigmaPos[1] = sigmaY;
     repaintFunc();
}


void SonarViewGL::setZoomMin(float value)
{
	zoom_min = value;
}

void SonarViewGL::setZoomMax(float value)
{
	zoom_max = value;
}
