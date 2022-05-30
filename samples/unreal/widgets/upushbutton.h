#ifndef UNREAL_UPUSHBUTTON_H_
#define UNREAL_UPUSHBUTTON_H_

#include <QPushButton>
#include <QWidget>
#include <QEvent>
#include "utranslatable.h"

namespace unreal
{
	class UPushButton : public QPushButton, public UTranslatable
	{
		Q_OBJECT
	public:
		explicit UPushButton(QWidget* parent = 0);
		~UPushButton();
        
		bool eventFilter(QObject* watched, QEvent* event);
	};
}

#endif