#ifndef FLOWER_GRID_COMPONENT_H_
#define FLOWER_GRID_COMPONENT_H_

#include <flower_component.h>
#include <octoon/game_object.h>
#include <module/grid_module.h>

namespace flower
{
	class GridComponent final : public RabbitComponent<GridModule>
	{
	public:
		GridComponent() noexcept;
		~GridComponent() noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(GridComponent);
		}

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	private:
		octoon::GameObjectPtr gizmo_;
	};
}

#endif