#ifndef OCTOON_SPOT_LIGHT_COMPONENT_H_
#define OCTOON_SPOT_LIGHT_COMPONENT_H_

#include <octoon/light_component.h>
#include <octoon/light/spot_light.h>

namespace octoon
{
	class OCTOON_EXPORT SpotLightComponent final : public LightComponent
	{
		OctoonDeclareSubInterface(SpotLightComponent, LightComponent)
	public:
		SpotLightComponent() noexcept;
		virtual ~SpotLightComponent() noexcept;

		void setIntensity(float value) noexcept override;
		void setColor(const math::float3& value) noexcept override;

		GameComponentPtr clone() const noexcept override;

	private:
		void onActivate() noexcept override;
		void onDeactivate() noexcept override;

		void onMoveAfter() noexcept override;

		void onLayerChangeAfter() noexcept override;

	private:
		SpotLightComponent(const SpotLightComponent&) = delete;
		SpotLightComponent& operator=(const SpotLightComponent&) = delete;

	private:
		std::shared_ptr<light::SpotLight> spotLight_;
	};
}

#endif