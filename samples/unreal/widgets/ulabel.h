#ifndef UNREAL_ULABEL_H_
#define UNREAL_ULABEL_H_

#include <QLabel>

namespace unreal
{
	class ULabel : public QLabel
	{
		Q_OBJECT
	public:
		explicit ULabel(QWidget* parent = 0);
		~ULabel();
	};
}

#endif