#ifndef FLOWER_MAIN_LIGHT_WINDOW_H_
#define FLOWER_MAIN_LIGHT_WINDOW_H_

#include <qwidget.h>
#include <qlayout.h>
#include <qscrollarea.h>

#include "flower_profile.h"
#include "color_dialog.h"

namespace flower
{
	class MainLightDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		MainLightDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<flower::FlowerProfile>& profile);
		~MainLightDock();

		void repaint();

		virtual void showEvent(QShowEvent* event) override;
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void closeEvent(QCloseEvent* event) override;
		virtual void paintEvent(QPaintEvent* e) noexcept override;

	public Q_SLOTS:
		void currentColorChanged(QColor);
		void resetEvent();
		void intensitySliderEvent(int);
		void intensityEditEvent(double value);
		void sliderRotationXEvent(int);
		void editRotationXEvent(double value);
		void sliderRotationYEvent(int);
		void editRotationYEvent(double value);
		void sliderRotationZEvent(int);
		void editRotationZEvent(double value);

	private:
		QWidget* scrollWidget_;
		QScrollArea* scrollArea_;
		QVBoxLayout* scrollLayout_;
		ColorDialog* colorDialog_;
		QDoubleSpinBox* editIntensity_;
		QDoubleSpinBox* editRotationX_;
		QDoubleSpinBox* editRotationY_;
		QDoubleSpinBox* editRotationZ_;
		QLabel* labelIntensity_;
		QLabel* labelRotationX_;
		QLabel* labelRotationY_;
		QLabel* labelRotationZ_;
		QSlider* sliderIntensity_;
		QSlider* sliderRotationX_;
		QSlider* sliderRotationY_;
		QSlider* sliderRotationZ_;
		QHBoxLayout* layout_;
		QHBoxLayout* layoutIntensity_;
		QHBoxLayout* layoutRotationX_;
		QHBoxLayout* layoutRotationY_;
		QHBoxLayout* layoutRotationZ_;
		QVBoxLayout* mainLayout_;
		QToolButton* resetButton_;
		QWidget* mainWidget_;
		std::shared_ptr<flower::FlowerProfile> profile_;
	};
}

#endif