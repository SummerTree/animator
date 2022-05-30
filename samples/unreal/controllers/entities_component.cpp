#include "entities_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"

namespace unreal
{
	EntitiesComponent::EntitiesComponent() noexcept
	{
	}

	EntitiesComponent::~EntitiesComponent() noexcept
	{
	}

	void
	EntitiesComponent::importAbc(std::string_view path) noexcept(false)
	{
		auto model = octoon::GameObject::create();
		model->addComponent<octoon::MeshAnimationComponent>(path);

		this->getContext()->profile->entitiesModule->objects.getValue().push_back(model);
		this->sendMessage("editor:project:open");
	}

	bool
	EntitiesComponent::importModel(std::string_view path) noexcept
	{
		auto model = octoon::MeshLoader::load(path);
		if (model)
		{
			auto smr = model->getComponent<octoon::SkinnedMeshRendererComponent>();
			if (smr)
				smr->setAutomaticUpdate(!this->getContext()->profile->offlineModule->getEnable());

			this->getContext()->profile->entitiesModule->objects.getValue().push_back(model);
			return true;
		}

		return false;
	}

	void
	EntitiesComponent::onInit() noexcept
	{
	}

	void
	EntitiesComponent::onEnable() noexcept
	{
	}

	void
	EntitiesComponent::onDisable() noexcept
	{
	}
}