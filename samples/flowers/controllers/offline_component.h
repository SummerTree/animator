#ifndef UNREAL_OFFLINE_COMPONENT_H_
#define UNREAL_OFFLINE_COMPONENT_H_

#include "../unreal_component.h"
#include "../module/offline_module.h"

namespace unreal
{
	class OfflineComponent final : public UnrealComponent<OfflineModule>
	{
	public:
		OfflineComponent() noexcept;
		virtual ~OfflineComponent() noexcept;

		void setMaxBounces(std::uint32_t num_bounces) noexcept;
		std::uint32_t getMaxBounces() const noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(OfflineComponent);
		}

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	private:
		OfflineComponent(const OfflineComponent&) = delete;
		OfflineComponent& operator=(const OfflineComponent&) = delete;
	};
}

#endif