
#ifndef SONARVIEWPLUGIN_H
#define SONARVIEWPLUGIN_H 

#include <QtGui/QtGui>
#include <QtDesigner/QDesignerCustomWidgetInterface>

class SonarViewPlugin : public QObject , public QDesignerCustomWidgetInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

 public:
   SonarViewPlugin(QObject *parent = 0);
   virtual ~SonarViewPlugin();

   bool isContainer() const;
   bool isInitialized() const;
   QIcon icon() const;
   QString domXml() const;
   QString group() const;
   QString includeFile() const;
   QString name() const;
   QString toolTip() const;
   QString whatsThis() const;
   QWidget *createWidget(QWidget *parent);
   void initialize(QDesignerFormEditorInterface *core);

 private:
   bool initialized; 
};
#endif
