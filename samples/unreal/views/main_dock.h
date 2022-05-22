#ifndef UNREAL_MAIN_DOCK_H_
#define UNREAL_MAIN_DOCK_H_

#include <qwidget.h>
#include <qtimer.h>
#include <qdrag.h>
#include <qmainwindow.h>

#include <octoon/octoon.h>
#include <octoon/game_listener.h>

#include "unreal_profile.h"
#include "unreal_behaviour.h"

#include "views/splash_screen.h"
#include "views/status_bar.h"
#include "views/toplevel_bar.h"
#include "views/view_dock.h"
#include "views/tool_dock.h"
#include "views/thumbnail_dock.h"
#include "views/light_dock.h"
#include "views/main_light_dock.h"
#include "views/environment_dock.h"
#include "views/material_dock.h"
#include "views/record_dock.h"
#include "views/camera_dock.h"

namespace unreal
{
	class MainDock final : public QMainWindow
	{
		Q_OBJECT
	public:
		MainDock(SplashScreen* splash) noexcept;
		~MainDock() noexcept;

		void restoreLayout() noexcept;
		void saveLayout() noexcept;

	private Q_SLOTS:
		void dragEnterEvent(QDragEnterEvent* event) noexcept override;
		void dragMoveEvent(QDragMoveEvent* event) noexcept override;
		void dropEvent(QDropEvent* event) noexcept override;
		void showEvent(QShowEvent* event) noexcept override;

		bool eventFilter(QObject* watched, QEvent* event);

	private:
		void onSunSignal() noexcept;
		void onLightSignal() noexcept;
		void onRecordSignal() noexcept;
		void onEnvironmentSignal() noexcept;
		void onMaterialSignal() noexcept;
		void onCameraSignal() noexcept;

		void open() noexcept(false);
		void update() noexcept;

		QDockWidget* visableDock() noexcept;

	private:
		bool init_flag;
		SplashScreen* splash_;

		QTimer timer;

		octoon::GameAppPtr gameApp_;
		octoon::GameObjectPtr behaviour_;

		std::shared_ptr<UnrealProfile> profile_;
		std::shared_ptr<SplashListener> listener_;

		std::unique_ptr<StatusBar> statusBar_;
		std::unique_ptr<ToplevelBar> toplevelDock_;
		std::unique_ptr<ViewDock> viewDock_;
		std::unique_ptr<ToolDock> toolDock_;
		std::unique_ptr<ThumbnailDock> thumbnailDock_;
		std::unique_ptr<RecordDock> recordDock_;
		std::unique_ptr<LightDock> lightDock_;
		std::unique_ptr<MainLightDock> mainLightDock_;
		std::unique_ptr<EnvironmentDock> environmentDock_;
		std::unique_ptr<MaterialDock> materialDock_;
		std::unique_ptr<CameraDock> cameraDock_;
	};
}

#endif