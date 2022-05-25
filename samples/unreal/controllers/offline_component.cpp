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
		for (auto& object : this->getContext()->profile->entitiesModule->objects)
		{
			auto smr = object->getComponent<octoon::SkinnedMeshRendererComponent>();
			if (smr)
				smr->setAutomaticUpdate(false);
		}

		auto videoFeature = this->getFeature<octoon::VideoFeature>();
		if (videoFeature)
		{
			videoFeature->setMaxBounces(this->getModel()->bounces);
			videoFeature->setGlobalIllumination(true);
		}
	}

	void
	OfflineComponent::onDisable() noexcept
	{
		for (auto& object : this->getContext()->profile->entitiesModule->objects)
		{
			auto smr = object->getComponent<octoon::SkinnedMeshRendererComponent>();
			if (smr)
				smr->setAutomaticUpdate(true);
		}

		auto videoFeature = this->getFeature<octoon::VideoFeature>();
		if (videoFeature)
			videoFeature->setGlobalIllumination(false);
	}
}