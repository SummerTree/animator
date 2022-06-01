#include "ucombobox.h"
#include <QCursor>
namespace unreal
{
	UComboBox::UComboBox(QWidget* parent)
		: QComboBox(parent)
	{
		setCursor(Qt::PointingHandCursor);
	}
	UComboBox::~UComboBox()
	{
	}
}
