#ifndef UNREAL_CAMERA_COMPONENT_H_
#define UNREAL_CAMERA_COMPONENT_H_

#include "../unreal_component.h"
#include "../module/camera_module.h"

namespace unreal
{
	class CameraComponent final : public UnrealComponent<CameraModule>
	{
	public:
		CameraComponent() noexcept;
		virtual ~CameraComponent() noexcept;

		bool loadAnimation(std::string_view filepath) noexcept(false);
		void removeAnimation() noexcept(false);

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(CameraComponent);
		}

	private:
		void onInit() noexcept override;

		void onEnable() noexcept override;
		void onDisable() noexcept override;

		void update() noexcept;

	private:
		CameraComponent(const CameraComponent&) = delete;
		CameraComponent& operator=(const CameraComponent&) = delete;

	private:
		float aperture;
	};
}

#endif