#include "material_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"
#include "../importer/material_importer.h"

namespace unreal
{
	MaterialComponent::MaterialComponent() noexcept
	{
	}

	MaterialComponent::~MaterialComponent() noexcept
	{
	}

	void
	MaterialComponent::onEnable() noexcept(false)
	{
		this->addMessageListener("editor:selected", [this](const std::any& data) {
			if (data.has_value())
			{
				auto hit = std::any_cast<octoon::RaycastHit>(data);
				if (!hit.object.lock())
					return;

				octoon::Materials materials;
				auto renderComponent = hit.object.lock()->getComponent<octoon::MeshRendererComponent>();
				if (renderComponent)
					materials = renderComponent->getMaterials();
				else
				{
					auto smr = hit.object.lock()->getComponent<octoon::SkinnedMeshRendererComponent>();
					if (smr)
						materials = smr->getMaterials();
				}

				bool dirty = false;

				for (auto& mat : materials)
				{
					dirty |= MaterialImporter::instance()->addMaterial(mat);
				}

				if (dirty)
					MaterialImporter::instance()->getSceneList().submit();

				this->sendMessage("editor:material:selected", MaterialImporter::instance()->getSceneMetadate(materials[hit.mesh]));
			}
		});
	}

	void
	MaterialComponent::onDisable() noexcept
	{
	}
}