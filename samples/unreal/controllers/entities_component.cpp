#include "entities_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"
#include <octoon/ass_loader.h>

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

		this->getContext()->profile->entitiesModule->objects.push_back(model);
		this->sendMessage("editor:project:open");
	}

	void
	EntitiesComponent::importAss(std::string_view path) noexcept(false)
	{
		auto& context = this->getContext();

		octoon::GameObjects objects = octoon::ASSLoader::load(path);

		for (auto& it : objects)
		{
			if (it->getComponent<octoon::CameraComponent>())
				context->profile->cameraModule->camera = it;
			else
			{
				auto renderer = it->getComponent<octoon::MeshRendererComponent>();
				if (renderer)
					renderer->setGlobalIllumination(true);

				context->profile->entitiesModule->objects.push_back(it);
			}
		}

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

			this->getContext()->profile->entitiesModule->objects.push_back(model);
			return true;
		}

		return false;
	}

	bool
	EntitiesComponent::exportModel(std::string_view path) noexcept
	{
		return false;
	}

	void
	EntitiesComponent::onInit() noexcept
	{
		this->getModel()->entities += [this](const std::vector<EntitesObject>& entites)
		{
			for (auto& it : entites)
			{
				if (!it.filepath.empty())
				{
					this->getModel()->objects.push_back(octoon::MeshLoader::load(it.filepath));
				}
			}
		};
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