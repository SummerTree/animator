#include <octoon/asset_database.h>
#include <octoon/asset_importer.h>
#include <octoon/game_component.h>

namespace octoon
{
	OctoonImplementSingleton(AssetDatabase)

	AssetDatabase::AssetDatabase() noexcept
		: defaultPackage_(std::make_shared<Package>(this))
	{
	}

	AssetDatabase::~AssetDatabase() noexcept
	{
		objectCaches_.clear();
		this->packages_.clear();
	}

	void
	AssetDatabase::mountPackage(const std::u8string& name, const std::filesystem::path& diskPath) noexcept(false)
	{
		if (!this->packages_.contains(name))
		{
			auto package = std::make_shared<Package>(this);
			package->open(diskPath);

			this->packages_[name] = std::move(package);
		}
	}

	void
	AssetDatabase::unmountPackage(const std::u8string& name) noexcept(false)
	{
		this->packages_.erase(this->packages_.find(name));
	}

	void
	AssetDatabase::importAsset(const std::filesystem::path& diskPath, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			package->importAsset(diskPath, packagePath);
		else
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Texture>& asset, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
		{
			package->createAsset(asset, packagePath);
		}
		else
		{
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Animation>& asset, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
		{
			package->createAsset(asset, packagePath);
		}
		else
		{
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Material>& asset, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
		{
			package->createAsset(asset, packagePath);
		}
		else
		{
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
		{
			package->createAsset(asset, packagePath);
		}
		else
		{
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
		}
	}

	void
	AssetDatabase::createPrefab(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
		{
			package->createPrefab(asset, packagePath);
		}
		else
		{
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
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
	AssetDatabase::deleteAsset(const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			package->deleteAsset(packagePath);
		else
			throw std::runtime_error(std::string("Destroy asset at path ") + (char*)path.u8string().c_str() + " failed.");
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

		for (auto& package : packages_)
			package.second->saveAssets();
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::string& uuid) const noexcept
	{
		for (auto& package : packages_)
		{
			auto path = package.second->getAssetPath(uuid);
			if (!path.empty())
				return std::filesystem::path(package.first).append(path.wstring());
		}

		return std::filesystem::path();
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::shared_ptr<const Object>& object) const noexcept
	{
		auto asset = assetToPath_.find(object);
		if (asset != assetToPath_.end())
			return asset->second;

		return std::filesystem::path();
	}

	std::filesystem::path
	AssetDatabase::getAbsolutePath(const std::filesystem::path& path) const noexcept
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			return package->getAbsolutePath(packagePath);

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
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			return package->getAssetGuid(packagePath);
		return std::string();
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
	AssetDatabase::createFolder(const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			return package->createFolder(packagePath);
	}

	void
	AssetDatabase::deleteFolder(const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			return package->deleteFolder(packagePath);
	}

	std::shared_ptr<Package>
	AssetDatabase::getPackage(const std::filesystem::path& assetPath, std::filesystem::path& packagePath) const noexcept(false)
	{
		if (assetPath.is_absolute())
		{
			packagePath = assetPath;
			return defaultPackage_;
		}
		else
		{
			auto path = std::filesystem::path(assetPath).u8string();
			for (auto& it : path)
			{
				if (it == '\\')
					it = '/';
			}

			for (auto& package : packages_)
			{
				auto length = package.first.length();
				if (path.size() > length)
				{
					if (package.first == path.substr(0, length))
					{
						packagePath = path.substr(length);
						return package.second;
					}
				}
			}

			throw std::runtime_error(std::string("Invalid path ") + (char*)assetPath.u8string().c_str() + " failed.");
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
			std::filesystem::path packagePath;
			auto package = this->getPackage(assetPath, packagePath);
			if (package)
			{
				auto metaData = package->loadMetadataAtPath(packagePath);
				if (metaData.is_object())
				{
					metaData["label"] = labels;
					package->createMetadataAtPath(packagePath, metaData);
				}

				this->labels_[asset] = std::move(labels);
			}
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
			std::filesystem::path packagePath;
			auto package = this->getPackage(assetPath, packagePath);
			if (package)
			{
				auto metaData = package->loadMetadataAtPath(packagePath);
				if (metaData.is_object())
				{
					metaData["label"] = labels;
					package->createMetadataAtPath(packagePath, metaData);
				}

				this->labels_[asset] = labels;
			}
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
			std::filesystem::path packagePath;
			auto package = this->getPackage(assetPath, packagePath);
			if (package)
			{
				auto it = this->labels_.find(asset);
				if (it != this->labels_.end())
					return it->second;

				auto metaData = package->loadMetadataAtPath(packagePath);
				if (metaData.is_object())
				{
					for (auto& label : metaData["label"])
						this->labels_[asset].push_back(label.get<std::string>());

					return this->labels_[asset];
				}
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
			if (objectCaches_.contains(path))
			{
				auto cache = objectCaches_.at(path);
				if (!cache.expired())
					return cache.lock();
			}

			std::filesystem::path packagePath;
			auto package = this->getPackage(path, packagePath);
			if (package)
			{
				auto asset = AssetImporter::loadAssetAtPath(package->getAbsolutePath(packagePath));
				if (asset)
				{
					auto metadata = package->loadMetadataAtPath(packagePath);
					if (metadata.is_object())
					{
						if (metadata.contains("labels"))
						{
							for (auto& it : metadata["labels"])
								labels_[asset].push_back(it.get<std::string>());
						}
					}
					else
					{
						if (!path.is_absolute())
							package->createMetadataAtPath(packagePath);
					}

					assetToPath_[asset] = path;
					objectCaches_[path] = asset;

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