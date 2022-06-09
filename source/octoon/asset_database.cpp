#include <octoon/asset_database.h>
#include <octoon/runtime/uuid.h>
#include <octoon/vmd_loader.h>
#include <octoon/pmx_loader.h>
#include <octoon/ass_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/animation/animation.h>
#include <octoon/mesh_animation_component.h>
#include <fstream>
#include <filesystem>
#include <codecvt>

namespace octoon
{
	OctoonImplementSingleton(AssetDatabase)

	AssetDatabase::AssetDatabase() noexcept
	{
	}

	AssetDatabase::~AssetDatabase() noexcept
	{
	}

	std::string
	AssetDatabase::getAssetPath(const std::shared_ptr<RttiObject>& asset) const noexcept
	{
		if (assetPathList_.contains(asset))
			return assetPathList_.at(asset);
		return std::string();
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<RttiObject>& asset) noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		else
		{
			auto guid = octoon::make_guid();
			assetGuidList_[asset] = guid;
			return guid;
		}
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<RttiObject>& asset) const noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		return std::string();
	}

	nlohmann::json
	AssetDatabase::createAsset(const std::shared_ptr<octoon::Animation>& animation, std::string_view outputPath) noexcept
	{
		if (animation)
		{
			auto uuid = this->getAssetGuid(animation);
			auto rootPath = std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid);
			auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directories(rootPath);

			octoon::VMDLoader::save(motionPath.string(), *animation);
			std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = uuid + ".vmd";
			package["path"] = (char*)motionPath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->packageList_[std::string(uuid)] = package;

			return package;
		}

		return nlohmann::json();
	}

	std::shared_ptr<RttiObject>
	AssetDatabase::loadAssetAtPath(std::string_view path) noexcept(false)
	{
		auto ext = std::string(path.substr(path.find_last_of('.')));
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == ".vmd")
		{
			auto motion = octoon::VMDLoader::load(path);
			if (motion)
			{
				assetPathList_[motion] = path;
				assetGuidList_[motion] = make_guid();
				return motion;
			}
		}
		else if (ext == ".hdr" || ext == ".bmp" || ext == ".tga" || ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".dds")
		{
			auto texture = std::make_shared<octoon::Texture>((std::string)path);
			if (texture)
			{
				assetPathList_[texture] = path;
				assetGuidList_[texture] = make_guid();
				return texture;
			}
		}
		else if (ext == ".pmx")
		{
			auto model = PMXLoader::load(path, PMXLoadFlagBits::AllBit);
			if (model)
			{
				assetPathList_[model] = path;
				assetGuidList_[model] = make_guid();
				return model;
			}
		}
		else if (ext == ".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				model->addComponent<MeshAnimationComponent>(path);
				assetPathList_[model] = path;
				assetGuidList_[model] = make_guid();
				return model;
			}
		}

		return nullptr;
	}
}