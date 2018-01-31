#include "SonarWidget.h"
#include "./SonarPlot.h"
#include <iostream>

SonarWidget::SonarWidget(QWidget *parent)
    : QWidget(parent)
{
    resize(1020,670);
    
    plot = new SonarPlot(this);
    plot->setGeometry (10,10,BASE_WIDTH,BASE_HEIGHT);
    connect(this,SIGNAL(rangeChanged(int)),plot,SLOT(rangeChanged(int)));
    connect(this,SIGNAL(sonarPaletteChanged(int)),plot,SLOT(sonarPaletteChanged(int)));
    
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, plot->palette().color(QPalette::Background));
    setAutoFillBackground(true);
    setPalette(Pal);
    
    createGainComponent();
    createRangeComponent();
    createPaletteComponent();

    show();
}

void SonarWidget::resizeEvent ( QResizeEvent * event )
{
  plot->setGeometry (10,10,width()-20,height()-70);
  lbGain->setGeometry(10,height()-70,50,20);
  slGain->setGeometry(70,height()-70,150,20);
  edGain->setGeometry(230,height()-70,50,20);
  lbRange->setGeometry(10,height()-40,50,20);
  slRange->setGeometry(70,height()-40,150,20);
  edRange->setGeometry(230,height()-40,50,20);
  lbPalette->setGeometry(width()-160,height()-40,50,20);
  comboPalette->setGeometry(width()-100,height()-40,80,20);
  QWidget::resizeEvent (event);
}

SonarWidget::~SonarWidget()
{
  if(plot){
    delete plot;
  }
}

void SonarWidget::setData(const base::samples::SonarScan scan)
{
    base::samples::Sonar sonar(scan);
    setData(sonar);
}

void SonarWidget::setData(const base::samples::Sonar sonar)
{
    plot->setData(sonar);
}

void SonarWidget::setGain(int value)
{
  slGain->setValue(value);
}

void SonarWidget::setRange(int value)
{
  slRange->setValue(value);
}

void SonarWidget::setSectorScan(bool continuous, base::Angle left, base::Angle right)
{
    plot->setSectorScan(continuous, left, right);
}

void SonarWidget::setMinRange(int value)
{
  slRange->setMinimum(value);
}

void SonarWidget::setMaxRange(int value)
{
  slRange->setMaximum(value);
}

void SonarWidget::setSonarPalette(int value)
{
  comboPalette->setCurrentIndex(value);
}

void SonarWidget::enableAutoRanging(bool value)
{
  slRange->setEnabled(!value);
}

void SonarWidget::onSlGainChanged(int value)
{
  QString str;
  str.setNum(value);
  edGain->setText(str + " %");
  gainChanged(value);
}

void SonarWidget::onSlRangeChanged(int value)
{
  QString str;
  str.setNum(value);
  edRange->setText(str + " m");
  rangeChanged(value);
}

void SonarWidget::onComboPaletteChanged(int value)
{
    sonarPaletteChanged(value);
}

void SonarWidget::createGainComponent() {
    lbGain = new QLabel(this);
    lbGain->setGeometry(10, height() - 70, 50, 20);
    QPalette Pal = lbGain->palette();
    Pal.setColor(QPalette::Foreground, Qt::white);
    lbGain->setPalette(Pal);
    lbGain->setText("Gain:");
    slGain = new QSlider(Qt::Horizontal, this);
    slGain->setGeometry(70, height() - 70, 150, 20);
    slGain->setMinimum(0);
    slGain->setMaximum(100);
    slGain->setValue(50);
    edGain = new QLineEdit(this);
    edGain->setGeometry(230, height() - 70, 50, 20);
    edGain->setAlignment(Qt::AlignRight);
    Pal = edGain->palette();
    Pal.setColor(QPalette::Base, plot->palette().color(QPalette::Background));
    Pal.setColor(QPalette::Text, Qt::white);
    setAutoFillBackground(true);
    edGain->setPalette(Pal);
    edGain->setReadOnly(true);
    edGain->setText("50 %");
    connect(slGain,SIGNAL(valueChanged(int)),this,SLOT(onSlGainChanged(int)));
}

void SonarWidget::createRangeComponent() {
    lbRange = new QLabel(this);
    lbRange->setGeometry(10,height()-40,50,20);
    QPalette Pal=lbRange->palette();
    Pal.setColor(QPalette::Foreground,Qt::white);
    lbRange->setPalette(Pal);
    lbRange->setText("Range:");
    slRange = new QSlider(Qt::Horizontal, this);
    slRange->setGeometry(70,height()-40,150,20);
    slRange->setMinimum(1);
    slRange->setMaximum(150);
    slRange->setValue(5);
    edRange = new QLineEdit(this);
    edRange->setGeometry(230,height()-40,50,20);
    edRange->setAlignment(Qt::AlignRight);
    Pal=edRange->palette();
    Pal.setColor(QPalette::Base, plot->palette().color(QPalette::Background));
    Pal.setColor(QPalette::Text,Qt::white);
    setAutoFillBackground(true);
    edRange->setPalette(Pal);
    edRange->setReadOnly(true);
    edRange->setText("5 m");
    connect(slRange,SIGNAL(valueChanged(int)),this,SLOT(onSlRangeChanged(int)));
}

void SonarWidget::createPaletteComponent() {
    lbPalette = new QLabel(this);
    lbPalette->setGeometry(width()-160, height() - 40, 50, 20);
    QPalette Pal=lbPalette->palette();
    Pal.setColor(QPalette::Foreground,Qt::white);
    lbPalette->setPalette(Pal);
    lbPalette->setText("Palette:");
    comboPalette = new QComboBox(this);
    comboPalette->setGeometry(width()-100,height()-40,80,20);
    Pal=comboPalette->palette();
    Pal.setColor(comboPalette->backgroundRole(), plot->palette().color(QPalette::Background));
    Pal.setColor(comboPalette->foregroundRole(),Qt::white);
    setAutoFillBackground(true);
    comboPalette->setPalette(Pal);
    comboPalette->insertItem(comboPalette->count()+1,"Jet");
    comboPalette->insertItem(comboPalette->count()+1,"Hot");
    comboPalette->insertItem(comboPalette->count()+1,"Gray");
    connect(comboPalette,SIGNAL(currentIndexChanged(int)),this,SLOT(onComboPaletteChanged(int)));
}
