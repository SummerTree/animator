#include <octoon/asset_importer.h>
#include <octoon/texture_importer.h>
#include <octoon/vmd_importer.h>
#include <octoon/pmx_importer.h>
#include <octoon/obj_importer.h>
#include <octoon/ass_importer.h>
#include <octoon/fbx_importer.h>
#include <octoon/mesh_animation_component.h>

namespace octoon
{
	OctoonImplementSingleton(AssetImporter)
	OctoonImplementSubClass(AssetImporter, Object, "AssetImporter")

	std::map<std::filesystem::path, std::shared_ptr<const AssetImporter>> AssetImporter::assets_;

	AssetImporter::AssetImporter() noexcept
	{
	}

	AssetImporter::~AssetImporter() noexcept
	{
	}

	void
	AssetImporter::setAssetPath(const std::shared_ptr<const Object>& asset, const std::filesystem::path& path) noexcept
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

	std::shared_ptr<const AssetImporter>
	AssetImporter::getAtPath(const std::filesystem::path& path) const noexcept
	{
		if (assets_.find(path) != assets_.end())
			return assets_.at(path);
		return nullptr;
	}

	std::filesystem::path
	AssetImporter::getAssetPath(const std::shared_ptr<const Object>& object) const noexcept
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
	AssetImporter::getAssetExtension(const std::shared_ptr<const Object>& asset, std::string_view defaultExtension) const noexcept
	{
		auto assetPath = AssetImporter::instance()->getAssetPath(asset);
		if (!assetPath.empty())
			return assetPath.extension();
		return defaultExtension;
	}

	bool
	AssetImporter::isSubAsset(const std::shared_ptr<const Object>& asset) const noexcept
	{
		return this->subAssetToPath_.contains(asset);
	}

	void
	AssetImporter::addRemap(const std::shared_ptr<const Object>& asset, const std::filesystem::path& path)
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
	AssetImporter::unload() noexcept
	{
		caches_.clear();
	}

	std::shared_ptr<Object>
	AssetImporter::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		auto ext = path.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == u8".vmd")
		{
			auto motionImporter = std::make_shared<VMDImporter>();
			auto motion = motionImporter->load(path);
			if (motion)
			{
				caches_.push_back(motion);
				assetToPath_[motion] = path;
				return std::move(motion);
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto textureImporter = std::make_shared<TextureImporter>();
			auto texture = textureImporter->load(path);
			if (texture)
			{
				this->assets_[path] = textureImporter;

				caches_.push_back(texture);
				assetToPath_[texture] = path;
				return texture;
			}
		}
		else if (ext == u8".pmx")
		{
			auto modelImporter = std::make_shared<PMXImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				this->assets_[path] = modelImporter;

				caches_.push_back(model);
				assetToPath_[model] = path;
				return model;
			}
		}
		else if (ext == u8".obj")
		{
			auto modelImporter = std::make_shared<OBJImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				this->assets_[path] = modelImporter;

				caches_.push_back(model);
				assetToPath_[model] = path;
				return std::move(model);
			}
		}
		else if (ext == u8".fbx")
		{
			auto modelImporter = std::make_shared<FBXImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				this->assets_[path] = modelImporter;

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
					this->addRemap(it, path);

				return std::move(model);
			}
		}

		return nullptr;
	}
}