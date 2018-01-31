#ifndef _MULTI_VIEW_WIDGET_H
#define _MULTI_VIEW_WIDGET_H

#include <widget_collection/generic_widgets/PaintWidget.h>
#include <QtDesigner/QDesignerExportWidget>


class QDESIGNER_WIDGET_EXPORT MultiWidget : public PaintWidget 
{
	Q_OBJECT
  Q_CLASSINFO("Author", "Matthias Goldhoorn")
	Q_PROPERTY(bool hideWhenMinimized READ isHiddenWhenMinimized WRITE hideWhenMin USER false)
	Q_PROPERTY(QString minimizedLabel READ getMinimizedLabel WRITE setMinimizedLabel)

Q_SIGNALS:
	void activityChanged(bool);
public:
	MultiWidget(QWidget *parent=0):
		PaintWidget(parent),
		isActive(false),
		hideWhenMinimized(false),
		wasHidden(false)
	{};

public Q_SLOTS:

	void setActive(bool b){
		isActive=b;
		Q_EMIT activityChanged(b);

		for(QObjectList::const_iterator it = children().begin(); it != children().end(); it++){
			MultiWidget *child = dynamic_cast<MultiWidget*>(*it);
			if(child > 0){
				child->setActive(isActive);
			}
		}

		if(!isActive){
			if(hideWhenMinimized){
				wasHidden=isHidden();
				hide();
			}
		}else{
			//if(!wasHidden){
				show();
			//}
		}
	}

	QString getMinimizedLabel(){return minimizedLabel;}
	void setMinimizedLabel(QString label){minimizedLabel = label;}


    /**
     * Returns whether this is a container widget
     * @return will always return true
     */
    
		void childEvent(QChildEvent* event){
			QWidget::childEvent(event);
		}

    bool event(QEvent* event){
			return QWidget::event(event);
		}

		bool isHiddenWhenMinimized() const{
			return hideWhenMinimized;
		}

		void hideWhenMin(bool b){
			hideWhenMinimized = b;
		}

protected:
	/** True if this Widget is not Minimized */
	bool isActive;
  bool hideWhenMinimized;
	bool wasHidden;
	QString minimizedLabel;
};


#endif
