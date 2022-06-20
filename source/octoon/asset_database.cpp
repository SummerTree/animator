#include <octoon/asset_database.h>
#include <octoon/asset_bundle.h>
#include <octoon/asset_preview.h>
#include <octoon/runtime/uuid.h>
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
	}

	void
	AssetDatabase::createAsset(const std::shared_ptr<Texture>& texture, const std::filesystem::path& path) noexcept(false)
	{
		assert(texture);
		assert(!path.empty());

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
			metadata["uuid"] = AssetDatabase::instance()->getAssetGuid(texture);
			metadata["name"] = texture->getName();
			metadata["suffix"] = (char*)extension.c_str();
			metadata["path"] = (char*)path.u8string().c_str();
			metadata["mipmap"] = texture->getMipLevel();

			std::ofstream ifs(metaPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = metadata.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}
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
	AssetDatabase::createAsset(const std::shared_ptr<Animation>& animation, const std::filesystem::path& path) noexcept(false)
	{
		assert(!path.empty());

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
			metadata["uuid"] = AssetDatabase::instance()->getAssetGuid(animation);
			metadata["name"] = animation->getName();
			metadata["path"] = (char*)path.u8string().c_str();

			std::ofstream ifs(metaPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = metadata.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}
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
	AssetDatabase::createAsset(const std::shared_ptr<Material>& material, const std::filesystem::path& path) noexcept(false)
	{
		auto rootPath = path.parent_path();
		auto filename = path.filename();
		auto extension = path.extension();

		auto metaPath = std::filesystem::path(rootPath).append(filename.u8string().substr(0, filename.u8string().find_last_of('.')) + u8".metadata");

		try
		{
			auto standardMaterial = material->downcast<MeshStandardMaterial>();

			nlohmann::json mat;
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

			if (standardMaterial->getColorMap())
				mat["colorMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getColorMap());
			if (standardMaterial->getOpacityMap())
				mat["opacityMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getOpacityMap());
			if (standardMaterial->getNormalMap())
				mat["normalMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getNormalMap());
			if (standardMaterial->getRoughnessMap())
				mat["roughnessMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getRoughnessMap());
			if (standardMaterial->getSpecularMap())
				mat["specularMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getSpecularMap());
			if (standardMaterial->getMetalnessMap())
				mat["metalnessMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getMetalnessMap());
			if (standardMaterial->getEmissiveMap())
				mat["emissiveMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getEmissiveMap());
			if (standardMaterial->getAnisotropyMap())
				mat["anisotropyMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getAnisotropyMap());
			if (standardMaterial->getClearCoatMap())
				mat["clearCoatMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getClearCoatMap());
			if (standardMaterial->getClearCoatRoughnessMap())
				mat["clearCoatRoughnessMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getClearCoatRoughnessMap());
			if (standardMaterial->getSubsurfaceMap())
				mat["subsurfaceMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getSubsurfaceMap());
			if (standardMaterial->getSubsurfaceColorMap())
				mat["subsurfaceColorMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getSubsurfaceColorMap());
			if (standardMaterial->getSheenMap())
				mat["sheenMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getSheenMap());
			if (standardMaterial->getLightMap())
				mat["lightMap"] = AssetDatabase::instance()->getAssetGuid(standardMaterial->getLightMap());

			std::filesystem::create_directories(rootPath);

			std::ofstream ifs(path, std::ios_base::binary);
			if (ifs)
			{
				auto dump = mat.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			nlohmann::json metadata;
			metadata["uuid"] = AssetDatabase::instance()->getAssetGuid(material);
			metadata["path"] = (char*)path.u8string().c_str();

			std::ofstream metaFs(metaPath, std::ios_base::binary);
			if (metaFs)
			{
				auto dump = metadata.dump();
				metaFs.write(dump.c_str(), dump.size());
				metaFs.close();
			}
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
	AssetDatabase::createAsset(const std::shared_ptr<PMX>& pmx, const std::filesystem::path& path) noexcept(false)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

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
			metadata["uuid"] = AssetDatabase::instance()->getAssetGuid(pmx);
			metadata["visible"] = true;
			metadata["name"] = cv.to_bytes(pmx->description.japanModelName.data());
			metadata["path"] = (char*)path.u8string().c_str();
			
			std::ofstream ifs(metaPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = metadata.dump();
				ifs.write(dump.c_str(), dump.size());
			}
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
	AssetDatabase::createAsset(const std::shared_ptr<GameObject>& gameObject, const std::filesystem::path& path) noexcept(false)
	{
		assert(!path.empty());

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		auto rootPath = path.parent_path();
		auto filename = path.filename();
		auto extension = path.extension();
		auto metaPath = std::filesystem::path(rootPath).append(filename.u8string().substr(0, filename.u8string().find_last_of('.')) + u8".metadata");

		try
		{
			std::filesystem::create_directories(rootPath);

			PMXLoader::save(*gameObject, path);
			std::filesystem::permissions(path, std::filesystem::perms::owner_write);

			nlohmann::json metadata;
			metadata["uuid"] = this->getAssetGuid(gameObject);
			metadata["visible"] = true;
			metadata["name"] = gameObject->getName();
			metadata["path"] = (char*)path.u8string().c_str();

			std::ofstream ifs(metaPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = metadata.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}
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

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::shared_ptr<const RttiObject>& asset) noexcept
	{
		if (assetPathList_.contains(asset))
			return assetPathList_.at(asset);
		return std::string();
	}

	std::filesystem::path
	AssetDatabase::getAssetPath(const std::shared_ptr<const RttiObject>& asset) const noexcept
	{
		if (assetPathList_.contains(asset))
			return assetPathList_.at(asset);
		return std::string();
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<const RttiObject>& asset) noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		else
		{
			auto guid = make_guid();
			assetGuidList_[asset] = guid;
			return guid;
		}
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<const RttiObject>& asset) const noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
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

	std::shared_ptr<RttiObject>
	AssetDatabase::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		auto ext = path.extension().string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == ".vmd")
		{
			auto motion = VMDLoader::load(path);
			if (motion)
			{
				assetPathList_[motion] = path;

				auto metadata = this->loadMetadataAtPath(path);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						motion->setName(metadata["name"].get<std::string>());

					if (metadata.contains("uuid"))
						assetGuidList_[motion] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[motion] = make_guid();
				}

				return motion;
			}
		}
		else if (ext == ".hdr" || ext == ".bmp" || ext == ".tga" || ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".dds")
		{
			auto texture = std::make_shared<Texture>();
			if (texture->load(path))
			{
				texture->setName((char*)path.filename().u8string().c_str());

				auto metadata = this->loadMetadataAtPath(path);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						texture->setName(metadata["name"].get<std::string>());

					if (metadata.contains("mipmap"))
						texture->setMipLevel(metadata["mipmap"].get<nlohmann::json::number_integer_t>());

					if (metadata.contains("uuid"))
						assetGuidList_[texture] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[texture] = make_guid();
				}

				texture->apply();

				assetPathList_[texture] = path;

				return texture;
			}
		}
		else if (ext == ".pmx")
		{
			octoon::PMXLoadFlags flags = octoon::PMXLoadFlagBits::AllBit;
			/*if (it.find("materials") != it.end())
				flags = flags & ~octoon::PMXLoadFlagBits::MaterialBit;*/

			auto model = PMXLoader::load(path, flags);
			if (model)
			{
				auto metadata = this->loadMetadataAtPath(path);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						model->setName(metadata["name"].get<std::string>());
					if (metadata.contains("uuid"))
						assetGuidList_[model] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[model] = make_guid();
				}

				assetPathList_[model] = path;

				return model;
			}
		}
		else if (ext == ".obj")
		{
			auto model = OBJLoader::load(path);
			if (model)
			{
				auto metadata = this->loadMetadataAtPath(path);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						model->setName(metadata["name"].get<std::string>());
					if (metadata.contains("uuid"))
						assetGuidList_[model] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[model] = make_guid();
				}

				assetPathList_[model] = path;

				return model;
			}
		}
		else if (ext == ".fbx")
		{
			auto model = FBXLoader::load(path);
			if (model)
			{
				auto metadata = this->loadMetadataAtPath(path);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						model->setName(metadata["name"].get<std::string>());
					if (metadata.contains("uuid"))
						assetGuidList_[model] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[model] = make_guid();
				}

				assetPathList_[model] = path;

				return model;
			}
		}
		else if (ext == ".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				auto alembic = model->addComponent<MeshAnimationComponent>();

				auto metadata = this->loadMetadataAtPath(path);
				if (metadata.is_object())
				{
					if (metadata.contains("name"))
						model->setName(metadata["name"].get<std::string>());
					if (metadata.contains("uuid"))
						assetGuidList_[model] = metadata["uuid"].get<std::string>();

					std::unordered_map<std::string, octoon::MaterialPtr> materials;

					if (metadata.contains("materials"))
					{
						for (auto& material : metadata["materials"])
						{
							auto data = material["data"].get<nlohmann::json::string_t>();
							auto name = material["name"].get<std::string>();
							materials[name] = octoon::AssetBundle::instance()->loadAsset<octoon::Material>(data);
						}
						
						alembic->setMaterials(std::move(materials));
						alembic->setFilePath(path);
					}
				}
				else
				{
					alembic->setFilePath(path);
					assetGuidList_[model] = make_guid();
				}

				assetPathList_[model] = path;

				return model;
			}
		}
		else if (ext == ".mat")
		{
			std::ifstream ifs(path);
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
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*colorMap);
					if (texture) texture->apply();
					material->setColorMap(texture);
				}
				if (opacityMap != mat.end() && (*opacityMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*opacityMap);
					if (texture) texture->apply();
					material->setOpacityMap(texture);
				}
				if (normalMap != mat.end() && (*normalMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*normalMap);
					if (texture) texture->apply();
					material->setNormalMap(texture);
				}
				if (roughnessMap != mat.end() && (*roughnessMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*roughnessMap);
					if (texture) texture->apply();
					material->setRoughnessMap(texture);
				}
				if (specularMap != mat.end() && (*specularMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*specularMap);
					if (texture) texture->apply();
					material->setSpecularMap(texture);
				}
				if (metalnessMap != mat.end() && (*metalnessMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*metalnessMap);
					if (texture) texture->apply();
					material->setMetalnessMap(texture);
				}
				if (emissiveMap != mat.end() && (*emissiveMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*emissiveMap);
					if (texture) texture->apply();
					material->setEmissiveMap(texture);
				}
				if (anisotropyMap != mat.end() && (*anisotropyMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*anisotropyMap);
					if (texture) texture->apply();
					material->setAnisotropyMap(texture);
				}
				if (clearCoatMap != mat.end() && (*clearCoatMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*clearCoatMap);
					if (texture) texture->apply();
					material->setClearCoatMap(texture);
				}
				if (clearCoatRoughnessMap != mat.end() && (*clearCoatRoughnessMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*clearCoatRoughnessMap);
					if (texture) texture->apply();
					material->setClearCoatRoughnessMap(texture);
				}
				if (subsurfaceMap != mat.end() && (*subsurfaceMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*subsurfaceMap);
					if (texture) texture->apply();
					material->setSubsurfaceMap(texture);
				}
				if (subsurfaceColorMap != mat.end() && (*subsurfaceColorMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*subsurfaceColorMap);
					if (texture) texture->apply();
					material->setSubsurfaceColorMap(texture);
				}
				if (sheenMap != mat.end() && (*sheenMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*sheenMap);
					if (texture) texture->apply();
					material->setSheenMap(texture);
				}
				if (lightMap != mat.end() && (*lightMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*lightMap);
					if (texture) texture->apply();
					material->setLightMap(texture);
				}

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

				auto metadata = this->loadMetadataAtPath(path);
				if (metadata.is_object())
				{
					if (metadata.contains("uuid"))
						assetGuidList_[material] = metadata["uuid"].get<std::string>();
				}
				else
				{
					assetGuidList_[material] = make_guid();
				}

				assetPathList_[material] = path;

				return material;
			}
		}

		return nullptr;
	}
}