#include "upushbutton.h"

#include <QCursor>
namespace unreal
{
	UPushButton::UPushButton(QWidget* parent)
		: QPushButton(parent)
	{
		setCursor(Qt::PointingHandCursor);
        this->installEventFilter(this);
	}
	UPushButton::~UPushButton()
	{
	}
    bool
	UPushButton::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() == QEvent::LanguageChange)
			retranslate();
		return QPushButton::eventFilter(watched, event);
	}
}
