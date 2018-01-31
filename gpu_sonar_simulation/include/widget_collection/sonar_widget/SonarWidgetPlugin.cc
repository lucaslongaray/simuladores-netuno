#include "SonarWidgetPlugin.h"
#include "SonarWidget.h"

SonarWidgetPlugin::SonarWidgetPlugin(QObject *parent)
    : QObject(parent)
{
    initialized = false;
}

SonarWidgetPlugin::~SonarWidgetPlugin()
{
}

bool SonarWidgetPlugin::isContainer() const
{
    return false;
}

bool SonarWidgetPlugin::isInitialized() const
{
    return initialized;
}

QIcon SonarWidgetPlugin::icon() const
{
    return QIcon("");
}

QString SonarWidgetPlugin::domXml() const
{
        return "<ui language=\"c++\">\n"
            " <widget class=\"SonarWidget\" name=\"SonarWidget\">\n"
            "  <property name=\"geometry\">\n"
            "   <rect>\n"
            "    <x>0</x>\n"
            "    <y>0</y>\n"
            "     <width>300</width>\n"
            "     <height>120</height>\n"
            "   </rect>\n"
            "  </property>\n"
//            "  <property name=\"toolTip\" >\n"
//            "   <string>SonarWidget</string>\n"
//            "  </property>\n"
//            "  <property name=\"whatsThis\" >\n"
//            "   <string>SonarWidget</string>\n"
//            "  </property>\n"
            " </widget>\n"
            "</ui>\n";
}

QString SonarWidgetPlugin::group() const {
    return "Rock-Robotics";
}

QString SonarWidgetPlugin::includeFile() const {
    return "SonarWidget/SonarWidget.h";
}

QString SonarWidgetPlugin::name() const {
    return "SonarWidget";
}

QString SonarWidgetPlugin::toolTip() const {
    return whatsThis();
}

QString SonarWidgetPlugin::whatsThis() const
{
    return "";
}

QWidget* SonarWidgetPlugin::createWidget(QWidget *parent)
{
    return new SonarWidget(parent);
}

void SonarWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
     if (initialized)
         return;
     initialized = true;
}
