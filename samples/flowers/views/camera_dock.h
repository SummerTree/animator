#ifndef FLOWER_CAMERA_DOCK_H_
#define FLOWER_CAMERA_DOCK_H_

#include <qdockwidget.h>
#include <qdialog.h>
#include <qboxlayout.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <optional>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtimer.h>
#include "flower_behaviour.h"
#include "spoiler.h"
#include <octoon/game_object.h>

namespace flower
{
	class FocalTargetWindow final : public QToolButton
	{
		Q_OBJECT
	public:
		FocalTargetWindow() noexcept;
		~FocalTargetWindow() noexcept;

		void mouseMoveEvent(QMouseEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;

	Q_SIGNALS:
		void mouseMoveSignal();

	private:
		QPoint startPos;
	};
	
	class CameraDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		CameraDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept;
		~CameraDock() noexcept;

		void showEvent(QShowEvent* event) override;

	private Q_SLOTS:
		void onApertureChanged(double);
		void onFocalDistanceChanged(double);
		void updateTarget();

	public:
		QLabel* dofInfoLabel_;
		QLabel* apertureLabel_;
		QLabel* focalDistanceName_;
		QLabel* focalDistanceLabel_;

		QToolButton* focalTargetButton_;
		QDoubleSpinBox* apertureSpinbox_;
		QDoubleSpinBox* focalDistanceSpinbox_;

		QWidget* mainWidget_;
		QVBoxLayout* mainLayout_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<FlowerProfile> profile_;
	};
}

#endif