#include <octoon/asset_database.h>

namespace octoon
{
	OctoonImplementSingleton(AssetDatabase)

	AssetDatabase::AssetDatabase() noexcept
	{
	}

	AssetDatabase::~AssetDatabase() noexcept
	{
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
			package->createAsset(asset, packagePath);
		else
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Animation>& asset, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			package->createAsset(asset, packagePath);
		else
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Material>& asset, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			return package->createAsset(asset, packagePath);
		else
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			return package->createAsset(asset, packagePath);
		else
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)path.u8string().c_str() + " failed.");
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
			if (it.expired())
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
	AssetDatabase::getAbsolutePath(const std::filesystem::path& path) const noexcept
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			return package->getAbsolutePath(packagePath);

		return std::filesystem::path();
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

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::shared_ptr<const Object>& asset) const noexcept
	{
		for (auto& package : packages_)
		{
			auto path = package.second->getAssetPath(asset);
			if (!path.empty())
				return path;
		}

		return std::filesystem::path();
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

	std::shared_ptr<Package>
	AssetDatabase::getPackage(const std::filesystem::path& assetPath, std::filesystem::path& packagePath) const noexcept(false)
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

	bool
	AssetDatabase::contains(const std::shared_ptr<const Object>& asset) const noexcept
	{
		for (auto& package : packages_)
		{
			if (package.second->contains(asset))
				return true;
		}

		return false;
	}

	std::shared_ptr<Object>
	AssetDatabase::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		std::filesystem::path packagePath;
		auto package = this->getPackage(path, packagePath);
		if (package)
			return package->loadAssetAtPath(packagePath);
		return nullptr;
	}

	void
	AssetDatabase::setDirty(const std::shared_ptr<Object>& object, bool dirty) noexcept(false)
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

	void
	AssetDatabase::clearUpdate() noexcept
	{
		this->dirtyList_.clear();
	}
}