#ifndef FLOWER_VIEW_DOCK_H_
#define FLOWER_VIEW_DOCK_H_

#include <qwidget.h>
#include <qtimer.h>
#include <qdrag.h>
#include <qmainwindow.h>

#include <octoon/octoon.h>
#include <octoon/game_listener.h>

#include "flower_profile.h"
#include "flower_behaviour.h"

#include "views/splash_screen.h"
#include "views/status_bar.h"
#include "views/toplevel_bar.h"
#include "views/view_dock.h"
#include "views/tool_dock.h"
#include "views/main_light_dock.h"
#include "views/environment_dock.h"
#include "views/material_dock.h"
#include "views/record_dock.h"

namespace flower
{
	class MainDock final : public QMainWindow
	{
		Q_OBJECT
	public:
		MainDock(SplashScreen* splash) noexcept;
		~MainDock() noexcept;

	private Q_SLOTS:
		void paintEvent(QPaintEvent* e) noexcept override;
		void resizeEvent(QResizeEvent* e) noexcept override;
		void dragEnterEvent(QDragEnterEvent* event) noexcept override;
		void dragMoveEvent(QDragMoveEvent* event) noexcept override;
		void dropEvent(QDropEvent* event) noexcept override;
		void showEvent(QShowEvent* event) noexcept override;

		void setupEvent() noexcept;
		void updateEvent() noexcept;

		bool eventFilter(QObject* watched, QEvent* event);

		void onSunSignal() noexcept;
		void onLightSignal() noexcept;
		void onRecordSignal() noexcept;
		void onEnvironmentSignal() noexcept;
		void onMaterialSignal() noexcept;

	private:
		void open(int w, int h) noexcept(false);

	private:
		bool init_flag;
		SplashScreen* splash_;

		QTimer timer;

		octoon::GameAppPtr gameApp_;
		octoon::GameObjectPtr behaviour_;

		std::shared_ptr<flower::FlowerProfile> profile_;
		std::shared_ptr<SplashListener> listener_;

		std::unique_ptr<StatusBar> statusBar_;
		std::unique_ptr<ToplevelBar> toplevelDock_;
		std::unique_ptr<ViewDock> viewDock_;
		std::unique_ptr<ToolDock> toolDock_;
		std::unique_ptr<RecordDock> recordDock_;
		std::unique_ptr<MainLightDock> mainLightDock_;
		std::unique_ptr<EnvironmentDock> environmentDock_;
		std::unique_ptr<MaterialDock> materialDock_;
	};
}

#endif