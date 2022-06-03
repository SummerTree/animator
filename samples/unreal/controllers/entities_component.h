#ifndef UNREAL_ENTITIES_COMPONENT_H_
#define UNREAL_ENTITIES_COMPONENT_H_

#include "../unreal_component.h"
#include "../module/entities_module.h"

namespace unreal
{
	class EntitiesComponent final : public UnrealComponent<EntitiesModule>
	{
	public:
		EntitiesComponent() noexcept;
		virtual ~EntitiesComponent() noexcept;

		void importAbc(std::string_view path) noexcept(false);
		octoon::GameObjectPtr importModel(std::string_view path) noexcept;

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