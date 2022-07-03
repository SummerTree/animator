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
		: defaultPackage_(std::make_shared<AssetPipeline>(u8""))
	{
	}

	AssetDatabase::~AssetDatabase() noexcept
	{
		assetCaches_.clear();
		paths_.clear();
		uniques_.clear();
		this->packages_.clear();
	}

	void
	AssetDatabase::mountPackage(const std::u8string& name, const std::filesystem::path& diskPath) noexcept(false)
	{
		if (!this->packages_.contains(name))
		{
			auto package = std::make_shared<AssetPipeline>(name);
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
	AssetDatabase::importAsset(const std::filesystem::path& relativePath) noexcept(false)
	{
		auto metadata = this->loadMetadataAtPath(relativePath);
		if (metadata.is_object())
		{
			if (metadata.contains("uuid"))
			{
				auto guid = metadata["uuid"].get<std::string>();
				paths_[relativePath] = guid;
				uniques_[guid] = relativePath;
			}
		}
		else
		{
			if (!relativePath.is_absolute())
				this->createMetadataAtPath(relativePath);
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

			this->createMetadataAtPath(relativePath);

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

		try
		{
			auto absolutePath = this->getAbsolutePath(relativePath);
			auto extension = absolutePath.extension().u8string();

			if (asset->save(absolutePath, (char*)extension.substr(1).c_str()))
			{
				auto uuid = MD5(std::filesystem::path(relativePath).make_preferred().u8string()).toString();

				nlohmann::json metadata;
				metadata["uuid"] = uuid;
				metadata["name"] = asset->getName();
				metadata["suffix"] = (char*)extension.c_str();
				metadata["mipmap"] = asset->getMipLevel();

				std::ofstream ifs(absolutePath.u8string() + u8".meta", std::ios_base::binary);
				if (ifs)
				{
					auto dump = metadata.dump();
					ifs.write(dump.c_str(), dump.size());
					ifs.close();
				}

				paths_[relativePath] = uuid;
				uniques_[uuid] = relativePath;
			}
			else
			{
				throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");
			}
		}
		catch (const std::exception& e)
		{
			this->deleteAsset(relativePath);
			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Animation>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			std::ofstream stream(this->getAbsolutePath(relativePath), io::ios_base::binary);
			if (stream)
			{
				VMDImporter::save(stream, *asset);
				this->createMetadataAtPath(relativePath);
			}
			else
			{
				throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");
			}
		}
		catch (const std::exception& e)
		{
			this->deleteAsset(relativePath);
			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Material>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			std::ofstream ifs(this->getAbsolutePath(relativePath), std::ios_base::binary);
			if (ifs)
			{
				nlohmann::json mat;
				mat["type"] = asset->type_name();
				mat["name"] = asset->getName();
				mat["blendEnable"] = asset->getBlendEnable();
				mat["blendOp"] = asset->getBlendOp();
				mat["blendSrc"] = asset->getBlendSrc();
				mat["blendDest"] = asset->getBlendDest();
				mat["blendAlphaOp"] = asset->getBlendAlphaOp();
				mat["blendAlphaSrc"] = asset->getBlendAlphaSrc();
				mat["blendAlphaDest"] = asset->getBlendAlphaDest();
				mat["colorWriteMask"] = asset->getColorWriteMask();
				mat["depthEnable"] = asset->getDepthEnable();
				mat["depthBiasEnable"] = asset->getDepthBiasEnable();
				mat["depthBoundsEnable"] = asset->getDepthBoundsEnable();
				mat["depthClampEnable"] = asset->getDepthClampEnable();
				mat["depthWriteEnable"] = asset->getDepthWriteEnable();
				mat["depthMin"] = asset->getDepthMin();
				mat["depthMax"] = asset->getDepthMax();
				mat["depthBias"] = asset->getDepthBias();
				mat["depthSlopeScaleBias"] = asset->getDepthSlopeScaleBias();
				mat["stencilEnable"] = asset->getStencilEnable();
				mat["scissorTestEnable"] = asset->getScissorTestEnable();

				for (auto& it : asset->getMaterialParams())
				{
					switch (it.type)
					{
					case PropertyTypeInfo::PropertyTypeInfoFloat:
						mat[it.key] = asset->get<math::float1>(it.key);
						break;
					case PropertyTypeInfo::PropertyTypeInfoFloat2:
						mat[it.key] = asset->get<math::float2>(it.key).to_array();
						break;
					case PropertyTypeInfo::PropertyTypeInfoFloat3:
						mat[it.key] = asset->get<math::float3>(it.key).to_array();
						break;
					case PropertyTypeInfo::PropertyTypeInfoFloat4:
						mat[it.key] = asset->get<math::float4>(it.key).to_array();
						break;
					case PropertyTypeInfo::PropertyTypeInfoString:
						mat[it.key] = asset->get<std::string>(it.key);
						break;
					case PropertyTypeInfo::PropertyTypeInfoBool:
						mat[it.key] = asset->get<bool>(it.key);
						break;
					case PropertyTypeInfo::PropertyTypeInfoInt:
						mat[it.key] = asset->get<int>(it.key);
						break;
					case PropertyTypeInfo::PropertyTypeInfoTexture:
					{
						auto texture = asset->get<std::shared_ptr<Texture>>(it.key);
						if (texture)
						{
							if (!AssetDatabase::instance()->contains(texture))
							{
								auto texturePath = std::filesystem::path("Assets/Textures").append(make_guid() + ".png");
								this->createFolder(std::filesystem::path("Assets/Textures"));
								this->createAsset(texture, texturePath);
								mat[it.key] = AssetDatabase::instance()->getAssetGuid(texturePath);
							}
							else
							{
								mat[it.key] = AssetDatabase::instance()->getAssetGuid(texture);
							}
						}
					}
					break;
					default:
						break;
					}
				}

				auto dump = mat.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();

				this->createMetadataAtPath(relativePath);
			}
			else
			{
				throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");
			}
		}
		catch (const std::exception& e)
		{
			this->deleteAsset(relativePath);
			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			std::ofstream ifs(this->getAbsolutePath(relativePath), std::ios_base::binary);
			if (ifs)
			{
				nlohmann::json prefab;

				auto modelPath = AssetDatabase::instance()->getAssetPath(asset);
				if (!modelPath.empty())
				{
					if (modelPath.is_absolute())
					{
						auto outputPath = std::filesystem::path("Assets/Models").append(octoon::make_guid()).append(modelPath.filename().wstring());
						this->importAsset(modelPath, outputPath);
						prefab["model"] = AssetDatabase::instance()->getAssetGuid(outputPath);
					}
					else
					{
						prefab["model"] = AssetDatabase::instance()->getAssetGuid(modelPath);
					}
				}

				asset->save(prefab);

				auto dump = prefab.dump();
				ifs.write(dump.c_str(), dump.size());

				this->createMetadataAtPath(relativePath);
			}
			else
			{
				throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");
			}
		}
		catch (const std::exception& e)
		{
			this->deleteAsset(relativePath);
			throw e;
		}
	}

	void
	AssetDatabase::createPrefab(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating prefab at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (AssetDatabase::instance()->contains(asset) && AssetDatabase::instance()->isPartOfPrefabAsset(asset))
			throw std::runtime_error(std::string("Creating prefab at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			std::ofstream ifs(this->getAbsolutePath(relativePath), std::ios_base::binary);
			if (ifs)
			{
				nlohmann::json prefab;
				asset->save(prefab);

				auto dump = prefab.dump();
				ifs.write(dump.c_str(), dump.size());

				this->createMetadataAtPath(relativePath);
			}
			else
			{
				throw std::runtime_error(std::string("Creating prefab at path ") + (char*)relativePath.u8string().c_str() + " failed.");
			}
		}
		catch (const std::exception& e)
		{
			this->deleteAsset(relativePath);
			throw e;
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
		auto absolutePath = this->getAbsolutePath(relativePath);
		if (std::filesystem::is_directory(absolutePath))
		{
			std::filesystem::remove_all(absolutePath);
			std::filesystem::remove(absolutePath.concat(".meta"));
		}
		else
		{
			auto uuid = AssetDatabase::instance()->getAssetGuid(relativePath);
			if (!uuid.empty())
			{
				paths_.erase(paths_.find(relativePath));
				uniques_.erase(uniques_.find(uuid));
			}

			if (std::filesystem::exists(absolutePath))
				std::filesystem::remove(absolutePath);

			auto metadata = std::filesystem::path(absolutePath).concat(L".meta");
			if (std::filesystem::exists(metadata))
				std::filesystem::remove(metadata);
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

		for (auto& package : packages_)
			package.second->saveAssets();
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::string& uuid) const noexcept
	{
		auto item = uniques_.find(uuid);
		if (item != uniques_.end())
			return item->second;

		return std::filesystem::path();
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::shared_ptr<const Object>& object) const noexcept
	{
		return AssetManager::instance()->getAssetPath(object);
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
		auto item = paths_.find(path);
		if (item != paths_.end())
			return item->second;

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
	AssetDatabase::createFolder(const std::filesystem::path& assetFolder_) noexcept(false)
	{
		if (!assetFolder_.empty())
		{
			auto relativePath = std::filesystem::path(assetFolder_).make_preferred();
			auto absolutePath = this->getAbsolutePath(relativePath);
			auto path = relativePath.wstring();

			std::filesystem::create_directories(std::filesystem::path(absolutePath));

			auto offset = path.find_first_of(std::filesystem::path::preferred_separator);
			if (offset > 0 && offset < path.size())
			{
				auto folder = path.substr(0, offset);
				if (folder == L"Assets")
					offset = path.find_first_of(std::filesystem::path::preferred_separator, offset + 1);
				else if (folder == L"Packages")
				{
					offset = path.find_first_of(std::filesystem::path::preferred_separator, offset + 1);
					offset = path.find_first_of(std::filesystem::path::preferred_separator, offset + 1);
				}
			}

			while (offset > 0 && offset < path.size())
			{
				this->createMetadataAtPath(path.substr(0, offset));
				offset = path.find_first_of(std::filesystem::path::preferred_separator, offset + 1);
			}

			this->createMetadataAtPath(assetFolder_);
		}
		else
		{
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)assetFolder_.u8string().c_str() + " failed.");
		}
	}

	void
	AssetDatabase::deleteFolder(const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!relativePath.empty())
		{
			auto folderPath = AssetDatabase::instance()->getAbsolutePath(relativePath);
			if (std::filesystem::exists(folderPath))
			{
				std::filesystem::remove_all(folderPath);
				AssetDatabase::instance()->removeMetadataAtPath(relativePath);
			}
		}
		else
		{
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");
		}
	}

	std::shared_ptr<AssetPipeline>
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
			auto metaData = this->loadMetadataAtPath(assetPath);
			if (metaData.is_object())
			{
				metaData["label"] = labels;
				this->createMetadataAtPath(assetPath, metaData);
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
			auto metaData = this->loadMetadataAtPath(assetPath);
			if (metaData.is_object())
			{
				metaData["label"] = labels;
				this->createMetadataAtPath(assetPath, metaData);
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

			auto metaData = this->loadMetadataAtPath(assetPath);
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

	void
	AssetDatabase::createMetadataAtPath(const std::filesystem::path& relativePath) noexcept(false)
	{
		nlohmann::json metadata;
		metadata["uuid"] = MD5(std::filesystem::path(relativePath).make_preferred().u8string()).toString();

		this->createMetadataAtPath(relativePath, metadata);
	}

	void
	AssetDatabase::createMetadataAtPath(const std::filesystem::path& path, const nlohmann::json& json) noexcept(false)
	{
		std::ofstream ifs(AssetDatabase::instance()->getAbsolutePath(path).concat(L".meta"), std::ios_base::binary);
		if (ifs)
		{
			auto uuid = json["uuid"].get<std::string>();

			paths_[path] = uuid;
			uniques_[uuid] = path;

			auto dump = json.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}
		else
		{
			throw std::runtime_error(std::string("Creating metadata at path ") + (char*)path.u8string().c_str() + " failed.");
		}
	}

	void
	AssetDatabase::removeMetadataAtPath(const std::filesystem::path& relativePath_) noexcept
	{
		auto relativePath = std::filesystem::path(relativePath_).make_preferred();
		auto uuid = this->getAssetGuid(relativePath);
		paths_.erase(paths_.find(relativePath));
		uniques_.erase(uniques_.find(uuid));

		auto metaPath = AssetDatabase::instance()->getAbsolutePath(relativePath).concat(L".meta");
		if (std::filesystem::exists(metaPath))
			std::filesystem::remove(metaPath);
	}

	nlohmann::json
	AssetDatabase::loadMetadataAtPath(const std::filesystem::path& relativePath) noexcept(false)
	{
		std::ifstream ifs(AssetDatabase::instance()->getAbsolutePath(relativePath).concat(L".meta"));
		if (ifs)
		{
			auto metaData = nlohmann::json::parse(ifs);
			return metaData;
		}

		return nlohmann::json();
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

			std::filesystem::path packagePath;
			auto package = this->getPackage(path, packagePath);
			if (package)
			{
				this->importAsset(path);

				auto asset = package->loadAssetAtPath(path);
				if (asset)
					assetCaches_[path] = asset;

				return asset;
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