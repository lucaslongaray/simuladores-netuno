#ifndef SONARWIDGET_H
#define SONARWIDGET_H

#include <QtGui>
#include <base-types/base/samples/SonarScan.hpp>
#include <base-types/base/samples/Sonar.hpp>

class SonarPlot;

class SonarWidget : public QWidget
{
    Q_OBJECT
protected:
    SonarPlot *plot;
    double scaleX;
    double scaleY;
    void resizeEvent ( QResizeEvent * event );
    QLabel *lbGain;
    QLabel *lbRange;
    QLabel *lbPalette;
    QLineEdit *edGain;
    QLineEdit *edRange;
      
public:
    SonarWidget(QWidget *parent = 0);
    virtual ~SonarWidget();
    void createGainComponent();
    void createRangeComponent();
    void createPaletteComponent();
    QSlider *slGain;
    QSlider *slRange;
    QComboBox *comboPalette;

public Q_SLOTS:
    void setData(const base::samples::SonarScan scan);
    void setData(const base::samples::Sonar sonar);
    void setGain(int);
    void setRange(int);
    void setMaxRange(int);
    void setMinRange(int);
    void setSonarPalette(int);
    void enableAutoRanging(bool);

    // only for scanning sonars
    void setSectorScan(bool continuous, base::Angle left, base::Angle right);

protected Q_SLOTS:
    void onSlGainChanged(int);
    void onSlRangeChanged(int);
    void onComboPaletteChanged(int);

Q_SIGNALS:
    void gainChanged(int);
    void rangeChanged(int);
    void sonarPaletteChanged(int);
    
};

#endif /* SONAR_WIDGET_H */
