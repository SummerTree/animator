#ifndef UNREAL_USPINLINE_H_
#define UNREAL_USPINLINE_H_

#include <QSpinBox>
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>

#include "uspinbox.h"
#include "ulabel.h"

namespace unreal
{
	class USpinLine : public QWidget
	{
		Q_OBJECT
	public:
		explicit USpinLine(QWidget* parent = 0);
		~USpinLine();

        void setupUI();

		void setValue(int value);

        static USpinLine* create(QWidget* parent, QString label, int min = 0, int max = 100, int step = 1, int value = 0);
	Q_SIGNALS:
		void valueChanged(int value);

	public Q_SLOTS:
		void onDisable();
		void onEnable();
    public:
        USpinBox* spinbox_;
        ULabel* label_;
        QHBoxLayout* layout_;
	};
}

#endif