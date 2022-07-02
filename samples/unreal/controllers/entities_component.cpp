#include "entities_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"
#include <octoon/asset_importer.h>

namespace unreal
{
	EntitiesComponent::EntitiesComponent() noexcept
	{
	}

	EntitiesComponent::~EntitiesComponent() noexcept
	{
	}

	octoon::GameObjectPtr
	EntitiesComponent::importModel(const std::filesystem::path& path) noexcept(false)
	{
		auto model = octoon::AssetImporter::instance()->loadAssetAtPath<octoon::GameObject>(path);
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