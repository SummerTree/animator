#include <octoon/asset_database.h>
#include <octoon/asset_bundle.h>
#include <octoon/asset_loader.h>
#include <octoon/asset_preview.h>
#include <octoon/runtime/guid.h>
#include <octoon/runtime/md5.h>
#include <octoon/runtime/string.h>
#include <octoon/vmd_loader.h>
#include <octoon/pmx_loader.h>
#include <octoon/obj_loader.h>
#include <octoon/ass_loader.h>
#include <octoon/fbx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/animation/animation.h>
#include <octoon/mesh_animation_component.h>
#include <octoon/PMREM_loader.h>
#include <octoon/video/renderer.h>
#include <octoon/mesh/sphere_mesh.h>
#include <octoon/environment_light_component.h>
#include <octoon/mesh_renderer_component.h>
#include <octoon/skinned_mesh_renderer_component.h>
#include <octoon/animator_component.h>
#include <octoon/transform_component.h>

#include <fstream>
#include <filesystem>
#include <codecvt>

namespace octoon
{
	OctoonImplementSingleton(AssetDatabase)

	AssetDatabase::AssetDatabase() noexcept
	{
	}

	AssetDatabase::~AssetDatabase() noexcept
	{
		this->close();
	}

	void
	AssetDatabase::open(const std::filesystem::path& assetPath) noexcept(false)
	{
		this->close();

		this->assetPath_ = assetPath;
		this->assetPath_.make_preferred();

		if (!std::filesystem::exists(assetPath_))
			std::filesystem::create_directories(assetPath_);

		std::ifstream ifs(std::filesystem::path(this->assetPath_).append("Library/AssetDb.json"), std::ios_base::binary);
		if (ifs)
		{
			try
			{
				auto assetDb = nlohmann::json::parse(ifs);

				for (auto it = assetDb.begin(); it != assetDb.end(); ++it)
				{
					auto uuid = it.key();
					auto path = std::filesystem::path((char8_t*)it.value().get<std::string>().c_str());

					assetGuidList_[path] = uuid;
					assetPathList_[uuid] = path;

					if (uuid == "f7ab8b4ed1abfb4f302ca8d19244554a")
						continue;
				}
			}
			catch (...)
			{
			}
		}
	}

	void
	AssetDatabase::close() noexcept
	{
		assetPath_.clear();
		dirtyList_.clear();
		assetGuidList_.clear();
		assetPathList_.clear();
		objectCaches_.clear();
		objectPathList_.clear();
	}

	void
	AssetDatabase::importAsset(const std::filesystem::path& diskPath, const std::filesystem::path& relativePath) noexcept(false)
	{
		assert(!diskPath.empty());

		if (relativePath.empty())
			return;

		if (relativePath.wstring().substr(0, 6) != L"Assets")
			return;

		if (diskPath.empty() || !std::filesystem::exists(diskPath))
			return;

		auto rootPath = relativePath.parent_path();
		auto assetPath = std::filesystem::path(this->assetPath_).append(relativePath.u8string());

		try
		{
			this->createFolder(rootPath);

			std::filesystem::copy_file(diskPath, assetPath, std::filesystem::copy_options::overwrite_existing);
			std::filesystem::permissions(assetPath, std::filesystem::perms::owner_write);

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
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(assetPath))
				std::filesystem::remove(assetPath);

			if (std::filesystem::exists(assetPath.u8string() + u8".metadata"))
				std::filesystem::remove(assetPath.u8string() + u8".metadata");

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Texture>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty() || relativePath.wstring().substr(0, 6) != L"Assets")
			return;

		auto uuid = MD5(std::filesystem::path(relativePath).make_preferred().u8string()).toString();
		auto assetPath = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto metaPath = assetPath.u8string() + u8".metadata";
		auto extension = assetPath.extension();
		auto type = extension.string().substr(extension.string().find_last_of(".") + 1);

		try
		{
			this->createFolder(relativePath.parent_path());
			asset->save(assetPath, type.c_str());
			std::filesystem::permissions(assetPath, std::filesystem::perms::owner_write);

			nlohmann::json metadata;
			metadata["uuid"] = uuid;
			metadata["name"] = asset->getName();
			metadata["suffix"] = (char*)extension.c_str();
			metadata["mipmap"] = asset->getMipLevel();

			std::ofstream ifs(metaPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = metadata.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			assetGuidList_[relativePath] = uuid;
			assetPathList_[uuid] = relativePath;

			objectPathList_[asset] = relativePath;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(assetPath))
				std::filesystem::remove(assetPath);

			if (std::filesystem::exists(metaPath))
				std::filesystem::remove(metaPath);

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Animation>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty() || relativePath.wstring().substr(0, 6) != L"Assets")
			return;

		auto uuid = MD5(std::filesystem::path(relativePath).make_preferred().u8string()).toString();
		auto assetPath = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto metaPath = assetPath.u8string() + u8".metadata";

		try
		{
			this->createFolder(relativePath.parent_path());

			VMDLoader::save(assetPath, *asset);
			std::filesystem::permissions(assetPath, std::filesystem::perms::owner_write);

			this->objectPathList_[asset] = relativePath;
			this->createMetadataAtPath(relativePath);
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(assetPath))
				std::filesystem::remove(assetPath);

			if (std::filesystem::exists(metaPath))
				std::filesystem::remove(metaPath);

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Material>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty() || relativePath.wstring().substr(0, 6) != L"Assets")
			return;

		auto uuid = MD5(std::filesystem::path(relativePath).make_preferred().u8string()).toString();
		auto assetPath = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto metaPath = assetPath.u8string() + u8".metadata";

		try
		{
			this->createFolder(relativePath.parent_path());

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
						if (!this->contains(texture))
						{
							auto texturePath = std::filesystem::path("Assets/Textures").append(make_guid() + ".png");
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

			std::ofstream ifs(assetPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = mat.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->objectPathList_[asset] = relativePath;
			this->createMetadataAtPath(relativePath);
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(assetPath))
				std::filesystem::remove(assetPath);

			if (std::filesystem::exists(metaPath))
				std::filesystem::remove(metaPath);

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const GameObject>& asset, const std::filesystem::path& relativePath) noexcept(false)
	{
		if (!asset || relativePath.empty() || relativePath.wstring().substr(0, 6) != L"Assets")
			return;

		auto uuid = MD5(std::filesystem::path(relativePath).make_preferred().u8string()).toString();
		auto rootPath = relativePath.parent_path();
		auto assetPath = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto metaPath = assetPath.u8string() + u8".metadata";

		try
		{
			this->createFolder(rootPath);

			nlohmann::json prefab;

			if (!this->contains(asset))
			{
				auto modelPath = AssetLoader::instance()->getAssetPath(asset);
				if (!modelPath.empty())
				{
					auto outputPath = std::filesystem::path("Assets/Models").append(octoon::make_guid()).append(modelPath.filename().wstring());
					this->importAsset(modelPath, outputPath);

					objectPathList_[asset] = outputPath;
					prefab["model"] = this->getAssetGuid(outputPath);
				}
			}
			else
			{
				prefab["model"] = this->getAssetGuid(asset);
			}

			asset->save(prefab, *this);

			std::ofstream ifs(assetPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = prefab.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			this->objectPathList_[asset] = relativePath;
			this->createMetadataAtPath(relativePath);
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(assetPath))
				std::filesystem::remove(assetPath);

			if (std::filesystem::exists(metaPath))
				std::filesystem::remove(metaPath);

			throw e;
		}
	}

	void
	AssetDatabase::deleteAsset(const std::filesystem::path& relativePath) noexcept(false)
	{
		if (relativePath.empty() || relativePath.wstring().substr(0, 6) != L"Assets")
			return;

		auto path = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		if (std::filesystem::is_directory(path))
			std::filesystem::remove_all(path);
		else
		{
			auto uuid = this->getAssetGuid(relativePath);
			if (!uuid.empty())
			{
				assetGuidList_.erase(assetGuidList_.find(relativePath));
				assetPathList_.erase(assetPathList_.find(uuid));
			}

			if (std::filesystem::exists(path))
				std::filesystem::remove(path);

			auto metadata = std::filesystem::path(assetPath_).append(relativePath.wstring() + L".metadata");
			if (std::filesystem::exists(metadata))
				std::filesystem::remove(metadata);
		}
	}

	void
	AssetDatabase::saveAssets() noexcept(false)
	{
		for (auto it = dirtyList_.begin(); it != dirtyList_.end(); ++it)
		{
			if (!(*it).expired())
			{
				auto item = (*it).lock();

				if (item->isInstanceOf<Texture>())
					this->createAsset(item->downcast_pointer<Texture>(), this->getAssetPath(item));
				else if (item->isInstanceOf<Material>())
					this->createAsset(item->downcast_pointer<Material>(), this->getAssetPath(item));
				else if (item->isInstanceOf<Animation>())
					this->createAsset(item->downcast_pointer<Animation>(), this->getAssetPath(item));
				else if (item->isInstanceOf<GameObject>())
					this->createAsset(item->downcast_pointer<GameObject>(), this->getAssetPath(item));
			}

			dirtyList_.erase(it);
		}

		nlohmann::json assetDb;

		for (auto& it : assetPathList_)
		{
			auto path = it.second.u8string();
			if (std::filesystem::exists(std::filesystem::path(this->assetPath_).append(path)))
				assetDb[it.first] = (char*)path.c_str();
		}

		auto assetRoot = std::filesystem::path(this->assetPath_).append("Library");
		std::filesystem::create_directories(assetRoot);

		std::ofstream ifs(std::filesystem::path(assetRoot).append("AssetDb.json"), std::ios_base::binary);
		if (ifs)
		{
			auto dump = assetDb.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::string& uuid) const noexcept
	{
		auto it = assetPathList_.find(uuid);
		if (it != assetPathList_.end())
			return (*it).second;
		return std::filesystem::path();
	}

	void
	AssetDatabase::createFolder(const std::filesystem::path& assetFolder_) noexcept(false)
	{
		if (assetFolder_.empty() || assetFolder_.wstring().substr(0, 6) != L"Assets")
			return;

		auto relativePath = std::filesystem::path(assetFolder_).make_preferred();
		auto path = relativePath.wstring();

		std::filesystem::create_directories(std::filesystem::path(this->assetPath_).append(path));

		auto folderPath = std::filesystem::path(this->assetPath_).append(path + L".metadata");
		if (!std::filesystem::exists(folderPath))
			this->createMetadataAtPath(relativePath);

		auto offset = path.find_first_of(std::filesystem::path::preferred_separator);
		if (offset > 0 && offset < path.size())
		{
			auto folder = path.substr(0, offset);
			if (folder == L"Assets")
				offset = path.find_first_of(std::filesystem::path::preferred_separator, offset + 1);
		}

		while (offset > 0 && offset < path.size())
		{
			auto metaPath = std::filesystem::path(this->assetPath_).append(path.substr(0, offset) + L".metadata");
			if (!std::filesystem::exists(metaPath))
				this->createMetadataAtPath(path.substr(0, offset));

			offset = path.find_first_of(std::filesystem::path::preferred_separator, offset + 1);
		}
	}

	void
	AssetDatabase::deleteFolder(const std::filesystem::path& assetFolder) noexcept(false)
	{
		if (assetFolder.empty() || assetFolder.wstring().substr(0, 6) != L"Assets")
			return;

		auto folderPath = std::filesystem::path(this->assetPath_).append(assetFolder.wstring());
		if (std::filesystem::exists(folderPath))
		{
			std::filesystem::remove_all(folderPath);
			this->removeMetadataAtPath(assetFolder);
		}
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::shared_ptr<const Object>& asset) const noexcept
	{
		auto it = objectPathList_.find(asset);
		if (it != objectPathList_.end())
			return (*it).second;

		return std::filesystem::path();
	}

	std::string
	AssetDatabase::getAssetGuid(const std::filesystem::path& relativePath) const noexcept
	{
		if (!relativePath.empty() && relativePath.wstring().substr(0, 6) == L"Assets")
		{
			auto it = assetGuidList_.find(relativePath);
			if (it != assetGuidList_.end())
				return (*it).second;
		}

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
	AssetDatabase::createMetadataAtPath(const std::filesystem::path& relativePath_) noexcept(false)
	{
		auto relativePath = std::filesystem::path(relativePath_).make_preferred();
		auto uuid = MD5(relativePath.u8string()).toString();
		auto metaPath = std::filesystem::path(this->assetPath_).append(relativePath.wstring() + L".metadata");

		nlohmann::json metadata;
		metadata["uuid"] = uuid;

		assetGuidList_[relativePath] = uuid;
		assetPathList_[uuid] = relativePath;

		std::ofstream ifs(metaPath, std::ios_base::binary);
		if (ifs)
		{
			auto dump = metadata.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}
	}

	void
	AssetDatabase::removeMetadataAtPath(const std::filesystem::path& relativePath_) noexcept
	{
		auto relativePath = std::filesystem::path(relativePath_).make_preferred();
		auto uuid = this->getAssetGuid(relativePath);
		assetGuidList_.erase(assetGuidList_.find(relativePath));
		assetPathList_.erase(assetPathList_.find(uuid));

		auto metaPath = std::filesystem::path(this->assetPath_).append(relativePath.wstring() + L".metadata");
		if (std::filesystem::exists(metaPath))
			std::filesystem::remove(metaPath);
	}

	nlohmann::json
	AssetDatabase::loadMetadataAtPath(const std::filesystem::path& relativePath) noexcept(false)
	{
		auto metaPath = std::filesystem::path(this->assetPath_).append(relativePath.wstring() + L".metadata");
		std::ifstream ifs(metaPath);
		if (ifs)
		{
			auto metaData = nlohmann::json::parse(ifs);

			if (metaData.contains("uuid"))
			{
				auto guid = metaData["uuid"].get<std::string>();
				assetGuidList_[relativePath] = guid;
				assetPathList_[std::move(guid)] = (char*)relativePath.u8string().c_str();

				return metaData;
			}
		}

		return nlohmann::json();
	}

	bool
	AssetDatabase::contains(const std::shared_ptr<const Object>& asset) const noexcept
	{
		auto it = objectPathList_.find(asset);
		if (it != objectPathList_.end())
		{
			auto path = (*it).second;
			return std::filesystem::exists(std::filesystem::path(this->assetPath_).append(path.u8string()));
		}

		return false;
	}

	std::shared_ptr<Object>
	AssetDatabase::loadAssetAtPath(const std::filesystem::path& relativePath) noexcept(false)
	{
		if (relativePath.empty() || relativePath.wstring().substr(0, 6) != L"Assets")
			return nullptr;
		
		if (objectCaches_.contains(relativePath))
		{
			auto cache = objectCaches_.at(relativePath);
			if (!cache.expired())
				return cache.lock();
		}

		auto ext = relativePath.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		auto fullPath = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		if (ext == u8".vmd")
		{
			auto motion = AssetLoader::instance()->loadAssetAtPath<Animation>(fullPath);
			if (motion)
			{
				if (motion->getName().empty())
					motion->setName((char*)fullPath.filename().c_str());

				auto metadata = this->loadMetadataAtPath(relativePath);
				if (metadata.is_null())
					this->createMetadataAtPath(relativePath);

				objectCaches_[relativePath] = motion;
				objectPathList_[motion] = relativePath;

				return motion;
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = AssetLoader::instance()->loadAssetAtPath<Texture>(fullPath);
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
				objectPathList_[texture] = relativePath;

				return texture;
			}
		}
		else if (ext == u8".pmx" || ext == u8".obj" || ext == u8".fbx")
		{
			auto model = AssetLoader::instance()->loadAssetAtPath<GameObject>(fullPath);
			if (model)
			{
				auto metadata = this->loadMetadataAtPath(relativePath);
				if (!metadata.is_object())
					this->createMetadataAtPath(relativePath);

				objectCaches_[relativePath] = model;
				objectPathList_[model] = relativePath;

				return model;
			}
		}
		else if (ext == u8".abc")
		{
			auto model = AssetLoader::instance()->loadAssetAtPath<GameObject>(fullPath);
			if (model)
			{
				auto alembic = model->addComponent<MeshAnimationComponent>();
				alembic->setFilePath(fullPath);

				auto metadata = this->loadMetadataAtPath(relativePath);
				if (!metadata.is_object())
					this->createMetadataAtPath(relativePath);

				objectCaches_[relativePath] = model;
				objectPathList_[model] = relativePath;

				return model;
			}
		}
		else if (ext == u8".mat")
		{
			std::ifstream ifs(fullPath);
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
				objectPathList_[material] = relativePath;

				return std::move(material);
			}
		}
		else if (ext == u8".prefab")
		{
			std::ifstream ifs(fullPath);
			if (ifs)
			{
				auto prefab = nlohmann::json::parse(ifs);
				if (prefab.contains("model"))
				{
					auto path = this->getAssetPath(prefab["model"].get<std::string>());
					if (!path.empty())
					{
						auto cache = this->loadAssetAtPath<GameObject>(path);

						auto object = std::make_shared<GameObject>();
						object->load(prefab, *this);

						auto metadata = this->loadMetadataAtPath(relativePath);
						if (!metadata.is_object())
							this->createMetadataAtPath(relativePath);

						objectCaches_[relativePath] = object;
						objectPathList_[object] = relativePath;

						return object;
					}
				}
			}
		}

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