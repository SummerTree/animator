#include <octoon/asset_database.h>
#include <octoon/asset_bundle.h>
#include <octoon/asset_loader.h>
#include <octoon/asset_preview.h>
#include <octoon/runtime/uuid.h>
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
		assetPath_ = assetPath;

		if (!std::filesystem::exists(assetPath_))
			std::filesystem::create_directories(assetPath_);

		std::ifstream ifs(std::filesystem::path(this->assetPath_).append("Library").append("AssetDb.json"), std::ios_base::binary);
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
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Texture>& texture, const std::filesystem::path& relativePath) noexcept(false)
	{
		assert(texture);
		assert(!relativePath.empty() && relativePath.compare("Assets") > 0);

		auto uuid = MD5(relativePath.u8string()).toString();
		auto path = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto rootPath = path.parent_path();
		auto filename = path.filename();
		auto extension = path.extension();
		auto type = extension.string().substr(extension.string().find_last_of(".") + 1);
		auto metaPath = std::filesystem::path(rootPath).append(filename.u8string().substr(0, filename.u8string().find_last_of('.')) + u8".metadata");

		try
		{
			std::filesystem::create_directories(rootPath);
			texture->save(path, type.c_str());
			std::filesystem::permissions(path, std::filesystem::perms::owner_write);

			nlohmann::json metadata;
			metadata["uuid"] = uuid;
			metadata["name"] = texture->getName();
			metadata["suffix"] = (char*)extension.c_str();
			metadata["mipmap"] = texture->getMipLevel();

			std::ofstream ifs(metaPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = metadata.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			assetGuidList_[relativePath] = uuid;
			assetPathList_[uuid] = relativePath;

			objectPathList_[texture] = relativePath;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(path))
				std::filesystem::remove(path);

			if (std::filesystem::exists(metaPath))
				std::filesystem::remove(metaPath);

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Animation>& animation, const std::filesystem::path& relativePath) noexcept(false)
	{
		assert(!relativePath.empty() && relativePath.compare("Assets") > 0);

		auto uuid = MD5(relativePath.u8string()).toString();
		auto path = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto rootPath = path.parent_path();
		auto filename = path.filename();
		auto extension = path.extension();
		auto metaPath = std::filesystem::path(rootPath).append(filename.u8string().substr(0, filename.u8string().find_last_of('.')) + u8".metadata");

		try
		{
			std::filesystem::create_directories(rootPath);

			VMDLoader::save(path, *animation);
			std::filesystem::permissions(path, std::filesystem::perms::owner_write);

			nlohmann::json metadata;
			metadata["uuid"] = uuid;
			metadata["name"] = animation->getName();

			std::ofstream ifs(metaPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = metadata.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			assetGuidList_[relativePath] = uuid;
			assetPathList_[uuid] = relativePath;

			objectPathList_[animation] = relativePath;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(path))
				std::filesystem::remove(path);

			if (std::filesystem::exists(metaPath))
				std::filesystem::remove(metaPath);

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const Material>& material, const std::filesystem::path& relativePath) noexcept(false)
	{
		assert(!relativePath.empty() && relativePath.compare("Assets") > 0);

		auto uuid = MD5(relativePath.u8string()).toString();
		auto parentPath = relativePath.parent_path();
		auto path = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto rootPath = path.parent_path();
		auto filename = path.filename();
		auto extension = path.extension();
		auto metaPath = std::filesystem::path(rootPath).append(filename.u8string().substr(0, filename.u8string().find_last_of('.')) + u8".metadata");

		try
		{
			auto standardMaterial = material->downcast<MeshStandardMaterial>();

			nlohmann::json mat;
			mat["type"] = standardMaterial->type_name();
			mat["name"] = standardMaterial->getName();
			mat["opacity"] = standardMaterial->getOpacity();
			mat["smoothness"] = standardMaterial->getSmoothness();
			mat["roughness"] = standardMaterial->getRoughness();
			mat["metalness"] = standardMaterial->getMetalness();
			mat["anisotropy"] = standardMaterial->getAnisotropy();
			mat["sheen"] = standardMaterial->getSheen();
			mat["specular"] = standardMaterial->getSpecular();
			mat["refractionRatio"] = standardMaterial->getRefractionRatio();
			mat["clearCoat"] = standardMaterial->getClearCoat();
			mat["clearCoatRoughness"] = standardMaterial->getClearCoatRoughness();
			mat["subsurface"] = standardMaterial->getSubsurface();
			mat["reflectionRatio"] = standardMaterial->getReflectionRatio();
			mat["transmission"] = standardMaterial->getTransmission();
			mat["lightMapIntensity"] = standardMaterial->getLightMapIntensity();
			mat["emissiveIntensity"] = standardMaterial->getEmissiveIntensity();
			mat["gamma"] = standardMaterial->getGamma();
			mat["offset"] = standardMaterial->getOffset().to_array();
			mat["repeat"] = standardMaterial->getRepeat().to_array();
			mat["normalScale"] = standardMaterial->getNormalScale().to_array();
			mat["color"] = standardMaterial->getColor().to_array();
			mat["emissive"] = standardMaterial->getEmissive().to_array();
			mat["subsurfaceColor"] = standardMaterial->getSubsurfaceColor().to_array();
			mat["blendEnable"] = standardMaterial->getBlendEnable();
			mat["blendOp"] = standardMaterial->getBlendOp();
			mat["blendSrc"] = standardMaterial->getBlendSrc();
			mat["blendDest"] = standardMaterial->getBlendDest();
			mat["blendAlphaOp"] = standardMaterial->getBlendAlphaOp();
			mat["blendAlphaSrc"] = standardMaterial->getBlendAlphaSrc();
			mat["blendAlphaDest"] = standardMaterial->getBlendAlphaDest();
			mat["colorWriteMask"] = standardMaterial->getColorWriteMask();
			mat["depthEnable"] = standardMaterial->getDepthEnable();
			mat["depthBiasEnable"] = standardMaterial->getDepthBiasEnable();
			mat["depthBoundsEnable"] = standardMaterial->getDepthBoundsEnable();
			mat["depthClampEnable"] = standardMaterial->getDepthClampEnable();
			mat["depthWriteEnable"] = standardMaterial->getDepthWriteEnable();
			mat["depthMin"] = standardMaterial->getDepthMin();
			mat["depthMax"] = standardMaterial->getDepthMax();
			mat["depthBias"] = standardMaterial->getDepthBias();
			mat["depthSlopeScaleBias"] = standardMaterial->getDepthSlopeScaleBias();
			mat["stencilEnable"] = standardMaterial->getStencilEnable();
			mat["scissorTestEnable"] = standardMaterial->getScissorTestEnable();			

			if (standardMaterial->getColorMap() && !this->isPersistent(standardMaterial->getColorMap()))
				this->createAsset(standardMaterial->getColorMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getOpacityMap() && !this->isPersistent(standardMaterial->getOpacityMap()))
				this->createAsset(standardMaterial->getOpacityMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getNormalMap() && !this->isPersistent(standardMaterial->getNormalMap()))
				this->createAsset(standardMaterial->getNormalMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getRoughnessMap() && !this->isPersistent(standardMaterial->getRoughnessMap()))
				this->createAsset(standardMaterial->getRoughnessMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getSpecularMap() && !this->isPersistent(standardMaterial->getSpecularMap()))
				this->createAsset(standardMaterial->getSpecularMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getMetalnessMap() && !this->isPersistent(standardMaterial->getMetalnessMap()))
				this->createAsset(standardMaterial->getMetalnessMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getEmissiveMap() && !this->isPersistent(standardMaterial->getEmissiveMap()))
				this->createAsset(standardMaterial->getEmissiveMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getAnisotropyMap() && !this->isPersistent(standardMaterial->getAnisotropyMap()))
				this->createAsset(standardMaterial->getAnisotropyMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getClearCoatMap() && !this->isPersistent(standardMaterial->getClearCoatMap()))
				this->createAsset(standardMaterial->getClearCoatMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getClearCoatRoughnessMap() && !this->isPersistent(standardMaterial->getClearCoatRoughnessMap()))
				this->createAsset(standardMaterial->getClearCoatRoughnessMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getSubsurfaceMap() && !this->isPersistent(standardMaterial->getSubsurfaceMap()))
				this->createAsset(standardMaterial->getSubsurfaceMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getSubsurfaceColorMap() && !this->isPersistent(standardMaterial->getSubsurfaceColorMap()))
				this->createAsset(standardMaterial->getSubsurfaceColorMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getSheenMap() && !this->isPersistent(standardMaterial->getSheenMap()))
				this->createAsset(standardMaterial->getSheenMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));
			if (standardMaterial->getLightMap() && !this->isPersistent(standardMaterial->getLightMap()))
				this->createAsset(standardMaterial->getLightMap(), std::filesystem::path(parentPath).append(make_guid() + ".png"));

			if (standardMaterial->getColorMap())
				mat["colorMap"] = this->getAssetGuid(standardMaterial->getColorMap());
			if (standardMaterial->getOpacityMap())
				mat["opacityMap"] = this->getAssetGuid(standardMaterial->getOpacityMap());
			if (standardMaterial->getNormalMap())
				mat["normalMap"] = this->getAssetGuid(standardMaterial->getNormalMap());
			if (standardMaterial->getRoughnessMap())
				mat["roughnessMap"] = this->getAssetGuid(standardMaterial->getRoughnessMap());
			if (standardMaterial->getSpecularMap())
				mat["specularMap"] = this->getAssetGuid(standardMaterial->getSpecularMap());
			if (standardMaterial->getMetalnessMap())
				mat["metalnessMap"] = this->getAssetGuid(standardMaterial->getMetalnessMap());
			if (standardMaterial->getEmissiveMap())
				mat["emissiveMap"] = this->getAssetGuid(standardMaterial->getEmissiveMap());
			if (standardMaterial->getAnisotropyMap())
				mat["anisotropyMap"] = this->getAssetGuid(standardMaterial->getAnisotropyMap());
			if (standardMaterial->getClearCoatMap())
				mat["clearCoatMap"] = this->getAssetGuid(standardMaterial->getClearCoatMap());
			if (standardMaterial->getClearCoatRoughnessMap())
				mat["clearCoatRoughnessMap"] = this->getAssetGuid(standardMaterial->getClearCoatRoughnessMap());
			if (standardMaterial->getSubsurfaceMap())
				mat["subsurfaceMap"] = this->getAssetGuid(standardMaterial->getSubsurfaceMap());
			if (standardMaterial->getSubsurfaceColorMap())
				mat["subsurfaceColorMap"] = this->getAssetGuid(standardMaterial->getSubsurfaceColorMap());
			if (standardMaterial->getSheenMap())
				mat["sheenMap"] = this->getAssetGuid(standardMaterial->getSheenMap());
			if (standardMaterial->getLightMap())
				mat["lightMap"] = this->getAssetGuid(standardMaterial->getLightMap());

			std::filesystem::create_directories(rootPath);

			std::ofstream ifs(path, std::ios_base::binary);
			if (ifs)
			{
				auto dump = mat.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			nlohmann::json metadata;
			metadata["uuid"] = uuid;

			std::ofstream metaFs(metaPath, std::ios_base::binary);
			if (metaFs)
			{
				auto dump = metadata.dump();
				metaFs.write(dump.c_str(), dump.size());
				metaFs.close();
			}

			assetGuidList_[relativePath] = uuid;
			assetPathList_[uuid] = relativePath;

			objectPathList_[material] = relativePath;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(path))
				std::filesystem::remove(path);

			if (std::filesystem::exists(metaPath))
				std::filesystem::remove(metaPath);

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const PMX>& pmx, const std::filesystem::path& relativePath) noexcept(false)
	{
		assert(!relativePath.empty() && relativePath.compare("Assets") > 0);

		auto uuid = MD5(relativePath.u8string()).toString();
		auto path = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto rootPath = path.parent_path();
		auto filename = path.filename();
		auto extension = path.extension();
		auto metaPath = std::filesystem::path(rootPath).append(filename.u8string().substr(0, filename.u8string().find_last_of('.')) + u8".metadata");

		try
		{
			std::filesystem::create_directories(rootPath);

			std::ofstream stream(path, std::ios_base::binary);
			if (stream)
			{
				if (!PMX::save(stream, *pmx))
					throw std::runtime_error("Failed to create model");
			}
			else
			{
				throw std::runtime_error("Failed to create file");
			}

			std::filesystem::permissions(path, std::filesystem::perms::owner_write);

			for (auto& texture : pmx->textures)
			{
				auto texturePath = std::filesystem::path(rootPath).append(texture.name);

				if (std::filesystem::exists(texture.fullpath) && !std::filesystem::exists(texturePath))
				{
					auto textureRootPath = runtime::string::directory(texturePath);
					std::filesystem::create_directories(textureRootPath);
					std::filesystem::copy(texture.fullpath, texturePath);
					std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);
				}
			}

			nlohmann::json metadata;
			metadata["uuid"] = uuid;
			metadata["name"] = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(pmx->description.japanModelName.data());
			
			std::ofstream ifs(metaPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = metadata.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			assetGuidList_[relativePath] = uuid;
			assetPathList_[uuid] = relativePath;

			objectPathList_[pmx] = relativePath;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(path))
				std::filesystem::remove(path);

			if (std::filesystem::exists(metaPath))
				std::filesystem::remove(metaPath);

			throw e;
		}
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<const GameObject>& gameObject, const std::filesystem::path& relativePath) noexcept(false)
	{
		assert(!relativePath.empty() && relativePath.compare("Assets") > 0);

		auto uuid = MD5(relativePath.u8string()).toString();
		auto rootPath = relativePath.parent_path();
		auto filename = relativePath.filename();
		auto extension = relativePath.extension();

		auto assetPath = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		auto metaPath = std::filesystem::path(this->assetPath_).append(rootPath.u8string()).append(filename.u8string().substr(0, filename.u8string().find_last_of('.')) + u8".metadata");

		try
		{
			std::filesystem::create_directories(std::filesystem::path(this->assetPath_).append(rootPath.u8string()));

			nlohmann::json prefab;

			auto modelPath = AssetLoader::instance()->getAssetPath(gameObject);
			if (!modelPath.empty())
				prefab["model"] = (char*)modelPath.u8string().c_str();

			auto transform = gameObject->getComponent<TransformComponent>();
			if (transform)
				transform->save(prefab["transform"]);

			for (auto& component : gameObject->getComponents())
			{
				if (!component->isA<AnimatorComponent>())
					continue;

				auto animation = component->downcast<AnimatorComponent>();
				if (animation->getAnimation())
				{
					if (!this->isPersistent(animation->getAnimation()))
						this->createAsset(animation->getAnimation(), std::filesystem::path(rootPath).append(make_guid() + ".vmd"));

					nlohmann::json animationJson;
					animationJson["data"] = this->getAssetGuid(animation->getAnimation());
					animationJson["type"] = animation->getAvatar().empty() ? 1 : 0;

					prefab["animation"].push_back(animationJson);
				}
			}

			auto meshFilter = gameObject->getComponent<MeshFilterComponent>();
			if (meshFilter)
			{
				auto mesh = meshFilter->getMesh();
				if (mesh)
				{
					auto bound = mesh->getBoundingBoxAll();
					prefab["meshFilter"]["bound"][0] = bound.box().min.to_array();
					prefab["meshFilter"]["bound"][1] = bound.box().max.to_array();
				}
			}

			auto meshRenderer = gameObject->getComponent<MeshRendererComponent>();
			if (meshRenderer)
			{
				auto& materials = meshRenderer->getMaterials();

				for (std::size_t i = 0; i < materials.size(); i++)
				{
					if (!this->isPersistent(materials[i]))
						this->createAsset(materials[i], std::filesystem::path(rootPath).append(make_guid() + ".mat"));

					nlohmann::json materialJson;
					materialJson["data"] = this->getAssetGuid(materials[i]);
					materialJson["index"] = i;

					prefab["meshRenderer"]["materials"].push_back(materialJson);
				}
			}

			auto abc = gameObject->getComponent<MeshAnimationComponent>();
			if (abc)
			{
				prefab["alembic"]["path"] = (char*)assetPath.u8string().c_str();

				for (auto& pair : abc->getMaterials())
				{
					if (!this->isPersistent(pair.second))
						this->createAsset(pair.second, std::filesystem::path(rootPath).append(make_guid() + ".mat"));

					nlohmann::json materialJson;
					materialJson["data"] = this->getAssetGuid(pair.second);
					materialJson["name"] = pair.first;

					prefab["alembic"]["materials"].push_back(materialJson);
				}
			}

			std::ofstream ifs(assetPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = prefab.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			nlohmann::json metadata;
			metadata["uuid"] = uuid;
			metadata["name"] = gameObject->getName();

			std::ofstream metaStream(metaPath, std::ios_base::binary);
			if (metaStream)
			{
				auto dump = metadata.dump();
				metaStream.write(dump.c_str(), dump.size());
				metaStream.close();
			}

			assetGuidList_[relativePath] = uuid;
			assetPathList_[uuid] = relativePath;

			objectPathList_[gameObject] = relativePath;
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
		auto path = std::filesystem::path(this->assetPath_).append(relativePath.u8string());
		if (std::filesystem::is_directory(path))
			std::filesystem::remove_all(path);
		else
		{
			auto uuid = this->getAssetGuid(relativePath);

			assetGuidList_.erase(assetGuidList_.find(relativePath));
			assetPathList_.erase(assetPathList_.find(uuid));

			auto parent_path = path.parent_path();
			auto filepath = path.filename().string();
			auto filename = filepath.substr(0, filepath.find_last_of('.'));
			auto metadata = std::filesystem::path(parent_path).append(filename + ".metadata");

			std::filesystem::remove(path);
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

		nlohmann::json assetDb;

		for (auto& it : assetPathList_)
			assetDb[it.first] = (char*)it.second.u8string().c_str();

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

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::shared_ptr<const RttiObject>& asset) const noexcept
	{
		auto it = objectPathList_.find(asset);
		if (it != objectPathList_.end())
			return (*it).second;

		return std::filesystem::path();
	}

	std::string
	AssetDatabase::getAssetGuid(const std::filesystem::path& relativePath) const noexcept
	{
		auto it = assetGuidList_.find(relativePath.u8string());
		if (it != assetGuidList_.end())
			return (*it).second;
		return std::string();
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<const RttiObject>& asset) const noexcept
	{
		auto path = this->getAssetPath(asset);
		if (!path.empty())
			return this->getAssetGuid(path);
		return std::string();
	}

	nlohmann::json
	AssetDatabase::loadMetadataAtPath(const std::filesystem::path& path) noexcept(false)
	{
		auto parent_path = path.parent_path();
		auto filename = path.filename().string();
		auto uuid = filename.substr(0, filename.find_last_of('.'));
		auto metadata = std::filesystem::path(parent_path).append(uuid + ".metadata");

		if (std::filesystem::exists(metadata))
		{
			std::ifstream ifs(metadata);
			if (ifs)
				return nlohmann::json::parse(ifs);
		}

		return nlohmann::json();
	}

	bool
	AssetDatabase::isPersistent(const std::shared_ptr<const RttiObject>& asset) const noexcept
	{
		auto it = objectPathList_.find(asset);
		if (it != objectPathList_.end())
		{
			auto path = (*it).second;
			return std::filesystem::exists(std::filesystem::path(this->assetPath_).append(path.u8string()));
		}

		return false;
	}

	std::shared_ptr<RttiObject>
	AssetDatabase::loadAssetAtPath(const std::filesystem::path& relativePath) noexcept(false)
	{
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

				auto metadata = this->loadMetadataAtPath(fullPath);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						motion->setName(metadata["name"].get<std::string>());

					if (metadata.contains("uuid"))
						assetGuidList_[relativePath] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[relativePath] = MD5(relativePath.u8string()).toString();
				}

				objectPathList_[motion] = relativePath;
				assetPathList_[this->getAssetGuid(relativePath)] = relativePath;

				return motion;
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = AssetLoader::instance()->loadAssetAtPath<Texture>(fullPath);
			if (texture)
			{
				auto metadata = this->loadMetadataAtPath(fullPath);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						texture->setName(metadata["name"].get<std::string>());

					if (metadata.contains("mipmap"))
						texture->setMipLevel(metadata["mipmap"].get<nlohmann::json::number_integer_t>());

					if (metadata.contains("uuid"))
						assetGuidList_[relativePath] = metadata["uuid"].get<std::string>();
				}
				else
				{
					if (ext == u8".hdr")
						texture->setMipLevel(8);

					assetGuidList_[relativePath] = MD5(relativePath.u8string()).toString();
				}

				texture->apply();

				objectPathList_[texture] = relativePath;
				assetPathList_[this->getAssetGuid(relativePath)] = relativePath;

				return texture;
			}
		}
		else if (ext == u8".pmx" || ext == u8".obj" || ext == u8".fbx")
		{
			auto model = AssetLoader::instance()->loadAssetAtPath<GameObject>(fullPath);
			if (model)
			{
				auto metadata = this->loadMetadataAtPath(fullPath);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						model->setName(metadata["name"].get<std::string>());
					if (metadata.contains("uuid"))
						assetGuidList_[relativePath] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[relativePath] = MD5(relativePath.u8string()).toString();
				}

				objectPathList_[model] = relativePath;
				assetPathList_[this->getAssetGuid(relativePath)] = relativePath;

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

				auto metadata = this->loadMetadataAtPath(fullPath);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						model->setName(metadata["name"].get<std::string>());
					if (metadata.contains("uuid"))
						assetGuidList_[relativePath] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[relativePath] = MD5(relativePath.u8string()).toString();
				}

				objectPathList_[model] = relativePath;
				assetPathList_[this->getAssetGuid(relativePath)] = relativePath;

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

				auto metadata = this->loadMetadataAtPath(fullPath);
				if (metadata.is_object())
				{
					if (metadata.contains("uuid"))
						assetGuidList_[relativePath] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[relativePath] = MD5(relativePath.u8string()).toString();
				}

				objectPathList_[material] = relativePath;
				assetPathList_[this->getAssetGuid(relativePath)] = relativePath;

				return material;
			}
		}
		else if (ext == u8".prefab")
		{
			std::ifstream ifs(fullPath);
			if (ifs)
			{
				auto prefab = nlohmann::json::parse(ifs);

				GameObjectPtr object;

				if (prefab.contains("model"))
				{
					auto path = prefab["model"].get<std::string>();
					object = this->loadAssetAtPath<GameObject>(path);
				}

				if (prefab.contains("transform"))
				{
					auto transform = object->getComponent<octoon::TransformComponent>();
					if (transform)
						transform->load(prefab["transform"]);
				}

				for (auto& animationJson : prefab["animation"])
				{
					if (animationJson.find("data") == animationJson.end())
						continue;

					auto data = animationJson["data"].get<nlohmann::json::string_t>();
					auto animation = this->loadAssetAtPath<octoon::Animation>(this->getAssetPath(data));
					if (animation)
					{
						auto animationType = animationJson["type"].get<nlohmann::json::number_unsigned_t>();
						if (animationType == 0)
							object->addComponent<octoon::AnimatorComponent>(std::move(animation), object->getComponent<octoon::SkinnedMeshRendererComponent>()->getTransforms());
						else
							object->addComponent<octoon::AnimatorComponent>(std::move(animation));
					}
				}

				if (prefab.contains("meshRenderer"))
				{
					std::vector<std::shared_ptr<octoon::Material>> materials;
					materials.resize(prefab["meshRenderer"]["materials"].size());

					for (auto& materialJson : prefab["meshRenderer"]["materials"])
					{
						if (materialJson.find("data") == materialJson.end())
							continue;

						auto data = materialJson["data"].get<nlohmann::json::string_t>();
						auto index = materialJson["index"].get<nlohmann::json::number_unsigned_t>();
						auto material = this->loadAssetAtPath<octoon::Material>(this->getAssetPath(data));

						materials[index] = std::move(material);
					}

					auto meshRenderer = object->getComponent<octoon::MeshRendererComponent>();
					if (meshRenderer)
						meshRenderer->setMaterials(std::move(materials));
				}

				auto metadata = this->loadMetadataAtPath(fullPath);
				if (metadata.is_object())
				{
					if (metadata.contains("uuid"))
						assetGuidList_[relativePath] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[relativePath] = MD5(relativePath.u8string()).toString();
				}

				objectPathList_[object] = relativePath;
				assetPathList_[this->getAssetGuid(relativePath)] = relativePath;

				return object;
			}
		}

		return nullptr;
	}

	void
	AssetDatabase::setDirty(const std::shared_ptr<RttiObject>& object, bool dirty) noexcept(false)
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
	AssetDatabase::isDirty(const std::shared_ptr<RttiObject>& object) const noexcept
	{
		return this->dirtyList_.contains(object);
	}

	void
	AssetDatabase::clearUpdate() noexcept
	{
		this->dirtyList_.clear();
	}
}