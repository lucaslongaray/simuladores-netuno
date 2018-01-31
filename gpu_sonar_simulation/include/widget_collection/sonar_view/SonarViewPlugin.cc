#include "SonarViewPlugin.h"
#include "SonarView.h"


 SonarViewPlugin::SonarViewPlugin(QObject *parent)
     : QObject(parent)
 {
     initialized = false;
 }

 SonarViewPlugin::~SonarViewPlugin()
 {
 }

 void SonarViewPlugin::initialize(QDesignerFormEditorInterface *formEditor)
 {
     if (initialized)
         return;
     initialized = true;
 }

 bool SonarViewPlugin::isInitialized() const
 {
     return initialized;
 }

 QWidget *SonarViewPlugin::createWidget(QWidget *parent)
 {
     return new SonarView(parent);
 }

 QString SonarViewPlugin::name() const
 {
     return "SonarView";
 }

 QString SonarViewPlugin::group() const
 {
     return "Rock-Robotics";
 }

 QIcon SonarViewPlugin::icon() const
 {
     return QIcon(":/sonar_view/icon.png");
 }

 QString SonarViewPlugin::toolTip() const
 {
     return "Widget for displaying sonar data";
 }

 QString SonarViewPlugin::whatsThis() const
 {
     return "widget for displaying sonar data";
 }

 bool SonarViewPlugin::isContainer() const
 {
     return false;
 }

 QString SonarViewPlugin::domXml() const
 {
     return "<widget class=\"SonarView\" name=\"sonarview\">\n"
            " <property name=\"geometry\">\n"
            "  <rect>\n"
            "   <x>0</x>\n"
            "   <y>0</y>\n"
            "   <width>320</width>\n"
            "   <height>240</height>\n"
            "  </rect>\n"
            " </property>\n"
            " <property name=\"toolTip\" >\n"
            "  <string>ImageView</string>\n"
            " </property>\n"
            " <property name=\"whatsThis\" >\n"
            "  <string>Widget for displaying frames.</string>\n"
            " </property>\n"
            "</widget>\n";
 }

 QString SonarViewPlugin::includeFile() const
 {
     return "image_view_widget/sonar_view/SonarView.h";
 }

