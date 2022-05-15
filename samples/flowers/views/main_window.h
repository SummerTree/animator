#ifndef FLOWER_MAIN_WINDOW_H
#define FLOWER_MAIN_WINDOW_H

#include "view_dock.h"
#include "tool_dock.h"
#include "main_light_dock.h"
#include "environment_dock.h"
#include "toplevel_bar.h"
#include "setting_window.h"
#include "color_dialog.h"
#include "flower_profile.h"
#include "splash_screen.h"
#include "light_window.h"
#include "login_window.h"
#include "info_window.h"
#include "record_dock.h"
#include "material_dock.h"

namespace flower
{
	class MainWindow final : public QFrame
	{
		Q_OBJECT
	public:
		MainWindow(SplashScreen* splash) noexcept(false);
		~MainWindow();

	private Q_SLOTS:

		void onUpdateSignal() noexcept;

	private:
		bool open(int w, int h) noexcept;

	private:
		bool init_flag;
		SplashScreen* splash_;

		octoon::GameAppPtr gameApp_;
		octoon::GameObjectPtr behaviour_;

		std::unique_ptr<QTimer> timer;
		std::shared_ptr<flower::FlowerProfile> profile_;
		std::shared_ptr<SplashListener> listener_;

		std::unique_ptr<ToolDock> toolBar_;
		std::unique_ptr<ToplevelBar> toplevelDock_;
		std::unique_ptr<ViewDock> viewDock_;
		std::unique_ptr<SettingWindow> settingWindow_;
		std::unique_ptr<LightWindow> lightWindow_;
		std::unique_ptr<MainLightDock> sunWindow_;
		std::unique_ptr<EnvironmentDock> environmentWindow_;
		std::unique_ptr<LoginWindow> loginWindow_;
		std::unique_ptr<InfoWindow> infoWindow_;
		std::unique_ptr<RecordDock> recordWindow_;
		std::unique_ptr<MaterialDock> materialWindow_;

		std::unique_ptr<QHBoxLayout> mainLayout_;
		std::unique_ptr<QVBoxLayout> contextLayout_;
	};
}

#endif