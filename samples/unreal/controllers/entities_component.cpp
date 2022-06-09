#include "entities_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"
#include <octoon/model_importer.h>

namespace unreal
{
	EntitiesComponent::EntitiesComponent() noexcept
	{
	}

	EntitiesComponent::~EntitiesComponent() noexcept
	{
	}

	octoon::GameObjectPtr
	EntitiesComponent::importModel(std::string_view path) noexcept(false)
	{
		auto model = octoon::ModelImporter::instance()->importModel(path);
		if (model)
		{
			auto smr = model->getComponent<octoon::SkinnedMeshRendererComponent>();
			if (smr)
				smr->setAutomaticUpdate(!this->getContext()->profile->offlineModule->getEnable());

			this->getContext()->profile->entitiesModule->objects.getValue().push_back(model);
			return model;
		}

		return nullptr;
	}
}