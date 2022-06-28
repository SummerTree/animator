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

	void
	AssetLoader::setAssetPath(const std::shared_ptr<const Object>& asset, const std::filesystem::path& path) noexcept
	{
		auto it = assetToPath_.find(asset);
		if (it != assetToPath_.end())
		{
			for (auto& subAsset : this->subAssetToPath_)
			{
				if (subAsset.second == it->second)
					subAsset.second = path;
			}

			it->second = path;
		}
		else
		{
			assetToPath_[asset] = path;
		}
	}

	std::filesystem::path
	AssetLoader::getAssetPath(const std::shared_ptr<const Object>& object) const noexcept
	{
		auto asset = assetToPath_.find(object);
		if (asset != assetToPath_.end())
			return asset->second;

		auto subAsset = subAssetToPath_.find(object);
		if (subAsset != subAssetToPath_.end())
			return subAsset->second;

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

	bool
	AssetLoader::isSubAsset(const std::shared_ptr<const Object>& asset) const noexcept
	{
		return this->subAssetToPath_.contains(asset);
	}

	void
	AssetLoader::addObjectToAsset(const std::shared_ptr<const Object>& asset, const std::filesystem::path& path)
	{
		if (!this->isSubAsset(asset))
		{
			this->subAssetToPath_[asset] = path;
		}
		else
		{
			throw std::runtime_error(std::string("Add object to path ") + (char*)path.u8string().c_str() + " failed.");
		}
	}

	void
	AssetLoader::unload() noexcept
	{
		caches_.clear();
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
				caches_.push_back(motion);
				if (motion->getName().empty())
					motion->setName((char*)path.filename().u8string().c_str());

				assetToPath_[motion] = path;
				return std::move(motion);
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = std::make_shared<Texture>();
			if (texture->load(path))
			{
				caches_.push_back(texture);
				texture->setName((char*)path.filename().u8string().c_str());
				assetToPath_[texture] = path;
				return std::move(texture);
			}
		}
		else if (ext == u8".pmx")
		{
			auto model = PMXLoader::load(path, octoon::PMXLoadFlagBits::AllBit);
			if (model)
			{
				for (auto it : model->getComponents())
					this->addObjectToAsset(it, path);

				caches_.push_back(model);
				assetToPath_[model] = path;

				return std::move(model);
			}
		}
		else if (ext == u8".obj")
		{
			auto model = OBJLoader::load(path);
			if (model)
			{
				for (auto it : model->getComponents())
					this->addObjectToAsset(it, path);

				caches_.push_back(model);
				assetToPath_[model] = path;
				return std::move(model);
			}
		}
		else if (ext == u8".fbx")
		{
			auto model = FBXLoader::load(path);
			if (model)
			{
				for (auto it : model->getComponents())
					this->addObjectToAsset(it, path);

				caches_.push_back(model);
				assetToPath_[model] = path;
				return std::move(model);
			}
		}
		else if (ext == u8".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				caches_.push_back(model);
				auto alembic = model->addComponent<MeshAnimationComponent>();
				alembic->setFilePath(path);
				assetToPath_[model] = path;

				for (auto it : model->getComponents())
					this->addObjectToAsset(it, path);

				return std::move(model);
			}
		}

		return nullptr;
	}
}