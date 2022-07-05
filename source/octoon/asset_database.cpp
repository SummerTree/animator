#include <octoon/asset_database.h>
#include <octoon/asset_importer.h>
#include <octoon/asset_manager.h>
#include <octoon/game_component.h>
#include <octoon/runtime/md5.h>
#include <octoon/runtime/guid.h>
#include <octoon/vmd_importer.h>
#include <octoon/fbx_importer.h>
#include <octoon/pmx.h>

#include <fstream>

namespace octoon
{
	OctoonImplementSingleton(AssetDatabase)

	AssetDatabase::AssetDatabase() noexcept
	{
		this->assetPipeline_.push_back(std::make_shared<AssetPipeline>(u8""));
	}

	AssetDatabase::~AssetDatabase() noexcept
	{
		assetCaches_.clear();
		assetPipeline_.clear();
	}

	void
	AssetDatabase::mountPackage(const std::u8string& name, const std::filesystem::path& diskPath) noexcept(false)
	{
		for (auto& it : this->assetPipeline_)
		{
			if (it->getName() == name)
				throw std::runtime_error(std::string("Mount package at path ") + (char*)diskPath.u8string().c_str() + " failed.");
		}

		auto package = std::make_shared<AssetPipeline>(name);
		this->assetPipeline_.push_back(package);
		package->open(diskPath);
	}

	void
	AssetDatabase::unmountPackage(const std::u8string& name) noexcept(false)
	{
		for (auto it = this->assetPipeline_.begin(); it != this->assetPipeline_.end(); ++it)
		{
			if ((*it)->getName() == name)
			{
				this->assetPipeline_.erase(it);
				return;
			}
		}
	}

	void
	AssetDatabase::importAsset(const std::filesystem::path& relativePath) noexcept(false)
	{
		auto metadata = AssetManager::instance()->loadMetadataAtPath(relativePath);
		if (metadata.is_null())
		{
			if (!relativePath.is_absolute())
				AssetManager::instance()->createMetadataAtPath(relativePath);
		}
	}

	void
	AssetDatabase::importAsset(const std::filesystem::path& diskPath, const std::filesystem::path& relativePath) noexcept(false)
	{
		assert(!diskPath.empty());

		if (relativePath.empty())
			return;

		if (diskPath.empty() || !std::filesystem::exists(diskPath))
			return;

		auto rootPath = relativePath.parent_path();
		auto absolutePath = this->getAbsolutePath(relativePath);

		try
		{
			this->createFolder(rootPath);

			std::filesystem::copy_file(diskPath, absolutePath, std::filesystem::copy_options::overwrite_existing);
			std::filesystem::permissions(absolutePath, std::filesystem::perms::owner_write);

			AssetManager::instance()->createMetadataAtPath(relativePath);

			auto ext = diskPath.extension().u8string();
			for (auto& it : ext)
				it = (char)std::tolower(it);

			if (ext == u8".pmx")
			{
				auto pmx = std::make_shared<octoon::PMX>();
				if (octoon::PMX::load(diskPath, *pmx))
				{
					std::map<std::filesystem::path, std::wstring> diskPaths;
					for (auto& texture : pmx->textures)
					{
						if (std::filesystem::exists(texture.fullpath))
							diskPaths[texture.fullpath] = texture.name;
					}

					for (auto& path : diskPaths)
						this->importAsset(path.first, std::filesystem::path(rootPath).append(path.second));
				}
			}
			else if (ext == u8".fbx")
			{
				auto dependencies = octoon::FBXImporter::getDependencies(diskPath);

				std::map<std::filesystem::path, std::wstring> diskPaths;
				for (auto& it : dependencies)
				{
					auto fullpath = std::filesystem::path(diskPath.parent_path()).append(it.wstring());
					if (std::filesystem::exists(fullpath))
						diskPaths[fullpath] = it.wstring();
				}

				for (auto& path : diskPaths)
					this->importAsset(path.first, std::filesystem::path(rootPath).append(path.second));
			}
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(absolutePath))
				std::filesystem::remove(absolutePath);

			if (std::filesystem::exists(absolutePath.u8string() + u8".meta"))
				std::filesystem::remove(absolutePath.u8string() + u8".meta");

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Texture>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(relativePath))
			{
				it->createAsset(asset, relativePath);
				return;
			}
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Animation>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(relativePath))
			{
				it->createAsset(asset, relativePath);
				return;
			}
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Material>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(relativePath))
			{
				it->createAsset(asset, relativePath);
				return;
			}
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(relativePath))
			{
				it->createAsset(asset, relativePath);
				return;
			}
		}
	}

	void
	AssetDatabase::createPrefab(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating prefab at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset) && AssetDatabase::instance()->isPartOfPrefabAsset(asset))
			throw std::runtime_error(std::string("Creating prefab at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(relativePath))
			{
				it->createAsset(asset, relativePath);
				return;
			}
		}
	}

	bool
	AssetDatabase::isPartOfPrefabAsset(const std::shared_ptr<const GameObject>& asset) const noexcept
	{
		auto path = this->getAssetPath(asset);
		if (!path.empty())
		{
			auto ext = path.extension().wstring();
			for (auto& it : ext)
				it = (char)std::tolower(it);

			return ext == L".prefab";
		}

		for (auto& component : asset->getComponents())
		{
			auto assetPath = this->getAssetPath(component);
			if (!assetPath.empty())
			{
				auto ext = assetPath.extension().wstring();
				for (auto& it : ext)
					it = (char)std::tolower(it);

				if (ext == L".prefab" || ext == L".pmx" || ext == L".obj" || ext == L".fbx")
					return true;
			}
		}

		return false;
	}

	void
	AssetDatabase::deleteAsset(const std::filesystem::path& relativePath) noexcept(false)
	{
		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(relativePath))
			{
				it->deleteAsset(relativePath);
				return;
			}
		}
	}

	void
	AssetDatabase::saveAssets() noexcept(false)
	{
		for (auto& it : dirtyList_)
		{
			if (!it.expired())
			{
				auto item = it.lock();

				if (item->isInstanceOf<Texture>())
					this->createAsset(item->downcast_pointer<Texture>(), this->getAssetPath(item));
				else if (item->isInstanceOf<Material>())
					this->createAsset(item->downcast_pointer<Material>(), this->getAssetPath(item));
				else if (item->isInstanceOf<Animation>())
					this->createAsset(item->downcast_pointer<Animation>(), this->getAssetPath(item));
				else if (item->isInstanceOf<GameObject>())
					this->createAsset(item->downcast_pointer<GameObject>(), this->getAssetPath(item));
			}
		}

		dirtyList_.clear();

		for (auto& package : assetPipeline_)
			package->saveAssets();
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::string& uuid) const noexcept
	{
		return AssetManager::instance()->getAssetPath(uuid);
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::shared_ptr<const Object>& object) const noexcept
	{
		return AssetManager::instance()->getAssetPath(object);
	}

	std::filesystem::path
	AssetDatabase::getAbsolutePath(const std::filesystem::path& path) const noexcept
	{
		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(path))
				return it->getAbsolutePath(path);
		}

		return std::filesystem::path();
	}

	std::filesystem::path
	AssetDatabase::getAssetExtension(const std::shared_ptr<const Object>& asset, std::string_view defaultExtension) const noexcept
	{
		auto path = this->getAssetPath(asset);
		if (!path.empty())
			return path.extension();

		return defaultExtension;
	}

	std::string
	AssetDatabase::getAssetGuid(const std::filesystem::path& path) const noexcept
	{
		return AssetManager::instance()->getAssetGuid(path);
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<const Object>& asset) const noexcept
	{
		auto path = this->getAssetPath(asset);
		if (!path.empty())
			return this->getAssetGuid(path);
		return std::string();
	}

	void
	AssetDatabase::createFolder(const std::filesystem::path& relativePath) noexcept(false)
	{
		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(relativePath))
			{
				it->createFolder(relativePath);
				return;
			}
		}
	}

	void
	AssetDatabase::deleteFolder(const std::filesystem::path& relativePath) noexcept(false)
	{
		for (auto& it : assetPipeline_)
		{
			if (it->isValidPath(relativePath))
			{
				it->deleteFolder(relativePath);
				return;
			}
		}
	}

	bool
	AssetDatabase::contains(const std::shared_ptr<const Object>& asset) const noexcept
	{
		auto assetPath = this->getAssetPath(asset);
		if (!assetPath.empty())
		{
			if (!assetPath.is_absolute())
				return std::filesystem::exists(this->getAbsolutePath(assetPath));
		}

		return false;
	}

	void
	AssetDatabase::setLabels(const std::shared_ptr<const Object>& asset, std::vector<std::string>&& labels) noexcept(false)
	{
		auto assetPath = this->getAssetPath(asset);
		if (!assetPath.empty())
		{
			auto metaData = AssetManager::instance()->loadMetadataAtPath(assetPath);
			if (metaData.is_object())
			{
				metaData["label"] = labels;
				AssetManager::instance()->createMetadataAtPath(assetPath, metaData);
			}

			this->labels_[asset] = std::move(labels);
		}
		else
		{
			throw std::runtime_error(std::string("Failed to set labels."));
		}
	}

	void
	AssetDatabase::setLabels(const std::shared_ptr<const Object>& asset, const std::vector<std::string>& labels) noexcept(false)
	{
		auto assetPath = this->getAssetPath(asset);
		if (!assetPath.empty())
		{
			auto metaData = AssetManager::instance()->loadMetadataAtPath(assetPath);
			if (metaData.is_object())
			{
				metaData["label"] = labels;
				AssetManager::instance()->createMetadataAtPath(assetPath, metaData);
			}

			this->labels_[asset] = labels;
		}
		else
		{
			throw std::runtime_error(std::string("Failed to set labels."));
		}
	}

	const std::vector<std::string>&
	AssetDatabase::getLabels(const std::shared_ptr<const Object>& asset) noexcept(false)
	{
		auto assetPath = this->getAssetPath(asset);
		if (!assetPath.empty())
		{
			auto it = this->labels_.find(asset);
			if (it != this->labels_.end())
				return it->second;

			auto metaData = AssetManager::instance()->loadMetadataAtPath(assetPath);
			if (metaData.is_object())
			{
				for (auto& label : metaData["label"])
					this->labels_[asset].push_back(label.get<std::string>());

				return this->labels_[asset];
			}

			return defaultLabel_;
		}
		else
		{
			throw std::runtime_error(std::string("Failed to get labels."));
		}
	}

	bool
	AssetDatabase::isSubAsset(const std::shared_ptr<const Object>& asset) const noexcept
	{
		return false;
	}

	bool
	AssetDatabase::getGUIDAndLocalIdentifier(const std::shared_ptr<const Object>& asset, std::string& outGuid, std::int64_t& outLocalId)
	{
		auto assetPath = this->getAssetPath(asset);
		if (!assetPath.empty())
		{
			outGuid = this->getAssetGuid(assetPath);
			outLocalId = 0;
			return true;
		}

		return false;
	}

	std::shared_ptr<Object>
	AssetDatabase::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		if (!path.empty())
		{
			if (assetCaches_.contains(path))
			{
				auto cache = assetCaches_.at(path);
				if (!cache.expired())
					return cache.lock();
			}

			for (auto& it : assetPipeline_)
			{
				if (it->isValidPath(path))
				{
					auto asset = it->loadAssetAtPath(path);
					if (asset)
						assetCaches_[path] = asset;

					return asset;
				}
			}
		}

		return nullptr;
	}

	void
	AssetDatabase::setDirty(const std::shared_ptr<Object>& object, bool dirty) noexcept(false)
	{
		if (this->contains(object))
		{
			if (dirty)
				this->dirtyList_.insert(object);
			else
			{
				auto it = this->dirtyList_.find(object);
				if (it != this->dirtyList_.end())
					this->dirtyList_.erase(it);
			}
		}
	}

	bool
	AssetDatabase::isDirty() const noexcept
	{
		return !this->dirtyList_.empty();
	}

	bool
	AssetDatabase::isDirty(const std::shared_ptr<Object>& object) const noexcept
	{
		return this->dirtyList_.contains(object);
	}
}