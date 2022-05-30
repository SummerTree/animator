#include "main_light_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"

namespace unreal
{
	MainLightComponent::MainLightComponent() noexcept
	{
	}

	MainLightComponent::~MainLightComponent() noexcept
	{
	}

	void
	MainLightComponent::onInit() noexcept
	{
		this->getModel()->size += [this](float value)
		{
			auto& profile = this->getContext()->profile;

			auto& mainLight = profile->mainLightModule->mainLight.getValue();
			if (mainLight)
			{
				auto lightComponment = mainLight->getComponent<octoon::DirectionalLightComponent>();
				if (lightComponment)
					lightComponment->setSize(value);
			}
		};

		this->getModel()->intensity += [this](float value)
		{
			auto& profile = this->getContext()->profile;

			auto& mainLight = profile->mainLightModule->mainLight.getValue();
			if (mainLight)
			{
				auto lightComponment = mainLight->getComponent<octoon::DirectionalLightComponent>();
				if (lightComponment)
					lightComponment->setIntensity(value);
			}
		};

		this->getModel()->color += [this](const octoon::math::float3& value)
		{
			auto& profile = this->getContext()->profile;

			auto& mainLight = profile->mainLightModule->mainLight.getValue();
			if (mainLight)
			{
				auto lightComponment = mainLight->getComponent<octoon::DirectionalLightComponent>();
				if (lightComponment)
					lightComponment->setColor(value);
			}
		};

		this->getModel()->rotation += [this](const octoon::math::float3& value)
		{
			auto& profile = this->getContext()->profile;

			auto& mainLight = profile->mainLightModule->mainLight.getValue();
			if (mainLight)
			{
				auto transform = mainLight->getComponent<octoon::TransformComponent>();
				if (transform)
				{
					transform->setQuaternion(octoon::math::Quaternion(octoon::math::radians(value)));
					transform->setTranslate(-octoon::math::rotate(transform->getQuaternion(), octoon::math::float3::UnitZ) * 50);
				}
			}
		};
	}

	void
	MainLightComponent::onEnable() noexcept
	{
	}

	void
	MainLightComponent::onDisable() noexcept
	{
	}
}