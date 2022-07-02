#include <octoon/package.h>
#include <octoon/runtime/md5.h>
#include <octoon/runtime/guid.h>
#include <octoon/vmd_loader.h>
#include <octoon/pmx_loader.h>
#include <octoon/obj_loader.h>
#include <octoon/ass_loader.h>
#include <octoon/fbx_loader.h>
#include <octoon/asset_importer.h>
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
		objectCaches_.clear();
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
				auto dependencies = octoon::FBXLoader::getDependencies(diskPath);

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
				VMDLoader::save(stream, *asset);
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

				auto modelPath = AssetImporter::instance()->getAssetPath(asset);
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
	Package::loadAssetAtPath(const std::filesystem::path& relativePath) noexcept(false)
	{
		if (objectCaches_.contains(relativePath))
		{
			auto cache = objectCaches_.at(relativePath);
			if (!cache.expired())
				return cache.lock();
		}

		auto absolutePath = std::filesystem::path(this->rootPath_).append(relativePath.wstring());
		auto ext = absolutePath.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == u8".vmd")
		{
			auto motion = AssetImporter::instance()->loadAssetAtPath<Animation>(absolutePath);
			if (motion)
			{
				if (motion->getName().empty())
					motion->setName((char*)relativePath.filename().c_str());

				auto metadata = this->loadMetadataAtPath(relativePath);
				if (!metadata.is_object())
					this->createMetadataAtPath(relativePath);

				objectCaches_[relativePath] = motion;

				return motion;
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = AssetImporter::instance()->loadAssetAtPath<Texture>(absolutePath);
			if (texture)
			{
				auto metadata = this->loadMetadataAtPath(relativePath);
				if (metadata.is_object())
				{
					if (metadata.contains("mipmap"))
						texture->setMipLevel(metadata["mipmap"].get<nlohmann::json::number_integer_t>());
				}
				else
				{
					if (ext == u8".hdr")
						texture->setMipLevel(8);

					this->createMetadataAtPath(relativePath);
				}

				texture->apply();

				objectCaches_[relativePath] = texture;

				return texture;
			}
		}
		else if (ext == u8".pmx" || ext == u8".obj" || ext == u8".fbx")
		{
			auto model = AssetImporter::instance()->loadAssetAtPath<GameObject>(absolutePath);
			if (model)
			{
				auto metadata = this->loadMetadataAtPath(relativePath);
				if (!metadata.is_object())
					this->createMetadataAtPath(relativePath);

				objectCaches_[relativePath] = model;

				return model;
			}
		}
		else if (ext == u8".abc")
		{
			auto model = AssetImporter::instance()->loadAssetAtPath<GameObject>(absolutePath);
			if (model)
			{
				auto alembic = model->addComponent<MeshAnimationComponent>();
				alembic->setFilePath(relativePath);

				auto metadata = this->loadMetadataAtPath(relativePath);
				if (!metadata.is_object())
					this->createMetadataAtPath(relativePath);

				objectCaches_[relativePath] = model;

				return model;
			}
		}
		else if (ext == u8".mat")
		{
			std::ifstream ifs(absolutePath);
			if (ifs)
			{
				auto mat = nlohmann::json::parse(ifs);

				auto material = std::make_shared<MeshStandardMaterial>();
				auto name = mat.find("name");
				auto colorMap = mat.find("colorMap");
				auto opacityMap = mat.find("opacityMap");
				auto normalMap = mat.find("normalMap");
				auto roughnessMap = mat.find("roughnessMap");
				auto specularMap = mat.find("specularMap");
				auto metalnessMap = mat.find("metalnessMap");
				auto emissiveMap = mat.find("emissiveMap");
				auto anisotropyMap = mat.find("anisotropyMap");
				auto clearCoatMap = mat.find("clearCoatMap");
				auto clearCoatRoughnessMap = mat.find("clearCoatRoughnessMap");
				auto subsurfaceMap = mat.find("subsurfaceMap");
				auto subsurfaceColorMap = mat.find("subsurfaceColorMap");
				auto sheenMap = mat.find("sheenMap");
				auto lightMap = mat.find("lightMap");

				if (name != mat.end() && (*name).is_string())
					material->setName((*name).get<std::string>());
				if (colorMap != mat.end() && (*colorMap).is_string())
					material->setColorMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*colorMap)));
				if (opacityMap != mat.end() && (*opacityMap).is_string())
					material->setOpacityMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*opacityMap)));
				if (normalMap != mat.end() && (*normalMap).is_string())
					material->setNormalMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*normalMap)));
				if (roughnessMap != mat.end() && (*roughnessMap).is_string())
					material->setRoughnessMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*roughnessMap)));
				if (specularMap != mat.end() && (*specularMap).is_string())
					material->setSpecularMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*specularMap)));
				if (metalnessMap != mat.end() && (*metalnessMap).is_string())
					material->setMetalnessMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*metalnessMap)));
				if (emissiveMap != mat.end() && (*emissiveMap).is_string())
					material->setEmissiveMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*emissiveMap)));
				if (anisotropyMap != mat.end() && (*anisotropyMap).is_string())
					material->setAnisotropyMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*anisotropyMap)));
				if (clearCoatMap != mat.end() && (*clearCoatMap).is_string())
					material->setClearCoatMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*clearCoatMap)));
				if (clearCoatRoughnessMap != mat.end() && (*clearCoatRoughnessMap).is_string())
					material->setClearCoatRoughnessMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*clearCoatRoughnessMap)));
				if (subsurfaceMap != mat.end() && (*subsurfaceMap).is_string())
					material->setSubsurfaceMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*subsurfaceMap)));
				if (subsurfaceColorMap != mat.end() && (*subsurfaceColorMap).is_string())
					material->setSubsurfaceColorMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*subsurfaceColorMap)));
				if (sheenMap != mat.end() && (*sheenMap).is_string())
					material->setSheenMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*sheenMap)));
				if (lightMap != mat.end() && (*lightMap).is_string())
					material->setLightMap(this->loadAssetAtPath<Texture>(this->getAssetPath(*lightMap)));

				auto blendEnable = mat.find("blendEnable");
				auto blendOp = mat.find("blendOp");
				auto blendSrc = mat.find("blendSrc");
				auto blendDest = mat.find("blendDest");
				auto blendAlphaOp = mat.find("blendAlphaOp");
				auto blendAlphaSrc = mat.find("blendAlphaSrc");
				auto blendAlphaDest = mat.find("blendAlphaDest");

				if (blendEnable != mat.end() && (*blendEnable).is_boolean())
					material->setBlendEnable((*blendEnable).get<nlohmann::json::boolean_t>());
				if (blendOp != mat.end() && (*blendOp).is_number_unsigned())
					material->setBlendOp((BlendOp)(*blendOp).get<nlohmann::json::number_unsigned_t>());
				if (blendSrc != mat.end() && (*blendSrc).is_number_unsigned())
					material->setBlendSrc((BlendMode)(*blendSrc).get<nlohmann::json::number_unsigned_t>());
				if (blendDest != mat.end() && (*blendDest).is_number_unsigned())
					material->setBlendDest((BlendMode)(*blendDest).get<nlohmann::json::number_unsigned_t>());
				if (blendAlphaOp != mat.end() && (*blendAlphaOp).is_number_unsigned())
					material->setBlendAlphaOp((BlendOp)(*blendAlphaOp).get<nlohmann::json::number_unsigned_t>());
				if (blendAlphaSrc != mat.end() && (*blendAlphaSrc).is_number_unsigned())
					material->setBlendAlphaSrc((BlendMode)(*blendAlphaSrc).get<nlohmann::json::number_unsigned_t>());
				if (blendAlphaDest != mat.end() && (*blendAlphaDest).is_number_unsigned())
					material->setBlendAlphaDest((BlendMode)(*blendAlphaDest).get<nlohmann::json::number_unsigned_t>());

				auto depthEnable = mat.find("depthEnable");
				auto depthBiasEnable = mat.find("depthBiasEnable");
				auto depthBoundsEnable = mat.find("depthBoundsEnable");
				auto depthClampEnable = mat.find("depthClampEnable");
				auto depthWriteEnable = mat.find("depthWriteEnable");
				auto stencilEnable = mat.find("stencilEnable");
				auto scissorTestEnable = mat.find("scissorTestEnable");

				if (depthEnable != mat.end() && (*depthEnable).is_boolean())
					material->setDepthEnable((*depthEnable).get<nlohmann::json::boolean_t>());
				if (depthBiasEnable != mat.end() && (*depthBiasEnable).is_boolean())
					material->setDepthBiasEnable((*depthBiasEnable).get<nlohmann::json::boolean_t>());
				if (depthBoundsEnable != mat.end() && (*depthBoundsEnable).is_boolean())
					material->setDepthBoundsEnable((*depthBoundsEnable).get<nlohmann::json::boolean_t>());
				if (depthClampEnable != mat.end() && (*depthClampEnable).is_boolean())
					material->setDepthClampEnable((*depthClampEnable).get<nlohmann::json::boolean_t>());
				if (depthWriteEnable != mat.end() && (*depthWriteEnable).is_boolean())
					material->setDepthWriteEnable((*depthWriteEnable).get<nlohmann::json::boolean_t>());
				if (stencilEnable != mat.end() && (*stencilEnable).is_boolean())
					material->setStencilEnable((*stencilEnable).get<nlohmann::json::boolean_t>());
				if (scissorTestEnable != mat.end() && (*scissorTestEnable).is_boolean())
					material->setScissorTestEnable((*scissorTestEnable).get<nlohmann::json::boolean_t>());

				auto emissiveIntensity = mat.find("emissiveIntensity");
				auto opacity = mat.find("opacity");
				auto smoothness = mat.find("smoothness");
				auto roughness = mat.find("roughness");
				auto metalness = mat.find("metalness");
				auto anisotropy = mat.find("anisotropy");
				auto sheen = mat.find("sheen");
				auto specular = mat.find("specular");
				auto refractionRatio = mat.find("refractionRatio");
				auto clearCoat = mat.find("clearCoat");
				auto clearCoatRoughness = mat.find("clearCoatRoughness");
				auto subsurface = mat.find("subsurface");
				auto reflectionRatio = mat.find("reflectionRatio");
				auto transmission = mat.find("transmission");
				auto lightMapIntensity = mat.find("lightMapIntensity");
				auto gamma = mat.find("gamma");
				auto depthMin = mat.find("depthMin");
				auto depthMax = mat.find("depthMax");
				auto depthBias = mat.find("depthBias");
				auto depthSlopeScaleBias = mat.find("depthSlopeScaleBias");

				if (emissiveIntensity != mat.end() && (*emissiveIntensity).is_number_float())
					material->setEmissiveIntensity((*emissiveIntensity).get<nlohmann::json::number_float_t>());
				if (opacity != mat.end() && (*opacity).is_number_float())
					material->setOpacity((*opacity).get<nlohmann::json::number_float_t>());
				if (smoothness != mat.end() && (*smoothness).is_number_float())
					material->setSmoothness((*smoothness).get<nlohmann::json::number_float_t>());
				if (roughness != mat.end() && (*roughness).is_number_float())
					material->setRoughness((*roughness).get<nlohmann::json::number_float_t>());
				if (metalness != mat.end() && (*metalness).is_number_float())
					material->setMetalness((*metalness).get<nlohmann::json::number_float_t>());
				if (anisotropy != mat.end() && (*anisotropy).is_number_float())
					material->setAnisotropy((*anisotropy).get<nlohmann::json::number_float_t>());
				if (sheen != mat.end() && (*sheen).is_number_float())
					material->setSheen((*sheen).get<nlohmann::json::number_float_t>());
				if (specular != mat.end() && (*specular).is_number_float())
					material->setSpecular((*specular).get<nlohmann::json::number_float_t>());
				if (refractionRatio != mat.end() && (*refractionRatio).is_number_float())
					material->setRefractionRatio((*refractionRatio).get<nlohmann::json::number_float_t>());
				if (clearCoat != mat.end() && (*clearCoat).is_number_float())
					material->setClearCoat((*clearCoat).get<nlohmann::json::number_float_t>());
				if (clearCoatRoughness != mat.end() && (*clearCoatRoughness).is_number_float())
					material->setClearCoatRoughness((*clearCoatRoughness).get<nlohmann::json::number_float_t>());
				if (subsurface != mat.end() && (*subsurface).is_number_float())
					material->setSubsurface((*subsurface).get<nlohmann::json::number_float_t>());
				if (reflectionRatio != mat.end() && (*reflectionRatio).is_number_float())
					material->setReflectionRatio((*reflectionRatio).get<nlohmann::json::number_float_t>());
				if (transmission != mat.end() && (*transmission).is_number_float())
					material->setTransmission((*transmission).get<nlohmann::json::number_float_t>());
				if (lightMapIntensity != mat.end() && (*lightMapIntensity).is_number_float())
					material->setLightMapIntensity((*lightMapIntensity).get<nlohmann::json::number_float_t>());
				if (gamma != mat.end() && (*gamma).is_number_float())
					material->setGamma((*gamma).get<nlohmann::json::number_float_t>());
				if (depthMin != mat.end() && (*depthMin).is_number_float())
					material->setDepthMin((*depthMin).get<nlohmann::json::number_float_t>());
				if (depthMax != mat.end() && (*depthMax).is_number_float())
					material->setDepthMax((*depthMax).get<nlohmann::json::number_float_t>());
				if (depthBias != mat.end() && (*depthBias).is_number_float())
					material->setDepthBias((*depthBias).get<nlohmann::json::number_float_t>());
				if (depthSlopeScaleBias != mat.end() && (*depthSlopeScaleBias).is_number_float())
					material->setDepthSlopeScaleBias((*depthSlopeScaleBias).get<nlohmann::json::number_float_t>());

				auto offset = mat.find("offset");
				auto repeat = mat.find("repeat");
				auto normalScale = mat.find("normalScale");
				auto color = mat.find("color");
				auto emissive = mat.find("emissive");
				auto subsurfaceColor = mat.find("subsurfaceColor");

				if (offset != mat.end() && (*offset).is_array())
					material->setOffset(math::float2((*offset).get<std::array<float, 2>>()));
				if (repeat != mat.end() && (*repeat).is_array())
					material->setRepeat(math::float2((*repeat).get<std::array<float, 2>>()));
				if (normalScale != mat.end() && (*normalScale).is_array())
					material->setNormalScale(math::float2((*normalScale).get<std::array<float, 2>>()));
				if (color != mat.end() && (*color).is_array())
					material->setColor(math::float3((*color).get<std::array<float, 3>>()));
				if (emissive != mat.end() && (*emissive).is_array())
					material->setEmissive(math::float3((*emissive).get<std::array<float, 3>>()));
				if (subsurfaceColor != mat.end() && (*subsurfaceColor).is_array())
					material->setSubsurfaceColor(math::float3((*subsurfaceColor).get<std::array<float, 3>>()));

				auto metadata = this->loadMetadataAtPath(relativePath);
				if (!metadata.is_object())
					this->createMetadataAtPath(relativePath);

				objectCaches_[relativePath] = material;

				return std::move(material);
			}
		}
		else if (ext == u8".prefab")
		{
			std::ifstream ifs(absolutePath);
			if (ifs)
			{
				auto prefab = nlohmann::json::parse(ifs);
				auto object = std::make_shared<GameObject>();
				object->load(prefab);

				auto metadata = this->loadMetadataAtPath(relativePath);
				if (!metadata.is_object())
					this->createMetadataAtPath(relativePath);

				objectCaches_[relativePath] = object;

				AssetImporter::instance()->unload();

				return object;
			}
		}

		return nullptr;
	}
}