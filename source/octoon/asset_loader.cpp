#include <octoon/asset_loader.h>
#include <octoon/vmd_loader.h>
#include <octoon/pmx_loader.h>
#include <octoon/obj_loader.h>
#include <octoon/ass_loader.h>
#include <octoon/fbx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/mesh_animation_component.h>
#include <octoon/runtime/md5.h>

namespace octoon
{
	OctoonImplementSingleton(AssetLoader)

	AssetLoader::AssetLoader() noexcept
	{
	}

	AssetLoader::~AssetLoader() noexcept
	{
	}

	std::filesystem::path
	AssetLoader::getAssetPath(const std::shared_ptr<const RttiObject>& asset) const noexcept
	{
		auto it = assetPathList_.find(asset);
		if (it != assetPathList_.end())
			return (*it).second;
		return std::filesystem::path();
	}

	std::filesystem::path
	AssetLoader::getAssetExtension(const std::shared_ptr<const RttiObject>& asset, std::string_view defaultExtension) const noexcept
	{
		auto assetPath = AssetLoader::instance()->getAssetPath(asset);
		if (!assetPath.empty())
			return assetPath.extension();
		return defaultExtension;
	}

	std::shared_ptr<RttiObject>
	AssetLoader::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		auto ext = path.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == u8".vmd")
		{
			auto motion = VMDLoader::load(path);
			if (motion)
			{
				if (motion->getName().empty())
					motion->setName((char*)path.filename().u8string().c_str());

				return motion;
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = std::make_shared<Texture>();
			if (texture->load(path))
			{
				texture->setName((char*)path.filename().u8string().c_str());
				return texture;
			}
		}
		else if (ext == u8".pmx")
		{
			auto model = PMXLoader::load(path, octoon::PMXLoadFlagBits::AllBit);
			if (model)
				return model;
		}
		else if (ext == u8".obj")
		{
			auto model = OBJLoader::load(path);
			if (model)
				return model;
		}
		else if (ext == u8".fbx")
		{
			auto model = FBXLoader::load(path);
			if (model)
				return model;
		}
		else if (ext == u8".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				auto alembic = model->addComponent<MeshAnimationComponent>();
				alembic->setFilePath(path);

				return model;
			}
		}

		return nullptr;
	}
}