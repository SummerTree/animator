#ifndef UNREAL_UCOMBOBOX_H_
#define UNREAL_UCOMBOBOX_H_

#include <QCombobox>

namespace unreal
{
	class UComboBox final : public QComboBox
	{
		Q_OBJECT
	public:
		explicit UComboBox(QWidget* parent = 0);
		~UComboBox();
	};
}

#endif