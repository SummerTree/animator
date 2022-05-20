#ifndef FLOWER_HDRI_COMPONENT_H_
#define FLOWER_HDRI_COMPONENT_H_

#include <flower_component.h>
#include <octoon/game_object.h>
#include <octoon/hal/graphics_texture.h>

#include "../flower_component.h"
#include "../module/resource_module.h"

namespace flower
{
	class HDRiComponent final : public RabbitComponent<ResourceModule>
	{
	public:
		HDRiComponent() noexcept;
		~HDRiComponent() noexcept;

		nlohmann::json importHDRi(std::string_view path) noexcept(false);
		nlohmann::json getPackage(std::string_view uuid) noexcept;

		void save() noexcept(false);

		const nlohmann::json& getIndexList() const noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(HDRiComponent);
		}

	private:
		void initResourceList() noexcept;

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	private:
		nlohmann::json indexList_;

		std::map<std::string, nlohmann::json> packageList_;
	};
}

#endif