#include <octoon/asset_loader.h>
#include <octoon/vmd_loader.h>
#include <octoon/pmx_loader.h>
#include <octoon/obj_loader.h>
#include <octoon/ass_loader.h>
#include <octoon/fbx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/mesh_animation_component.h>

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
	AssetLoader::getAssetPath(const std::shared_ptr<const Object>& asset) const noexcept
	{
		auto it = pathList_.find(asset);
		if (it != pathList_.end())
			return (*it).second;
		return std::filesystem::path();
	}

	std::filesystem::path
	AssetLoader::getAssetExtension(const std::shared_ptr<const Object>& asset, std::string_view defaultExtension) const noexcept
	{
		auto assetPath = AssetLoader::instance()->getAssetPath(asset);
		if (!assetPath.empty())
			return assetPath.extension();
		return defaultExtension;
	}

	std::shared_ptr<Object>
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

				pathList_[motion] = path;
				return std::move(motion);
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = std::make_shared<Texture>();
			if (texture->load(path))
			{
				texture->setName((char*)path.filename().u8string().c_str());
				pathList_[texture] = path;
				return std::move(texture);
			}
		}
		else if (ext == u8".pmx")
		{
			auto model = PMXLoader::load(path, octoon::PMXLoadFlagBits::AllBit);
			if (model)
			{
				pathList_[model] = path;
				return std::move(model);
			}
		}
		else if (ext == u8".obj")
		{
			auto model = OBJLoader::load(path);
			if (model)
			{
				pathList_[model] = path;
				return std::move(model);
			}
		}
		else if (ext == u8".fbx")
		{
			auto model = FBXLoader::load(path);
			if (model)
			{
				pathList_[model] = path;
				return std::move(model);
			}
		}
		else if (ext == u8".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				auto alembic = model->addComponent<MeshAnimationComponent>();
				alembic->setFilePath(path);
				pathList_[model] = path;
				return std::move(model);
			}
		}

		return nullptr;
	}
}