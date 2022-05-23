#ifndef UNREAL_MOTION_DOCK_H_
#define UNREAL_MOTION_DOCK_H_

#include "spoiler.h"
#include "unreal_behaviour.h"
#include <QComboBox>
#include <octoon/game_object.h>
#include <optional>
#include <qboxlayout.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qdialog.h>
#include <qdockwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qtoolbutton.h>

namespace unreal
{
	class MotionDock final : public QDockWidget
	{
		Q_OBJECT
	  public:
		MotionDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept;
		~MotionDock() noexcept;

		void showEvent(QShowEvent* event) override;
		void paintEvent(QPaintEvent* e) noexcept override;
		void resizeEvent(QResizeEvent* e) noexcept override;
		void closeEvent(QCloseEvent* event) override;

	  private Q_SLOTS:
		void recordEvent(bool);


	  public:
		QVBoxLayout* mainLayout_;

		QWidget* mainWidget_;

		Spoiler* markSpoiler_;
		Spoiler* videoSpoiler_;
		QScrollArea* contentWidgetArea_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<UnrealProfile> profile_;
	};
}

#endif