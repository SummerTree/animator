#include <octoon/material_importer.h>
#include <octoon/mdl_loader.h>
#include <octoon/PMREM_loader.h>
#include <octoon/asset_database.h>
#include <octoon/asset_bundle.h>
#include <octoon/video_feature.h>
#include <octoon/environment_light_component.h>
#include <octoon/runtime/uuid.h>
#include <octoon/io/fstream.h>
#include <octoon/mesh/sphere_mesh.h>

#include <filesystem>
#include <fstream>

namespace octoon
{
	OctoonImplementSingleton(MaterialImporter)

	MaterialImporter::MaterialImporter() noexcept
	{
	}

	MaterialImporter::~MaterialImporter() noexcept
	{
		this->close();
	}

	nlohmann::json
	MaterialImporter::createPackage(std::string_view path) noexcept(false)
	{
		octoon::io::ifstream stream((std::string)path);
		if (stream)
		{
			octoon::MDLLoader loader;
			loader.load("resource", stream);

			nlohmann::json items;

			for (auto& mat : loader.getMaterials())
			{
				auto package = AssetDatabase::instance()->createAsset(mat, assertPath_);

				items.push_back(package["uuid"]);
				this->indexList_.push_back(package["uuid"]);
			}

			this->saveAssets();

			return items;
		}

		return nlohmann::json();
	}

	nlohmann::json
	MaterialImporter::createPackage(const std::shared_ptr<octoon::Material>& material) noexcept(false)
	{
		if (material)
		{
			auto it = this->assetPackageCache_.find(material);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_[material];

			auto uuid = AssetDatabase::instance()->getAssetGuid(material);

			nlohmann::json package = AssetDatabase::instance()->getPackage(material);
			if (package.find("uuid") != package.end())
			{
				for (auto& index : indexList_)
				{
					if (index == uuid)
						return package;
				}
			}

			package = AssetDatabase::instance()->createAsset(material, assertPath_);
			this->assetPackageCache_[material] = package;

			return package;
		}

		return nlohmann::json();
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

	std::shared_ptr<octoon::Material>
	MaterialImporter::loadPackage(std::string_view uuid) noexcept(false)
	{
		if (materials_.find(std::string(uuid)) != materials_.end())
			return materials_[std::string(uuid)];

		auto package = MaterialImporter::instance()->getPackage(uuid);
		if (package.is_object())
			return AssetDatabase::instance()->loadAssetAtPackage<Material>(package);

		return nullptr;
	}

	bool
	MaterialImporter::addMaterial(const std::shared_ptr<octoon::Material>& mat)
	{
		auto it = this->assetList_.find(mat);
		if (it == this->assetList_.end())
		{
			auto standard = mat->downcast_pointer<octoon::MeshStandardMaterial>();
			auto uuid = octoon::make_guid();

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = mat->getName();
			package["color"] = standard->getColor().to_array();

			this->sceneList_.push_back(uuid);
			this->packageList_[uuid] = package;
			this->materials_[uuid] = mat;
			this->assetList_[mat] = package;

			return true;
		}
		else
		{
			auto uuid = (*it).second["uuid"].get<nlohmann::json::string_t>();

			bool found = false;
			for (auto index = sceneList_.begin(); index != sceneList_.end(); ++index)
			{
				if ((*index).get<nlohmann::json::string_t>() == uuid)
				{
					found = true;
					break;
				}
			}

			if (!found)
				sceneList_.push_back(uuid);

			this->materials_[uuid] = mat;

			return true;
		}

		return false;
	}
}