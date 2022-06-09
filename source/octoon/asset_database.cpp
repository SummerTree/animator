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

	std::shared_ptr<octoon::RttiObject>
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
				return motion;
			}
		}
		else if (ext == ".hdr" || ext == ".bmp" || ext == ".tga" || ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".dds")
		{
			auto texture = std::make_shared<octoon::Texture>((std::string)path);
			if (texture)
			{
				assetPathList_[texture] = path;
				return texture;
			}
		}
		else if (ext == ".pmx")
		{
			auto model = PMXLoader::load(path, PMXLoadFlagBits::AllBit);
			if (model)
			{
				assetPathList_[model] = path;
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
				return model;
			}
		}

		return nullptr;
	}
}