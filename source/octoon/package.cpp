#include <octoon/package.h>
#include <octoon/runtime/md5.h>
#include <octoon/runtime/guid.h>
#include <octoon/pmx.h>
#include <octoon/vmd_importer.h>
#include <octoon/fbx_importer.h>
#include <octoon/texture_importer.h>
#include <octoon/asset_database.h>
#include <octoon/mesh_animation_component.h>

#include <fstream>

namespace octoon
{
	Package::Package(AssetDatabase* assetDatabase) noexcept
		: assetDatabase_(assetDatabase)
	{
	}

	Package::~Package() noexcept
	{
		this->close();
	}

	void
	Package::open(const std::filesystem::path& diskPath) noexcept(false)
	{
		this->close();

		this->rootPath_ = diskPath;

		std::ifstream ifs(std::filesystem::path(diskPath).append("manifest.json"), std::ios_base::binary);
		if (ifs)
		{
			auto assetDb = nlohmann::json::parse(ifs);

			for (auto it = assetDb.begin(); it != assetDb.end(); ++it)
			{
				auto uuid = it.value();
				auto path = std::filesystem::path((char8_t*)it.key().c_str());

				this->paths_[path] = uuid;
				this->uniques_[uuid] = path;
			}
		}
	}

	void
	Package::close() noexcept
	{
		rootPath_.clear();
		paths_.clear();
		uniques_.clear();
	}

	void
	Package::importAsset(const std::filesystem::path& diskPath, const std::filesystem::path& relativePath) noexcept(false)
	{
		assert(!diskPath.empty());

		if (relativePath.empty())
			return;

		if (diskPath.empty() || !std::filesystem::exists(diskPath))
			return;

		auto rootPath = relativePath.parent_path();
		auto absolutePath = std::filesystem::path(this->rootPath_).append(relativePath.wstring());

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
	Package::createAsset(const std::shared_ptr<const Texture>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (assetDatabase_->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			auto absolutePath = std::filesystem::path(this->rootPath_).append(relativePath.wstring());
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
	Package::createAsset(const std::shared_ptr<const Animation>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (assetDatabase_->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			std::ofstream stream(std::filesystem::path(this->rootPath_).append(relativePath.wstring()), io::ios_base::binary);
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
	Package::createAsset(const std::shared_ptr<const Material>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (assetDatabase_->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			std::ofstream ifs(std::filesystem::path(this->rootPath_).append(relativePath.wstring()), std::ios_base::binary);
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
							if (!assetDatabase_->contains(texture))
							{
								auto texturePath = std::filesystem::path("Assets/Textures").append(make_guid() + ".png");
								this->createFolder(std::filesystem::path("Assets/Textures"));
								this->createAsset(texture, texturePath);
								mat[it.key] = this->getAssetGuid(texturePath);
							}
							else
							{
								mat[it.key] = this->getAssetGuid(texture);
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
	Package::createAsset(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (assetDatabase_->contains(asset))
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			std::ofstream ifs(std::filesystem::path(this->rootPath_).append(relativePath.wstring()), std::ios_base::binary);
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
						prefab["model"] = this->getAssetGuid(outputPath);
					}
					else
					{
						prefab["model"] = this->getAssetGuid(modelPath);
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
	Package::createPrefab(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty())
			throw std::runtime_error(std::string("Creating prefab at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		if (assetDatabase_->contains(asset) && assetDatabase_->isPartOfPrefabAsset(asset))
			throw std::runtime_error(std::string("Creating prefab at path ") + (char*)relativePath.u8string().c_str() + " failed.");

		try
		{
			std::ofstream ifs(std::filesystem::path(this->rootPath_).append(relativePath.wstring()), std::ios_base::binary);
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

	std::filesystem::path
	Package::getAssetPath(const std::string& uuid) const noexcept
	{
		auto item = uniques_.find(uuid);
		if (item != uniques_.end())
			return item->second;

		return std::filesystem::path();
	}

	std::filesystem::path
	Package::getAbsolutePath(const std::filesystem::path& assetPath) const noexcept
	{
		return std::filesystem::path(this->rootPath_).append(assetPath.wstring());
	}

	std::string
	Package::getAssetGuid(const std::filesystem::path& relativePath) const noexcept
	{
		if (!relativePath.empty())
		{
			auto it = paths_.find(relativePath);
			if (it != paths_.end())
				return (*it).second;
		}

		return std::string();
	}

	std::string
	Package::getAssetGuid(const std::shared_ptr<const Object>& asset) const noexcept
	{
		auto path = assetDatabase_->getAssetPath(asset);
		if (!path.empty())
			return this->getAssetGuid(path);
		return std::string();
	}

	void
	Package::deleteAsset(const std::filesystem::path& relativePath) noexcept(false)
	{
		auto absolutePath = std::filesystem::path(this->rootPath_).append(relativePath.wstring());
		if (std::filesystem::is_directory(absolutePath))
		{
			std::filesystem::remove_all(absolutePath);
			std::filesystem::remove(absolutePath.concat(".meta"));
		}
		else
		{
			auto uuid = this->getAssetGuid(relativePath);
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
	Package::saveAssets() noexcept(false)
	{
		std::ofstream ifs(std::filesystem::path(rootPath_).append("manifest.json"), std::ios_base::binary);
		if (ifs)
		{
			nlohmann::json assetDb;

			for (auto& it : paths_)
			{
				auto path = it.first.u8string();
				if (std::filesystem::exists(std::filesystem::path(this->rootPath_).append(path)))
					assetDb[(char*)path.c_str()] = it.second;
			}

			auto dump = assetDb.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}
	}

	void
	Package::createFolder(const std::filesystem::path& assetFolder_) noexcept(false)
	{
		if (!assetFolder_.empty())
		{
			auto relativePath = std::filesystem::path(assetFolder_).make_preferred();
			auto absolutePath = std::filesystem::path(this->rootPath_).append(relativePath.wstring());
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
	Package::deleteFolder(const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!relativePath.empty())
		{
			auto folderPath = std::filesystem::path(this->rootPath_).append(relativePath.wstring());
			if (std::filesystem::exists(folderPath))
			{
				std::filesystem::remove_all(folderPath);
				this->removeMetadataAtPath(relativePath);
			}
		}
		else
		{
			throw std::runtime_error(std::string("Creating asset at path ") + (char*)relativePath.u8string().c_str() + " failed.");
		}
	}

	void
	Package::createMetadataAtPath(const std::filesystem::path& relativePath) noexcept(false)
	{
		nlohmann::json metadata;
		metadata["uuid"] = MD5(std::filesystem::path(relativePath).make_preferred().u8string()).toString();

		this->createMetadataAtPath(relativePath, metadata);
	}

	void
	Package::createMetadataAtPath(const std::filesystem::path& path, const nlohmann::json& json) noexcept(false)
	{
		std::ofstream ifs(std::filesystem::path(this->rootPath_).append(path.wstring()).concat(L".meta"), std::ios_base::binary);
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
	Package::removeMetadataAtPath(const std::filesystem::path& relativePath_) noexcept
	{
		auto relativePath = std::filesystem::path(relativePath_).make_preferred();
		auto uuid = this->getAssetGuid(relativePath);
		paths_.erase(paths_.find(relativePath));
		uniques_.erase(uniques_.find(uuid));

		auto metaPath = std::filesystem::path(this->rootPath_).append(relativePath.wstring()).concat(L".meta");
		if (std::filesystem::exists(metaPath))
			std::filesystem::remove(metaPath);
	}

	nlohmann::json
	Package::loadMetadataAtPath(const std::filesystem::path& relativePath) noexcept(false)
	{
		std::ifstream ifs(std::filesystem::path(this->rootPath_).append(relativePath.wstring()).concat(L".meta"));
		if (ifs)
		{
			auto metaData = nlohmann::json::parse(ifs);

			if (metaData.contains("uuid"))
			{
				auto guid = metaData["uuid"].get<std::string>();
				paths_[relativePath] = guid;
				uniques_[guid] = relativePath;

				return metaData;
			}
		}

		return nlohmann::json();
	}

	std::shared_ptr<Object>
	Package::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		auto assetImporter = AssetImporter::getAtPath(this->getAbsolutePath(path));
		if (assetImporter)
		{
			auto asset = assetImporter->importer();
			if (asset)
			{
				auto metadata = this->loadMetadataAtPath(path);
				if (metadata.is_object())
				{
					if (metadata.contains("uuid"))
					{
						auto guid = metadata["uuid"].get<std::string>();
						paths_[path] = guid;
						uniques_[guid] = path;
					}
				}
				else
				{
					if (!path.is_absolute())
						this->createMetadataAtPath(path);
				}
			}

			return asset;
		}

		return nullptr;
	}
}