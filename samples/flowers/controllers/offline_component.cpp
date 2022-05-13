#include "offline_component.h"
#include "../libs/nativefiledialog/nfd.h"
#include "../flower_profile.h"
#include "../flower_behaviour.h"

namespace flower
{
	OfflineComponent::OfflineComponent() noexcept
	{
	}

	OfflineComponent::~OfflineComponent() noexcept
	{
	}

	void
	OfflineComponent::setMaxBounces(std::uint32_t num_bounces) noexcept
	{
		if (this->getModel()->bounces != num_bounces)
		{
			this->getFeature<octoon::VideoFeature>()->setMaxBounces(num_bounces);
			this->getModel()->bounces = num_bounces;
		}		
	}

	std::uint32_t
	OfflineComponent::getMaxBounces() const noexcept
	{
		return this->getModel()->bounces;
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

		this->getFeature<octoon::VideoFeature>()->setMaxBounces(this->getModel()->bounces);
		this->getFeature<octoon::VideoFeature>()->setGlobalIllumination(true);

		this->sendMessage("flower:offline", true);
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

		this->getFeature<octoon::VideoFeature>()->setGlobalIllumination(false);

		this->sendMessage("flower:offline", false);
	}
}