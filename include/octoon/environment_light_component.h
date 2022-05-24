#ifndef OCTOON_ENVIRONMENT_LIGHT_COMPONENT_H_
#define OCTOON_ENVIRONMENT_LIGHT_COMPONENT_H_

#include <octoon/light_component.h>
#include <octoon/light/environment_light.h>

namespace octoon
{
	class OCTOON_EXPORT EnvironmentLightComponent final : public LightComponent
	{
		OctoonDeclareSubInterface(EnvironmentLightComponent, LightComponent)
	public:
		EnvironmentLightComponent() noexcept;
		virtual ~EnvironmentLightComponent() noexcept;

		void setIntensity(float value) noexcept override;
		void setColor(const math::float3& value) noexcept override;

		void setOffset(const math::float2& value) noexcept;
		const math::float2&  getOffset() const noexcept;

		void setShowBackground(bool show) noexcept;
		bool getShowBackground() const noexcept;

		void setBackgroundMap(const std::shared_ptr<GraphicsTexture>& texture) noexcept;
		const std::shared_ptr<GraphicsTexture>& getBackgroundMap() const noexcept;

		void setEnvironmentMap(const std::shared_ptr<GraphicsTexture>& texture) noexcept;
		const std::shared_ptr<GraphicsTexture>& getEnvironmentMap() const noexcept;

		GameComponentPtr clone() const noexcept override;

	private:
		void onActivate() noexcept override;
		void onDeactivate() noexcept override;

		void onMoveAfter() noexcept override;

		void onLayerChangeAfter() noexcept override;

	private:
		EnvironmentLightComponent(const EnvironmentLightComponent&) = delete;
		EnvironmentLightComponent& operator=(const EnvironmentLightComponent&) = delete;

	private:
		bool showBackground_;
		math::float2 offset_;
		std::shared_ptr<GraphicsTexture> radiance_;
		std::shared_ptr<GraphicsTexture> backgroundMap_;
		std::shared_ptr<GraphicsTexture> environmentMap_;
		std::shared_ptr<EnvironmentLight> environmentLight_;
	};
}

#endif