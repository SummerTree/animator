#ifndef UNREAL_USPINBOX_H_
#define UNREAL_USPINBOX_H_

#include <QSpinBox>

namespace unreal
{
	class USpinBox : public QSpinBox
	{
		Q_OBJECT
	public:
		explicit USpinBox(QWidget* parent = 0);
		~USpinBox();
	};
}

#endif