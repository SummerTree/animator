#include <octoon/material_importer.h>
#include <octoon/asset_database.h>

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

	std::string
	MaterialImporter::getAssetGuid(const std::shared_ptr<const octoon::Material>& asset) noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		return std::string();
	}

	std::string
	MaterialImporter::getAssetGuid(const std::shared_ptr<const octoon::Material>& asset) const noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		return std::string();
	}

	std::shared_ptr<Material>
	MaterialImporter::getMaterial(std::string_view uuid) noexcept(false)
	{
		if (materials_.contains(std::string(uuid)))
			return materials_[std::string(uuid)];

		return nullptr;
	}

	nlohmann::json
	MaterialImporter::getPackage(std::string_view uuid) noexcept
	{
		if (packageList_.contains((std::string)uuid))
			return packageList_[(std::string)uuid];

		return nlohmann::json();
	}

	bool
	MaterialImporter::addMaterial(const std::shared_ptr<Material>& mat)
	{
		if (!assetGuidList_.contains(mat))
		{
			auto standard = mat->downcast_pointer<MeshStandardMaterial>();
			auto uuid = AssetDatabase::instance()->getAssetGuid(mat);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = mat->getName();
			package["color"] = standard->getColor().to_array();

			this->sceneList_.push_back(uuid);
			this->materials_[uuid] = mat;
			this->assetGuidList_[mat] = uuid;
			this->packageList_[uuid] = package;

			return true;
		}

		return false;
	}
}