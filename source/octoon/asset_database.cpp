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
	AssetDatabase::getAssetGuid(const std::shared_ptr<RttiObject>& asset) const noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		return std::string();
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