#ifndef FLOWER_SELECTOR_COMPONENT_H_
#define FLOWER_SELECTOR_COMPONENT_H_

#include <flower_component.h>

#include <octoon/raycaster.h>
#include <octoon/game_object.h>
#include <octoon/material/material.h>

#include <module/selector_module.h>

#include <optional>

namespace flower
{
	class SelectorComponent final : public RabbitComponent<SelectorModule>
	{
	public:
		SelectorComponent() noexcept;
		~SelectorComponent() noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(SelectorComponent);
		}

	private:
		void handleMouseDown(const octoon::input::InputEvent& event) noexcept;
		void handleMouseMove(const octoon::input::InputEvent& event) noexcept;
		void handleMouseHover(const octoon::input::InputEvent& event) noexcept;

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

		void onUpdate() noexcept override;

		void onMouseDown(const octoon::input::InputEvent& event) noexcept;
		void onMouseMotion(const octoon::input::InputEvent& event) noexcept;
		void onMouseUp(const octoon::input::InputEvent& event) noexcept;

	private:
		std::optional<octoon::RaycastHit> intersectObjects(float x, float y) noexcept;

	private:
		octoon::GameObjectPtr gizmoHover_;
		octoon::GameObjectPtr gizmoSelected_;

		octoon::MaterialPtr gizmoHoverMtl_;
		octoon::MaterialPtr gizmoSelectedMtl_;
	};
}

#endif