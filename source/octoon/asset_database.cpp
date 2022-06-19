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
		this->close();
	}

	void
	AssetDatabase::open() noexcept(false)
	{
		AssetPreview::instance()->open();
	}

	void
	AssetDatabase::close() noexcept
	{
		assetList_.clear();
		assetPathList_.clear();
		assetGuidList_.clear();

		AssetPreview::instance()->close();
	}

	nlohmann::json
	AssetDatabase::createAsset(const std::shared_ptr<Texture>& texture, const std::filesystem::path& path) noexcept(false)
	{
		assert(!path.empty());

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		auto uuid = this->getAssetGuid(texture);
		auto filename = std::filesystem::path(this->getAssetPath(texture)).extension().u8string();
		auto rootPath = std::filesystem::path(path).append(uuid);
		auto texturePath = std::filesystem::path(rootPath).append(uuid + (char*)filename.c_str());
		auto packagePath = std::filesystem::path(rootPath).append("package.json");

		std::filesystem::create_directories(rootPath);

		auto extension = texturePath.string().substr(texturePath.string().find_last_of(".") + 1);
		texture->save(texturePath, extension.c_str());
		std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);

		nlohmann::json package;
		package["uuid"] = uuid;
		package["visible"] = true;
		package["name"] = texture->getName();
		package["suffix"] = (char*)filename.c_str();
		package["path"] = (char*)texturePath.u8string().c_str();
		package["mipmap"] = texture->getMipLevel() > 1;

		if (filename == u8".hdr")
		{
			auto name = texture->getName();
			auto width = texture->width();
			auto height = texture->height();
			auto data_ = (float*)texture->data();

			auto size = width * height * 3;
			auto pixels = std::make_unique<std::uint8_t[]>(size);

			for (std::size_t i = 0; i < size; i += 3)
			{
				pixels[i] = static_cast<std::uint8_t>(std::clamp(std::pow(data_[i], 1.0f / 2.2f) * 255.0f, 0.0f, 255.0f));
				pixels[i + 1] = static_cast<std::uint8_t>(std::clamp(std::pow(data_[i + 1], 1.0f / 2.2f) * 255.0f, 0.0f, 255.0f));
				pixels[i + 2] = static_cast<std::uint8_t>(std::clamp(std::pow(data_[i + 2], 1.0f / 2.2f) * 255.0f, 0.0f, 255.0f));
			}

			auto uuid2 = make_guid();
			auto texturePath2 = std::filesystem::path(rootPath).append(uuid2 + ".png");

			Texture image(Format::R8G8B8SRGB, width, height, pixels.get());
			image.resize(260, 130).save(texturePath2, "png");

			package["preview"] = (char*)texturePath2.u8string().c_str();
		}

		std::ofstream ifs(packagePath, std::ios_base::binary);
		if (ifs)
		{
			auto dump = package.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}

		this->packageList_[std::string(uuid)] = package;

		return package;
	}

	nlohmann::json
	AssetDatabase::createAsset(const std::shared_ptr<Animation>& animation, const std::filesystem::path& path) noexcept(false)
	{
		assert(!path.empty());

		auto uuid = this->getAssetGuid(animation);
		auto rootPath = std::filesystem::path(path).append(uuid);
		auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
		auto packagePath = std::filesystem::path(rootPath).append("package.json");

		std::filesystem::create_directories(rootPath);

		VMDLoader::save(motionPath, *animation);
		std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

		nlohmann::json package;
		package["uuid"] = uuid;
		package["visible"] = true;
		package["name"] = animation->getName();
		package["path"] = (char*)motionPath.u8string().c_str();

		std::ofstream ifs(packagePath, std::ios_base::binary);
		if (ifs)
		{
			auto dump = package.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}

		this->assetGuidList_[animation] = std::string(uuid);
		this->packageList_[std::string(uuid)] = package;

		return package;
	}

	nlohmann::json
	AssetDatabase::createAsset(const std::shared_ptr<Material>& material, const std::filesystem::path& path) noexcept(false)
	{
		auto uuid = this->getAssetGuid(material);
		auto outputPath = std::filesystem::path(path).append(uuid);
		auto outputTexturePath = std::filesystem::path(path);

		std::filesystem::create_directories(outputPath);

		try
		{
			auto writePreview = [this](const std::shared_ptr<Material>& material, std::filesystem::path path)
			{
				auto texture = AssetPreview::instance()->getAssetPreview(material);
				auto previewPath = std::filesystem::path(path).append(make_guid() + ".png");
				texture->save(previewPath, "png");
				return previewPath;
			};

			auto standardMaterial = material->downcast<MeshStandardMaterial>();

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = standardMaterial->getName();
			package["visible"] = true;
			package["preview"] = (char*)writePreview(material, outputPath).u8string().c_str();
			package["opacity"] = standardMaterial->getOpacity();
			package["smoothness"] = standardMaterial->getSmoothness();
			package["roughness"] = standardMaterial->getRoughness();
			package["metalness"] = standardMaterial->getMetalness();
			package["anisotropy"] = standardMaterial->getAnisotropy();
			package["sheen"] = standardMaterial->getSheen();
			package["specular"] = standardMaterial->getSpecular();
			package["refractionRatio"] = standardMaterial->getRefractionRatio();
			package["clearCoat"] = standardMaterial->getClearCoat();
			package["clearCoatRoughness"] = standardMaterial->getClearCoatRoughness();
			package["subsurface"] = standardMaterial->getSubsurface();
			package["reflectionRatio"] = standardMaterial->getReflectionRatio();
			package["transmission"] = standardMaterial->getTransmission();
			package["lightMapIntensity"] = standardMaterial->getLightMapIntensity();
			package["emissiveIntensity"] = standardMaterial->getEmissiveIntensity();
			package["gamma"] = standardMaterial->getGamma();
			package["offset"] = standardMaterial->getOffset().to_array();
			package["repeat"] = standardMaterial->getRepeat().to_array();
			package["normalScale"] = standardMaterial->getNormalScale().to_array();
			package["color"] = standardMaterial->getColor().to_array();
			package["emissive"] = standardMaterial->getEmissive().to_array();
			package["subsurfaceColor"] = standardMaterial->getSubsurfaceColor().to_array();
			package["blendEnable"] = standardMaterial->getBlendEnable();
			package["blendOp"] = standardMaterial->getBlendOp();
			package["blendSrc"] = standardMaterial->getBlendSrc();
			package["blendDest"] = standardMaterial->getBlendDest();
			package["blendAlphaOp"] = standardMaterial->getBlendAlphaOp();
			package["blendAlphaSrc"] = standardMaterial->getBlendAlphaSrc();
			package["blendAlphaDest"] = standardMaterial->getBlendAlphaDest();
			package["colorWriteMask"] = standardMaterial->getColorWriteMask();
			package["depthEnable"] = standardMaterial->getDepthEnable();
			package["depthBiasEnable"] = standardMaterial->getDepthBiasEnable();
			package["depthBoundsEnable"] = standardMaterial->getDepthBoundsEnable();
			package["depthClampEnable"] = standardMaterial->getDepthClampEnable();
			package["depthWriteEnable"] = standardMaterial->getDepthWriteEnable();
			package["depthMin"] = standardMaterial->getDepthMin();
			package["depthMax"] = standardMaterial->getDepthMax();
			package["depthBias"] = standardMaterial->getDepthBias();
			package["depthSlopeScaleBias"] = standardMaterial->getDepthSlopeScaleBias();
			package["stencilEnable"] = standardMaterial->getStencilEnable();
			package["scissorTestEnable"] = standardMaterial->getScissorTestEnable();			

			if (standardMaterial->getColorMap())
				package["colorMap"] = this->getAssetGuid(standardMaterial->getColorMap());
			if (standardMaterial->getOpacityMap())
				package["opacityMap"] = this->getAssetGuid(standardMaterial->getOpacityMap());
			if (standardMaterial->getNormalMap())
				package["normalMap"] = this->getAssetGuid(standardMaterial->getNormalMap());
			if (standardMaterial->getRoughnessMap())
				package["roughnessMap"] = this->getAssetGuid(standardMaterial->getRoughnessMap());
			if (standardMaterial->getSpecularMap())
				package["specularMap"] = this->getAssetGuid(standardMaterial->getSpecularMap());
			if (standardMaterial->getMetalnessMap())
				package["metalnessMap"] = this->getAssetGuid(standardMaterial->getMetalnessMap());
			if (standardMaterial->getEmissiveMap())
				package["emissiveMap"] = this->getAssetGuid(standardMaterial->getEmissiveMap());
			if (standardMaterial->getAnisotropyMap())
				package["anisotropyMap"] = this->getAssetGuid(standardMaterial->getAnisotropyMap());
			if (standardMaterial->getClearCoatMap())
				package["clearCoatMap"] = this->getAssetGuid(standardMaterial->getClearCoatMap());
			if (standardMaterial->getClearCoatRoughnessMap())
				package["clearCoatRoughnessMap"] = this->getAssetGuid(standardMaterial->getClearCoatRoughnessMap());
			if (standardMaterial->getSubsurfaceMap())
				package["subsurfaceMap"] = this->getAssetGuid(standardMaterial->getSubsurfaceMap());
			if (standardMaterial->getSubsurfaceColorMap())
				package["subsurfaceColorMap"] = this->getAssetGuid(standardMaterial->getSubsurfaceColorMap());
			if (standardMaterial->getSheenMap())
				package["sheenMap"] = this->getAssetGuid(standardMaterial->getSheenMap());
			if (standardMaterial->getLightMap())
				package["lightMap"] = this->getAssetGuid(standardMaterial->getLightMap());

			std::ofstream ifs(std::filesystem::path(outputPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			this->assetGuidList_[material] = std::string(uuid);
			this->packageList_[std::string(uuid)] = package;

			return package;
		}
		catch (std::exception& e)
		{
			std::filesystem::remove_all(outputPath);
			throw e;
		}
	}

	nlohmann::json
	AssetDatabase::createAsset(const std::shared_ptr<GameObject>& gameObject, const std::filesystem::path& path) noexcept(false)
	{
		assert(!path.empty());

		auto uuid = this->getAssetGuid(gameObject);
		auto rootPath = std::filesystem::path(path).append(uuid);
		auto modelPath = std::filesystem::path(rootPath).append(uuid + ".pmx");
		auto packagePath = std::filesystem::path(rootPath).append("package.json");

		std::filesystem::create_directories(rootPath);

		PMXLoader::save(*gameObject, modelPath);
		std::filesystem::permissions(modelPath, std::filesystem::perms::owner_write);

		nlohmann::json package;
		package["uuid"] = uuid;
		package["visible"] = true;
		package["name"] = uuid + ".pmx";
		package["path"] = (char*)modelPath.u8string().c_str();

		std::ofstream ifs(packagePath, std::ios_base::binary);
		if (ifs)
		{
			auto dump = package.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}

		this->assetGuidList_[gameObject] = std::string(uuid);
		this->packageList_[std::string(uuid)] = package;

		return package;
	}

	nlohmann::json
	AssetDatabase::createAsset(const PMX& pmx, const std::filesystem::path& path) noexcept(false)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		auto uuid = make_guid();
		auto rootPath = std::filesystem::path(path).append(uuid);
		auto modelPath = std::filesystem::path(rootPath).append(uuid + ".pmx");
		auto packagePath = std::filesystem::path(rootPath).append("package.json");

		try
		{
			std::filesystem::create_directories(rootPath);

			std::ofstream stream(modelPath, std::ios_base::binary);
			if (stream)
			{
				if (!PMX::save(stream, pmx))
					throw std::runtime_error("Failed to create model");
			}
			else
			{
				throw std::runtime_error("Failed to create file");
			}

			std::filesystem::permissions(modelPath, std::filesystem::perms::owner_write);

			for (auto& texture : pmx.textures)
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

			math::BoundingBox bound;
			for (auto& v : pmx.vertices)
				bound.encapsulate(math::float3(v.position.x, v.position.y, v.position.z));

			auto minBounding = bound.box().min;
			auto maxBounding = bound.box().max;

			auto writePreview = [this](const PMX& pmx, const math::BoundingBox& boundingBox, std::filesystem::path outputPath)
			{
				auto texture = AssetPreview::instance()->getAssetPreview(pmx, boundingBox);
				auto previewPath = std::filesystem::path(outputPath).append(make_guid() + ".png");
				texture->save(previewPath, "png");
				return previewPath;
			};

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = cv.to_bytes(pmx.description.japanModelName.data());
			package["path"] = (char*)modelPath.u8string().c_str();
			package["preview"] = (char*)writePreview(pmx, bound, rootPath).u8string().c_str();
			package["bound"][0] = { minBounding.x, minBounding.y, minBounding.z };
			package["bound"][1] = { maxBounding.x, maxBounding.y, maxBounding.z };

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			this->packageList_[std::string(uuid)] = package;

			return package;
		}
		catch (std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
		}
	}

	std::string
	AssetDatabase::getAssetPath(const std::shared_ptr<const RttiObject>& asset) noexcept
	{
		if (assetPathList_.contains(asset))
			return assetPathList_.at(asset);
		return std::string();
	}

	std::string
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
	AssetDatabase::getPackage(std::string_view uuid, const std::filesystem::path& path) noexcept
	{
		assert(!path.empty());

		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(path).append(uuid).append("package.json"));
			if (ifs)
			{
				auto package = nlohmann::json::parse(ifs);
				this->packageList_[std::string(uuid)] = package;
				return package;
			}
			else
			{
				return nlohmann::json();
			}
		}

		return this->packageList_[std::string(uuid)];
	}

	nlohmann::json
	AssetDatabase::getPackage(const std::shared_ptr<RttiObject>& object) const noexcept(false)
	{
		if (object)
		{
			auto it = assetList_.find(object);
			if (it != assetList_.end())
			{
				auto& package = (*it).second;

				if (package.contains("path"))
				{
					auto path = package["path"].get<std::string>();
					if (std::filesystem::exists(path))
						return package;
				}
				else
				{
					return package;
				}				
			}
		}

		return nlohmann::json();
	}

	std::shared_ptr<RttiObject>
	AssetDatabase::loadAssetAtPath(const std::filesystem::path& path, std::string_view uuid) noexcept(false)
	{
		auto ext = path.extension().string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == ".vmd")
		{
			auto motion = VMDLoader::load(path);
			if (motion)
			{
				assetPathList_[motion] = (char*)path.u8string().c_str();
				assetGuidList_[motion] = uuid;
				return motion;
			}
		}
		else if (ext == ".hdr" || ext == ".bmp" || ext == ".tga" || ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".dds")
		{
			auto texture = std::make_shared<Texture>();
			if (texture->load(path))
			{
				texture->setName((char*)path.filename().c_str());
				assetPathList_[texture] = (char*)path.u8string().c_str();
				assetGuidList_[texture] = uuid;
				return texture;
			}
		}
		else if (ext == ".pmx")
		{
			auto model = PMXLoader::load(path, PMXLoadFlagBits::AllBit);
			if (model)
			{
				assetPathList_[model] = (char*)path.u8string().c_str();
				assetGuidList_[model] = uuid;
				return model;
			}
		}
		else if (ext == ".obj")
		{
			auto model = OBJLoader::load(path);
			if (model)
			{
				assetPathList_[model] = (char*)path.u8string().c_str();
				assetGuidList_[model] = uuid;
				return model;
			}
		}
		else if (ext == ".fbx")
		{
			auto model = FBXLoader::load(path);
			if (model)
			{
				assetPathList_[model] = (char*)path.u8string().c_str();
				assetGuidList_[model] = uuid;
				return model;
			}
		}
		else if (ext == ".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				model->addComponent<MeshAnimationComponent>(path);
				assetPathList_[model] = (char*)path.u8string().c_str();
				assetGuidList_[model] = uuid;
				return model;
			}
		}

		return nullptr;
	}

	std::shared_ptr<RttiObject>
	AssetDatabase::loadAssetAtPackage(const nlohmann::json& package, const Rtti& type) noexcept(false)
	{
		if (type.isDerivedFrom(Texture::getRtti()))
		{
			if (package.contains("path") && package.contains("uuid"))
			{
				auto path = package["path"].get<std::string>();
				auto uuid = package["uuid"].get<std::string>();
				auto filepath = std::filesystem::path((char8_t*)path.c_str());

				auto texture = this->loadAssetAtPath<Texture>(filepath, uuid);
				if (texture)
				{
					if (package.contains("mipmap"))
					{
						if (package["mipmap"].get<nlohmann::json::boolean_t>())
							texture->setMipLevel(8);
					}

					texture->apply();

					packageList_[uuid] = package;
					assetList_[texture] = package;

					return texture;
				}
			}
		}
		else if (type.isDerivedFrom(Animation::getRtti()))
		{
			if (package.contains("path") && package.contains("uuid"))
			{
				auto uuid = package["uuid"].get<std::string>();
				auto path = package["path"].get<std::string>();
				auto filepath = std::filesystem::path((char8_t*)path.c_str());

				auto motion = this->loadAssetAtPath<Animation>(filepath, uuid);
				if (motion)
				{
					packageList_[uuid] = package;
					assetList_[motion] = package;

					return motion;
				}
			}
		}
		else if (type.isDerivedFrom(GameObject::getRtti()))
		{
			if (package.contains("path") && package.contains("uuid"))
			{
				auto uuid = package["uuid"].get<std::string>();
				auto path = package["path"].get<std::string>();
				auto filepath = std::filesystem::path((char8_t*)path.c_str());

				auto ext = filepath.extension().u8string();
				for (auto& it : ext)
					it = (char)std::tolower(it);

				if (ext == u8".pmx" || ext == u8".obj" || ext == u8".fbx")
				{
					auto gameObject = this->loadAssetAtPath<GameObject>(filepath, uuid);
					if (gameObject)
					{
						packageList_[uuid] = package;
						assetList_[gameObject] = package;

						return gameObject;
					}
				}
				else if (ext == u8".abc")
				{
					std::unordered_map<std::string, octoon::MaterialPtr> materials;

					if (package.contains("materials"))
					{
						for (auto& material : package["materials"])
						{
							auto data = material["data"].get<nlohmann::json::string_t>();
							auto name = material["name"].get<std::string>();
							materials[name] = octoon::AssetBundle::instance()->loadAsset<octoon::Material>(data);
						}
					}

					auto gameObject = std::make_shared<GameObject>();
					if (gameObject)
					{
						auto alembic = gameObject->addComponent<MeshAnimationComponent>();
						alembic->setMaterials(std::move(materials));
						alembic->setFilePath(filepath);

						packageList_[uuid] = package;
						assetList_[gameObject] = package;
						assetPathList_[gameObject] = path;
						assetGuidList_[gameObject] = uuid;
						return gameObject;
					}
				}
			}
		}
		else if (type.isDerivedFrom(Material::getRtti()))
		{
			if (package.contains("uuid"))
			{
				auto uuid = package["uuid"].get<std::string>();
				auto material = std::make_shared<MeshStandardMaterial>();

				auto name = package.find("name");
				auto colorMap = package.find("colorMap");
				auto opacityMap = package.find("opacityMap");
				auto normalMap = package.find("normalMap");
				auto roughnessMap = package.find("roughnessMap");
				auto specularMap = package.find("specularMap");
				auto metalnessMap = package.find("metalnessMap");
				auto emissiveMap = package.find("emissiveMap");
				auto anisotropyMap = package.find("anisotropyMap");
				auto clearCoatMap = package.find("clearCoatMap");
				auto clearCoatRoughnessMap = package.find("clearCoatRoughnessMap");
				auto subsurfaceMap = package.find("subsurfaceMap");
				auto subsurfaceColorMap = package.find("subsurfaceColorMap");
				auto sheenMap = package.find("sheenMap");
				auto lightMap = package.find("lightMap");

				if (name != package.end() && (*name).is_string())
					material->setName((*name).get<std::string>());
				if (colorMap != package.end() && (*colorMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*colorMap);
					if (texture) texture->apply();
					material->setColorMap(texture);
				}
				if (opacityMap != package.end() && (*opacityMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*opacityMap);
					if (texture) texture->apply();
					material->setOpacityMap(texture);
				}
				if (normalMap != package.end() && (*normalMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*normalMap);
					if (texture) texture->apply();
					material->setNormalMap(texture);
				}
				if (roughnessMap != package.end() && (*roughnessMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*roughnessMap);
					if (texture) texture->apply();
					material->setRoughnessMap(texture);
				}
				if (specularMap != package.end() && (*specularMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*specularMap);
					if (texture) texture->apply();
					material->setSpecularMap(texture);
				}
				if (metalnessMap != package.end() && (*metalnessMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*metalnessMap);
					if (texture) texture->apply();
					material->setMetalnessMap(texture);
				}
				if (emissiveMap != package.end() && (*emissiveMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*emissiveMap);
					if (texture) texture->apply();
					material->setEmissiveMap(texture);
				}
				if (anisotropyMap != package.end() && (*anisotropyMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*anisotropyMap);
					if (texture) texture->apply();
					material->setAnisotropyMap(texture);
				}
				if (clearCoatMap != package.end() && (*clearCoatMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*clearCoatMap);
					if (texture) texture->apply();
					material->setClearCoatMap(texture);
				}
				if (clearCoatRoughnessMap != package.end() && (*clearCoatRoughnessMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*clearCoatRoughnessMap);
					if (texture) texture->apply();
					material->setClearCoatRoughnessMap(texture);
				}
				if (subsurfaceMap != package.end() && (*subsurfaceMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*subsurfaceMap);
					if (texture) texture->apply();
					material->setSubsurfaceMap(texture);
				}
				if (subsurfaceColorMap != package.end() && (*subsurfaceColorMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*subsurfaceColorMap);
					if (texture) texture->apply();
					material->setSubsurfaceColorMap(texture);
				}
				if (sheenMap != package.end() && (*sheenMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*sheenMap);
					if (texture) texture->apply();
					material->setSheenMap(texture);
				}
				if (lightMap != package.end() && (*lightMap).is_string())
				{
					auto texture = AssetBundle::instance()->loadAsset<Texture>(*lightMap);
					if (texture) texture->apply();
					material->setLightMap(texture);
				}

				auto blendEnable = package.find("blendEnable");
				auto blendOp = package.find("blendOp");
				auto blendSrc = package.find("blendSrc");
				auto blendDest = package.find("blendDest");
				auto blendAlphaOp = package.find("blendAlphaOp");
				auto blendAlphaSrc = package.find("blendAlphaSrc");
				auto blendAlphaDest = package.find("blendAlphaDest");

				if (blendEnable != package.end() && (*blendEnable).is_boolean())
					material->setBlendEnable((*blendEnable).get<nlohmann::json::boolean_t>());
				if (blendOp != package.end() && (*blendOp).is_number_unsigned())
					material->setBlendOp((BlendOp)(*blendOp).get<nlohmann::json::number_unsigned_t>());
				if (blendSrc != package.end() && (*blendSrc).is_number_unsigned())
					material->setBlendSrc((BlendMode)(*blendSrc).get<nlohmann::json::number_unsigned_t>());
				if (blendDest != package.end() && (*blendDest).is_number_unsigned())
					material->setBlendDest((BlendMode)(*blendDest).get<nlohmann::json::number_unsigned_t>());
				if (blendAlphaOp != package.end() && (*blendAlphaOp).is_number_unsigned())
					material->setBlendAlphaOp((BlendOp)(*blendAlphaOp).get<nlohmann::json::number_unsigned_t>());
				if (blendAlphaSrc != package.end() && (*blendAlphaSrc).is_number_unsigned())
					material->setBlendAlphaSrc((BlendMode)(*blendAlphaSrc).get<nlohmann::json::number_unsigned_t>());
				if (blendAlphaDest != package.end() && (*blendAlphaDest).is_number_unsigned())
					material->setBlendAlphaDest((BlendMode)(*blendAlphaDest).get<nlohmann::json::number_unsigned_t>());

				auto depthEnable = package.find("depthEnable");
				auto depthBiasEnable = package.find("depthBiasEnable");
				auto depthBoundsEnable = package.find("depthBoundsEnable");
				auto depthClampEnable = package.find("depthClampEnable");
				auto depthWriteEnable = package.find("depthWriteEnable");
				auto stencilEnable = package.find("stencilEnable");
				auto scissorTestEnable = package.find("scissorTestEnable");

				if (depthEnable != package.end() && (*depthEnable).is_boolean())
					material->setDepthEnable((*depthEnable).get<nlohmann::json::boolean_t>());
				if (depthBiasEnable != package.end() && (*depthBiasEnable).is_boolean())
					material->setDepthBiasEnable((*depthBiasEnable).get<nlohmann::json::boolean_t>());
				if (depthBoundsEnable != package.end() && (*depthBoundsEnable).is_boolean())
					material->setDepthBoundsEnable((*depthBoundsEnable).get<nlohmann::json::boolean_t>());
				if (depthClampEnable != package.end() && (*depthClampEnable).is_boolean())
					material->setDepthClampEnable((*depthClampEnable).get<nlohmann::json::boolean_t>());
				if (depthWriteEnable != package.end() && (*depthWriteEnable).is_boolean())
					material->setDepthWriteEnable((*depthWriteEnable).get<nlohmann::json::boolean_t>());
				if (stencilEnable != package.end() && (*stencilEnable).is_boolean())
					material->setStencilEnable((*stencilEnable).get<nlohmann::json::boolean_t>());
				if (scissorTestEnable != package.end() && (*scissorTestEnable).is_boolean())
					material->setScissorTestEnable((*scissorTestEnable).get<nlohmann::json::boolean_t>());

				auto emissiveIntensity = package.find("emissiveIntensity");
				auto opacity = package.find("opacity");
				auto smoothness = package.find("smoothness");
				auto roughness = package.find("roughness");
				auto metalness = package.find("metalness");
				auto anisotropy = package.find("anisotropy");
				auto sheen = package.find("sheen");
				auto specular = package.find("specular");
				auto refractionRatio = package.find("refractionRatio");
				auto clearCoat = package.find("clearCoat");
				auto clearCoatRoughness = package.find("clearCoatRoughness");
				auto subsurface = package.find("subsurface");
				auto reflectionRatio = package.find("reflectionRatio");
				auto transmission = package.find("transmission");
				auto lightMapIntensity = package.find("lightMapIntensity");
				auto gamma = package.find("gamma");
				auto depthMin = package.find("depthMin");
				auto depthMax = package.find("depthMax");
				auto depthBias = package.find("depthBias");
				auto depthSlopeScaleBias = package.find("depthSlopeScaleBias");

				if (emissiveIntensity != package.end() && (*emissiveIntensity).is_number_float())
					material->setEmissiveIntensity((*emissiveIntensity).get<nlohmann::json::number_float_t>());
				if (opacity != package.end() && (*opacity).is_number_float())
					material->setOpacity((*opacity).get<nlohmann::json::number_float_t>());
				if (smoothness != package.end() && (*smoothness).is_number_float())
					material->setSmoothness((*smoothness).get<nlohmann::json::number_float_t>());
				if (roughness != package.end() && (*roughness).is_number_float())
					material->setRoughness((*roughness).get<nlohmann::json::number_float_t>());
				if (metalness != package.end() && (*metalness).is_number_float())
					material->setMetalness((*metalness).get<nlohmann::json::number_float_t>());
				if (anisotropy != package.end() && (*anisotropy).is_number_float())
					material->setAnisotropy((*anisotropy).get<nlohmann::json::number_float_t>());
				if (sheen != package.end() && (*sheen).is_number_float())
					material->setSheen((*sheen).get<nlohmann::json::number_float_t>());
				if (specular != package.end() && (*specular).is_number_float())
					material->setSpecular((*specular).get<nlohmann::json::number_float_t>());
				if (refractionRatio != package.end() && (*refractionRatio).is_number_float())
					material->setRefractionRatio((*refractionRatio).get<nlohmann::json::number_float_t>());
				if (clearCoat != package.end() && (*clearCoat).is_number_float())
					material->setClearCoat((*clearCoat).get<nlohmann::json::number_float_t>());
				if (clearCoatRoughness != package.end() && (*clearCoatRoughness).is_number_float())
					material->setClearCoatRoughness((*clearCoatRoughness).get<nlohmann::json::number_float_t>());
				if (subsurface != package.end() && (*subsurface).is_number_float())
					material->setSubsurface((*subsurface).get<nlohmann::json::number_float_t>());
				if (reflectionRatio != package.end() && (*reflectionRatio).is_number_float())
					material->setReflectionRatio((*reflectionRatio).get<nlohmann::json::number_float_t>());
				if (transmission != package.end() && (*transmission).is_number_float())
					material->setTransmission((*transmission).get<nlohmann::json::number_float_t>());
				if (lightMapIntensity != package.end() && (*lightMapIntensity).is_number_float())
					material->setLightMapIntensity((*lightMapIntensity).get<nlohmann::json::number_float_t>());
				if (gamma != package.end() && (*gamma).is_number_float())
					material->setGamma((*gamma).get<nlohmann::json::number_float_t>());
				if (depthMin != package.end() && (*depthMin).is_number_float())
					material->setDepthMin((*depthMin).get<nlohmann::json::number_float_t>());
				if (depthMax != package.end() && (*depthMax).is_number_float())
					material->setDepthMax((*depthMax).get<nlohmann::json::number_float_t>());
				if (depthBias != package.end() && (*depthBias).is_number_float())
					material->setDepthBias((*depthBias).get<nlohmann::json::number_float_t>());
				if (depthSlopeScaleBias != package.end() && (*depthSlopeScaleBias).is_number_float())
					material->setDepthSlopeScaleBias((*depthSlopeScaleBias).get<nlohmann::json::number_float_t>());

				auto offset = package.find("offset");
				auto repeat = package.find("repeat");
				auto normalScale = package.find("normalScale");
				auto color = package.find("color");
				auto emissive = package.find("emissive");
				auto subsurfaceColor = package.find("subsurfaceColor");

				if (offset != package.end() && (*offset).is_array())
					material->setOffset(math::float2((*offset).get<std::array<float, 2>>()));
				if (repeat != package.end() && (*repeat).is_array())
					material->setRepeat(math::float2((*repeat).get<std::array<float, 2>>()));
				if (normalScale != package.end() && (*normalScale).is_array())
					material->setNormalScale(math::float2((*normalScale).get<std::array<float, 2>>()));
				if (color != package.end() && (*color).is_array())
					material->setColor(math::float3((*color).get<std::array<float, 3>>()));
				if (emissive != package.end() && (*emissive).is_array())
					material->setEmissive(math::float3((*emissive).get<std::array<float, 3>>()));
				if (subsurfaceColor != package.end() && (*subsurfaceColor).is_array())
					material->setSubsurfaceColor(math::float3((*subsurfaceColor).get<std::array<float, 3>>()));

				this->assetList_[material] = package;
				this->assetGuidList_[material] = uuid;
				this->packageList_[uuid] = package;

				return material;
			}
		}

		return nullptr;
	}

	void
	AssetDatabase::addUpdateList(std::string_view uuid) noexcept(false)
	{
		this->updateList_.insert(std::string(uuid));
	}

	bool
	AssetDatabase::needUpdate(std::string_view uuid) const noexcept
	{
		return this->updateList_.find(std::string(uuid)) != this->updateList_.end();
	}

	void
	AssetDatabase::removeUpdateList(std::string_view uuid) noexcept(false)
	{
		auto it = this->updateList_.find(std::string(uuid));
		if (it != this->updateList_.end())
			this->updateList_.erase(it);
	}

	void
	AssetDatabase::clearUpdate() noexcept
	{
		this->updateList_.clear();
	}

	const std::set<std::string>&
	AssetDatabase::getUpdateList() const noexcept
	{
		return this->updateList_;
	}
}