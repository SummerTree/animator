#ifndef UNREAL_ENTITIES_COMPONENT_H_
#define UNREAL_ENTITIES_COMPONENT_H_

#include "../utils/pmm.h"
#include "../unreal_component.h"
#include "../module/sound_module.h"
#include "../module/entities_module.h"

#include <optional>
#include <octoon/octoon.h>

namespace unreal
{
	class EntitiesComponent final : public UnrealComponent<EntitiesModule>
	{
	public:
		EntitiesComponent() noexcept;
		virtual ~EntitiesComponent() noexcept;

		void importAbc(std::string_view path) noexcept(false);
		void importAss(std::string_view path) noexcept(false);
		bool importModel(std::string_view path) noexcept;

		bool exportModel(std::string_view path) noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(EntitiesComponent);
		}

	private:
		void onInit() noexcept override;

		void onEnable() noexcept override;
		void onDisable() noexcept override;
	};
}

#endif