#ifndef UNREAL_HDRI_COMPONENT_H_
#define UNREAL_HDRI_COMPONENT_H_

#include <unreal_component.h>
#include <octoon/game_object.h>
#include <octoon/hal/graphics_texture.h>

#include "../unreal_component.h"
#include "../module/resource_module.h"

namespace unreal
{
	class HDRiComponent final : public UnrealComponent<ResourceModule>
	{
	public:
		HDRiComponent() noexcept;
		~HDRiComponent() noexcept;

		nlohmann::json importPackage(std::string_view path) noexcept(false);
		nlohmann::json getPackage(std::string_view uuid) noexcept;
		bool removePackage(std::string_view uuid) noexcept;

		void save() noexcept(false);

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(HDRiComponent);
		}

	private:
		void initPackageIndices() noexcept(false);

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	private:
		std::map<std::string, nlohmann::json> packageList_;
	};
}

#endif