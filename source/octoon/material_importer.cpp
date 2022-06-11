#include <octoon/material_importer.h>
#include <octoon/runtime/uuid.h>

namespace octoon
{
	OctoonImplementSingleton(MaterialImporter)

	MaterialImporter::MaterialImporter() noexcept
	{
	}

	MaterialImporter::~MaterialImporter() noexcept
	{
	}

	nlohmann::json&
	MaterialImporter::getSceneList() noexcept
	{
		return this->sceneList_;
	}

	const nlohmann::json&
	MaterialImporter::getSceneList() const noexcept
	{
		return this->sceneList_;
	}

	std::shared_ptr<Material>
	MaterialImporter::getMaterial(std::string_view uuid) noexcept(false)
	{
		if (materials_.contains(std::string(uuid)))
			return materials_[std::string(uuid)];

		return nullptr;
	}

	bool
	MaterialImporter::addMaterial(const std::shared_ptr<Material>& mat)
	{
		if (!materialUUIDs_.contains(mat))
		{
			auto standard = mat->downcast_pointer<MeshStandardMaterial>();
			auto uuid = make_guid();

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = mat->getName();
			package["color"] = standard->getColor().to_array();

			this->sceneList_.push_back(uuid);
			this->materials_[uuid] = mat;
			this->materialUUIDs_[mat] = uuid;

			return true;
		}

		return false;
	}
}