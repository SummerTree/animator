#ifndef FLOWER_HDRI_COMPONENT_H_
#define FLOWER_HDRI_COMPONENT_H_

#include <flower_component.h>
#include <octoon/game_object.h>

#include "../flower_component.h"
#include "../module/resource_module.h"

namespace flower
{
	class HDRiComponent final : public RabbitComponent<ResourceModule>
	{
	public:
		HDRiComponent() noexcept;
		~HDRiComponent() noexcept;

		void importHDRi(std::string_view path) noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(HDRiComponent);
		}

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	private:

		std::map<std::string, nlohmann::json, std::less<>> hdriList_;
	};
}

#endif