#include "udockwidget.h"

namespace unreal
{
	UDockWidget::UDockWidget(QWidget* parent)
		: QDockWidget(parent)
	{
        this->installEventFilter(this);
	}
	UDockWidget::~UDockWidget()
	{
	}
}
