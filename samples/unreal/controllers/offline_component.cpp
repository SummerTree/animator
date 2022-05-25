#include "offline_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"

namespace unreal
{
	OfflineComponent::OfflineComponent() noexcept
	{
	}

	OfflineComponent::~OfflineComponent() noexcept
	{
	}

	void
	OfflineComponent::onInit() noexcept
	{
		this->getModel()->enable += [this](bool enable)
		{
			for (auto& object : this->getContext()->profile->entitiesModule->objects)
			{
				auto smr = object->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (smr)
					smr->setAutomaticUpdate(!enable);
			}

			auto videoFeature = this->getFeature<octoon::VideoFeature>();
			if (videoFeature)
			{
				videoFeature->setMaxBounces(this->getModel()->bounces);
				videoFeature->setGlobalIllumination(enable);
			}
		};

		this->getModel()->bounces += [this](std::uint32_t value)
		{
			auto videoFeature = this->getFeature<octoon::VideoFeature>();
			if (videoFeature)
				videoFeature->setMaxBounces(value);
		};
	}

	void
	OfflineComponent::onEnable() noexcept
	{
	}

	void
	OfflineComponent::onDisable() noexcept
	{
	}
}