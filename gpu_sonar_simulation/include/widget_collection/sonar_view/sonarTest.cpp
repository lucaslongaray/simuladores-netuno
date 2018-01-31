#include "SonarView.h"

#include <QApplication>

int main(int argc, char **argv){
	QApplication qapp(argc,argv);
	SonarView sw;
	sw.show();
	char data[600];
	for(double i=0;i<2.0*M_PI; i+=(2.0*M_PI)/6399.0){
		for(int j=0;j<600;j++){
			data[j]=255;
		}
		sw.setSonarScan(data,600,i/6399.0*2.0*M_PI,0.000094,false);
	}
	qapp.exec();
}
