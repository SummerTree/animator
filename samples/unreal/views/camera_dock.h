#ifndef UNREAL_CAMERA_DOCK_H_
#define UNREAL_CAMERA_DOCK_H_

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
#include <qcheckbox.h>
#include "unreal_behaviour.h"
#include "../widgets/spoiler.h"
#include <octoon/game_object.h>

namespace unreal
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
		CameraDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept;
		~CameraDock() noexcept;

		void showEvent(QShowEvent* event) override;
		void closeEvent(QCloseEvent* event) override;

		bool eventFilter(QObject* watched, QEvent* event);

	private Q_SLOTS:
		void onFovChanged(double);
		void onFocalLengthChanged(double);
		void onApertureChanged(double);
		void onFocusDistanceChanged(double);
		void dofEvent(int state);
		void onUpdateTarget();
		void onLoadAnimation();
		void onUnloadAnimation();

	public:
		QLabel* dofInfoLabel_;
		QLabel* apertureLabel_;
		QLabel* fovLabel_;
		QLabel* focalLengthLabel_;
		QLabel* focusDistanceName_;
		QLabel* focusDistanceLabel_;
		QLabel* dofLabel_;

		QCheckBox* dofButton_;
		QToolButton* focusTargetButton_;
		QDoubleSpinBox* fovSpinbox_;
		QDoubleSpinBox* apertureSpinbox_;
		QDoubleSpinBox* focalLengthSpinbox_;
		QDoubleSpinBox* focusDistanceSpinbox_;

		QToolButton* loadButton_;
		QToolButton* unloadButton_;

		QWidget* mainWidget_;
		QVBoxLayout* mainLayout_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<UnrealProfile> profile_;
	};
}

#endif