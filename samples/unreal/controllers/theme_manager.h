#ifndef UNREAL_THEME_MANAGER_H_
#define UNREAL_THEME_MANAGER_H_

#include <octoon/game_component.h>
#include <octoon/ui/imgui.h>

namespace unreal
{
	class ThemeManager final : public octoon::GameComponent
	{
		OctoonDeclareSubClass(ThemeManager, octoon::GameComponent)
	public:
		ThemeManager() noexcept;
		~ThemeManager() noexcept;

		octoon::GameComponentPtr clone() const noexcept override;

	private:
		void onActivate() noexcept override;
		void onDeactivate() noexcept override;

		void onGui() noexcept override;

	private:
		octoon::imgui::GuiStyle _style;
		octoon::imgui::GuiStyle _styleDefault;
	};
}

#endif
