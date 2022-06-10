#include <octoon/asset_database.h>
#include <octoon/runtime/uuid.h>
#include <octoon/vmd_loader.h>
#include <octoon/pmx_loader.h>
#include <octoon/ass_loader.h>
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
		: previewWidth_(256)
		, previewHeight_(256)
	{
	}

	AssetDatabase::~AssetDatabase() noexcept
	{
	}

	void
	AssetDatabase::open() noexcept(false)
	{
		this->initMaterialScene();
	}

	void
	AssetDatabase::close() noexcept
	{
		camera_.reset();
		geometry_.reset();
		directionalLight_.reset();
		environmentLight_.reset();
		scene_.reset();
	}

	nlohmann::json
	AssetDatabase::createAsset(std::string_view filepath, std::string_view path) noexcept(false)
	{
		std::wstring u16_conv = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes((char*)std::string(filepath).data());

		if (std::filesystem::exists(u16_conv))
		{
			auto uuid = octoon::make_guid();
			auto extension = filepath.substr(filepath.find_last_of("."));
			auto rootPath = std::filesystem::path(path).append(uuid);
			auto motionPath = std::filesystem::path(rootPath).append(uuid + std::string(extension));
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directory(path);
			std::filesystem::create_directory(rootPath);
			std::filesystem::copy(u16_conv, motionPath);
			std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

			auto filename = std::filesystem::path(u16_conv).filename().u8string();

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = (char*)filename.substr(0, filename.find_last_of('.')).c_str();
			package["path"] = (char*)motionPath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetDatabase::createAsset(const octoon::Texture& texture, std::string_view path) noexcept(false)
	{
		assert(!path.empty());

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		auto uuid = make_guid();
		auto filename = texture.getName().substr(texture.getName().find_last_of("."));
		auto rootPath = std::filesystem::path(path).append(uuid);
		auto texturePath = std::filesystem::path(rootPath).append(uuid + filename);
		auto packagePath = std::filesystem::path(rootPath).append("package.json");

		std::filesystem::create_directories(rootPath);

		auto outputPath = texturePath.string();
		auto extension = outputPath.substr(outputPath.find_last_of(".") + 1);
		texture.save(outputPath, extension.c_str());
		std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);

		nlohmann::json package;
		package["uuid"] = uuid;
		package["visible"] = true;
		package["name"] = (char*)std::filesystem::path(cv.from_bytes(texture.getName())).filename().u8string().c_str();
		package["path"] = (char*)texturePath.u8string().c_str();
		package["mipmap"] = texture.getMipLevel() > 1;

		if (filename == ".hdr")
		{
			auto name = texture.getName();
			auto width = texture.width();
			auto height = texture.height();
			float* data_ = (float*)texture.data();

			auto size = width * height * 3;
			auto pixels = std::make_unique<std::uint8_t[]>(size);

			for (std::size_t i = 0; i < size; i += 3)
			{
				pixels[i] = std::clamp<float>(std::pow(data_[i], 1.0f / 2.2f) * 255.0f, 0, 255);
				pixels[i + 1] = std::clamp<float>(std::pow(data_[i + 1], 1.0f / 2.2f) * 255.0f, 0, 255);
				pixels[i + 2] = std::clamp<float>(std::pow(data_[i + 2], 1.0f / 2.2f) * 255.0f, 0, 255);
			}

			auto uuid2 = octoon::make_guid();
			auto texturePath2 = std::filesystem::path(rootPath).append(uuid2 + ".png");

			octoon::Texture image(octoon::Format::R8G8B8SRGB, width, height, pixels.get());
			image.resize(260, 130).save(texturePath2.string(), "png");

			package["preview"] = texturePath2.string();
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
	AssetDatabase::createAsset(const octoon::Animation& animation, std::string_view path) noexcept(false)
	{
		assert(!path.empty());

		auto uuid = make_guid();
		auto rootPath = std::filesystem::path(path).append(uuid);
		auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
		auto packagePath = std::filesystem::path(rootPath).append("package.json");

		std::filesystem::create_directories(rootPath);

		octoon::VMDLoader::save(motionPath.string(), animation);
		std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

		nlohmann::json package;
		package["uuid"] = uuid;
		package["visible"] = true;
		package["name"] = uuid + ".vmd";
		package["path"] = (char*)motionPath.u8string().c_str();

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
	AssetDatabase::createAsset(const std::shared_ptr<Material>& material, std::string_view path) noexcept(false)
	{
		auto uuid = make_guid();
		auto outputPath = std::filesystem::path(path).append(uuid);
		auto outputTexturePath = std::filesystem::path(path);

		std::filesystem::create_directories(outputPath);

		try
		{
			auto writePreview = [this](const std::shared_ptr<Material>& material, std::filesystem::path path) -> nlohmann::json
			{
				Texture texture;
				auto uuid = make_guid();
				auto previewPath = std::filesystem::path(path).append(uuid + ".png");
				this->createMaterialPreview(material, texture);
				texture.save(previewPath.string(), "png");
				return previewPath.string();
			};

			auto writeFloat2 = [](const octoon::math::float2& v)
			{
				return nlohmann::json({ v.x, v.y });
			};

			auto writeFloat3 = [](const octoon::math::float3& v)
			{
				return nlohmann::json({ v.x, v.y, v.z });
			};

			auto standardMaterial = material->downcast<octoon::MeshStandardMaterial>();

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = standardMaterial->getName();
			package["visible"] = true;
			package["preview"] = writePreview(material, outputPath);
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
			package["gamma"] = standardMaterial->getGamma();
			package["offset"] = writeFloat2(standardMaterial->getOffset());
			package["repeat"] = writeFloat2(standardMaterial->getRepeat());
			package["normalScale"] = writeFloat2(standardMaterial->getNormalScale());
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
				package["colorMap"] = this->createAsset(*standardMaterial->getColorMap(), outputPath.string());
			if (standardMaterial->getOpacityMap())
				package["opacityMap"] = this->createAsset(*standardMaterial->getOpacityMap(), outputPath.string());
			if (standardMaterial->getNormalMap())
				package["normalMap"] = this->createAsset(*standardMaterial->getNormalMap(), outputPath.string());
			if (standardMaterial->getRoughnessMap())
				package["roughnessMap"] = this->createAsset(*standardMaterial->getRoughnessMap(), outputPath.string());
			if (standardMaterial->getSpecularMap())
				package["specularMap"] = this->createAsset(*standardMaterial->getSpecularMap(), outputPath.string());
			if (standardMaterial->getMetalnessMap())
				package["metalnessMap"] = this->createAsset(*standardMaterial->getMetalnessMap(), outputPath.string());
			if (standardMaterial->getEmissiveMap())
				package["emissiveMap"] = this->createAsset(*standardMaterial->getEmissiveMap(), outputPath.string());
			if (standardMaterial->getAnisotropyMap())
				package["anisotropyMap"] = this->createAsset(*standardMaterial->getAnisotropyMap(), outputPath.string());
			if (standardMaterial->getClearCoatMap())
				package["clearCoatMap"] = this->createAsset(*standardMaterial->getClearCoatMap(), outputPath.string());
			if (standardMaterial->getClearCoatRoughnessMap())
				package["clearCoatRoughnessMap"] = this->createAsset(*standardMaterial->getClearCoatRoughnessMap(), outputPath.string());
			if (standardMaterial->getSubsurfaceMap())
				package["subsurfaceMap"] = this->createAsset(*standardMaterial->getSubsurfaceMap(), outputPath.string());
			if (standardMaterial->getSubsurfaceColorMap())
				package["subsurfaceColorMap"] = this->createAsset(*standardMaterial->getSubsurfaceColorMap(), outputPath.string());
			if (standardMaterial->getSheenMap())
				package["sheenMap"] = this->createAsset(*standardMaterial->getSheenMap(), outputPath.string());
			if (standardMaterial->getLightMap())
				package["lightMap"] = this->createAsset(*standardMaterial->getLightMap(), outputPath.string());
			if (standardMaterial->getEmissiveIntensity())
				package["emissiveIntensity"] = standardMaterial->getEmissiveIntensity();

			std::ofstream ifs(std::filesystem::path(outputPath).append("package.json"), std::ios_base::binary);
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
			std::filesystem::remove_all(outputPath);
			throw e;
		}
	}

	std::string
	AssetDatabase::getAssetPath(const std::shared_ptr<RttiObject>& asset) noexcept
	{
		if (assetPathList_.contains(asset))
			return assetPathList_.at(asset);
		return std::string();
	}

	std::string
	AssetDatabase::getAssetPath(const std::shared_ptr<RttiObject>& asset) const noexcept
	{
		if (assetPathList_.contains(asset))
			return assetPathList_.at(asset);
		return std::string();
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<RttiObject>& asset) noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		else
		{
			auto guid = octoon::make_guid();
			assetGuidList_[asset] = guid;
			return guid;
		}
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<RttiObject>& asset) const noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		return std::string();
	}

	nlohmann::json
	AssetDatabase::getPackage(std::string_view uuid, std::string_view outputPath) noexcept
	{
		assert(!outputPath.empty());

		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(outputPath).append(uuid).append("package.json"));
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
					auto path = package["path"].get<nlohmann::json::string_t>();
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
	AssetDatabase::loadAssetAtPath(std::string_view path) noexcept(false)
	{
		auto ext = std::string(path.substr(path.find_last_of('.')));
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == ".vmd")
		{
			auto motion = octoon::VMDLoader::load(path);
			if (motion)
			{
				assetPathList_[motion] = path;
				assetGuidList_[motion] = make_guid();
				return motion;
			}
		}
		else if (ext == ".hdr" || ext == ".bmp" || ext == ".tga" || ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".dds")
		{
			auto texture = std::make_shared<octoon::Texture>((std::string)path);
			if (texture)
			{
				assetPathList_[texture] = path;
				assetGuidList_[texture] = make_guid();
				return texture;
			}
		}
		else if (ext == ".pmx")
		{
			auto model = PMXLoader::load(path, PMXLoadFlagBits::AllBit);
			if (model)
			{
				assetPathList_[model] = path;
				assetGuidList_[model] = make_guid();
				return model;
			}
		}
		else if (ext == ".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				model->addComponent<MeshAnimationComponent>(path);
				assetPathList_[model] = path;
				assetGuidList_[model] = make_guid();
				return model;
			}
		}

		return nullptr;
	}

	std::shared_ptr<RttiObject>
	AssetDatabase::loadAssetAtPath(std::string_view path, octoon::PMXLoadFlags flags) noexcept(false)
	{
		auto ext = path.substr(path.find_last_of("."));
		if (ext == ".pmx")
		{
			auto model = octoon::PMXLoader::load(path, flags);
			if (model)
			{
				assetPathList_[model] = path;
				assetGuidList_[model] = make_guid();
				return model;
			}
		}
		else if (ext == ".abc")
		{
			auto model = std::make_shared<octoon::GameObject>();
			if (model)
			{
				model->addComponent<octoon::MeshAnimationComponent>(path);
				assetPathList_[model] = path;
				assetGuidList_[model] = make_guid();
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
			if (package.find("path") != package.end())
			{
				auto path = package["path"].get<nlohmann::json::string_t>();
				auto uuid = package["uuid"].get<nlohmann::json::string_t>();
				auto it = this->assetCache_.find(uuid);
				if (it != this->assetCache_.end())
					return this->assetCache_[uuid]->downcast_pointer<octoon::Texture>();

				bool generateMipmap = false;
				if (package.find("mipmap") != package.end())
					generateMipmap = package["mipmap"].get<nlohmann::json::boolean_t>();

				auto texture = std::make_shared<octoon::Texture>(path);
				if (texture)
				{
					if (generateMipmap)
						texture->setMipLevel(8);

					texture->apply();

					packageList_[uuid] = package;
					assetCache_[uuid] = texture;
					assetList_[texture] = package;
					assetPathList_[texture] = path;
					assetGuidList_[texture] = uuid;

					return texture;
				}
			}
		}
		else if (type.isDerivedFrom(Animation::getRtti()))
		{
			if (package["path"].is_string())
			{
				auto uuid = package["uuid"].get<nlohmann::json::string_t>();
				auto it = this->assetCache_.find(uuid);
				if (it != this->assetCache_.end())
					return this->assetCache_[uuid]->downcast_pointer<octoon::Animation>();

				auto path = package["path"].get<nlohmann::json::string_t>();
				auto motion = octoon::VMDLoader::load(path.c_str());
				if (motion)
				{
					packageList_[uuid] = package;
					assetCache_[uuid] = motion;
					assetList_[motion] = package;
					assetPathList_[motion] = path;
					assetGuidList_[motion] = uuid;

					return motion;
				}
			}
		}
		else if (type.isDerivedFrom(Material::getRtti()))
		{
			auto uuid = package["uuid"].get<nlohmann::json::string_t>();
			auto it = this->assetCache_.find(uuid);
			if (it != this->assetCache_.end())
				return this->assetCache_[uuid]->downcast_pointer<octoon::Material>();

			auto material = std::make_shared<octoon::MeshStandardMaterial>();

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
				material->setName((*name).get<nlohmann::json::string_t>());
			if (colorMap != package.end() && (*colorMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*colorMap);
				if (texture) texture->apply();
				material->setColorMap(texture);
			}
			if (opacityMap != package.end() && (*opacityMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*opacityMap);
				if (texture) texture->apply();
				material->setOpacityMap(texture);
			}
			if (normalMap != package.end() && (*normalMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*normalMap);
				if (texture) texture->apply();
				material->setNormalMap(texture);
			}
			if (roughnessMap != package.end() && (*roughnessMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*roughnessMap);
				if (texture) texture->apply();
				material->setRoughnessMap(texture);
			}
			if (specularMap != package.end() && (*specularMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*specularMap);
				if (texture) texture->apply();
				material->setSpecularMap(texture);
			}
			if (metalnessMap != package.end() && (*metalnessMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*metalnessMap);
				if (texture) texture->apply();
				material->setMetalnessMap(texture);
			}
			if (emissiveMap != package.end() && (*emissiveMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*emissiveMap);
				if (texture) texture->apply();
				material->setEmissiveMap(texture);
			}
			if (anisotropyMap != package.end() && (*anisotropyMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*anisotropyMap);
				if (texture) texture->apply();
				material->setAnisotropyMap(texture);
			}
			if (clearCoatMap != package.end() && (*clearCoatMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*clearCoatMap);
				if (texture) texture->apply();
				material->setClearCoatMap(texture);
			}
			if (clearCoatRoughnessMap != package.end() && (*clearCoatRoughnessMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*clearCoatRoughnessMap);
				if (texture) texture->apply();
				material->setClearCoatRoughnessMap(texture);
			}
			if (subsurfaceMap != package.end() && (*subsurfaceMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*subsurfaceMap);
				if (texture) texture->apply();
				material->setSubsurfaceMap(texture);
			}
			if (subsurfaceColorMap != package.end() && (*subsurfaceColorMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*subsurfaceColorMap);
				if (texture) texture->apply();
				material->setSubsurfaceColorMap(texture);
			}
			if (sheenMap != package.end() && (*sheenMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*sheenMap);
				if (texture) texture->apply();
				material->setSheenMap(texture);
			}
			if (lightMap != package.end() && (*lightMap).is_object())
			{
				auto texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<Texture>(*lightMap);
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
				material->setBlendOp((octoon::BlendOp)(*blendOp).get<nlohmann::json::number_unsigned_t>());
			if (blendSrc != package.end() && (*blendSrc).is_number_unsigned())
				material->setBlendSrc((octoon::BlendMode)(*blendSrc).get<nlohmann::json::number_unsigned_t>());
			if (blendDest != package.end() && (*blendDest).is_number_unsigned())
				material->setBlendDest((octoon::BlendMode)(*blendDest).get<nlohmann::json::number_unsigned_t>());
			if (blendAlphaOp != package.end() && (*blendAlphaOp).is_number_unsigned())
				material->setBlendAlphaOp((octoon::BlendOp)(*blendAlphaOp).get<nlohmann::json::number_unsigned_t>());
			if (blendAlphaSrc != package.end() && (*blendAlphaSrc).is_number_unsigned())
				material->setBlendAlphaSrc((octoon::BlendMode)(*blendAlphaSrc).get<nlohmann::json::number_unsigned_t>());
			if (blendAlphaDest != package.end() && (*blendAlphaDest).is_number_unsigned())
				material->setBlendAlphaDest((octoon::BlendMode)(*blendAlphaDest).get<nlohmann::json::number_unsigned_t>());

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
				material->setOffset(octoon::math::float2((*offset)[0].get<nlohmann::json::number_float_t>(), (*offset)[1].get<nlohmann::json::number_float_t>()));
			if (repeat != package.end() && (*repeat).is_array())
				material->setRepeat(octoon::math::float2((*repeat)[0].get<nlohmann::json::number_float_t>(), (*repeat)[1].get<nlohmann::json::number_float_t>()));
			if (normalScale != package.end() && (*normalScale).is_array())
				material->setNormalScale(octoon::math::float2((*normalScale)[0].get<nlohmann::json::number_float_t>(), (*normalScale)[1].get<nlohmann::json::number_float_t>()));
			if (color != package.end() && (*color).is_array())
				material->setColor(octoon::math::float3((*color)[0].get<nlohmann::json::number_float_t>(), (*color)[1].get<nlohmann::json::number_float_t>(), (*color)[2].get<nlohmann::json::number_float_t>()));
			if (emissive != package.end() && (*emissive).is_array())
				material->setEmissive(octoon::math::float3((*emissive)[0].get<nlohmann::json::number_float_t>(), (*emissive)[1].get<nlohmann::json::number_float_t>(), (*emissive)[2].get<nlohmann::json::number_float_t>()));
			if (subsurfaceColor != package.end() && (*subsurfaceColor).is_array())
				material->setSubsurfaceColor(octoon::math::float3((*subsurfaceColor)[0].get<nlohmann::json::number_float_t>(), (*subsurfaceColor)[1].get<nlohmann::json::number_float_t>(), (*subsurfaceColor)[2].get<nlohmann::json::number_float_t>()));

			packageList_[uuid] = package;
			assetCache_[uuid] = material;
			assetList_[material] = package;
			assetGuidList_[material] = uuid;
			this->packageList_[uuid] = package;
			this->assetCache_[uuid] = material;
			this->assetList_[material] = package;

			return material;
		}

		return nullptr;
	}

	std::shared_ptr<octoon::GraphicsTexture>
	AssetDatabase::createMaterialPreview(const std::shared_ptr<Material>& material)
	{
		assert(scene_);

		auto renderer = octoon::Renderer::instance();
		if (renderer)
		{
			geometry_->setMaterial(material);
			renderer->render(scene_);
			material->setDirty(true);
		}

		auto framebufferDesc = framebuffer_->getFramebufferDesc();
		return framebufferDesc.getColorAttachment(0).getBindingTexture();
	}

	void
	AssetDatabase::createMaterialPreview(const std::shared_ptr<Material>& material, octoon::Texture& texture)
	{
		auto colorTexture = this->createMaterialPreview(material);
		auto width = colorTexture->getTextureDesc().getWidth();
		auto height = colorTexture->getTextureDesc().getHeight();

		std::uint8_t* data;
		if (colorTexture->map(0, 0, width, height, 0, (void**)&data))
		{
			texture.create(octoon::Format::R8G8B8SRGB, width, height);

			auto destData = texture.data();

			constexpr auto size = 16;

			for (std::uint32_t y = 0; y < height; y++)
			{
				for (std::uint32_t x = 0; x < width; x++)
				{
					auto src = (y * height + x) * 4;
					auto dest = (y * height + x) * 3;

					std::uint8_t u = x / size % 2;
					std::uint8_t v = y / size % 2;
					std::uint8_t bg = (u == 0 && v == 0 || u == v) ? 200u : 255u;

					destData[dest] = octoon::math::lerp(bg, data[src], data[src + 3] / 255.f);
					destData[dest + 1] = octoon::math::lerp(bg, data[src + 1], data[src + 3] / 255.f);
					destData[dest + 2] = octoon::math::lerp(bg, data[src + 2], data[src + 3] / 255.f);
				}
			}

			colorTexture->unmap();
		}
	}

	void
	AssetDatabase::initMaterialScene() noexcept(false)
	{
		auto renderer = octoon::Renderer::instance();
		if (renderer)
		{
			std::uint32_t width = previewWidth_;
			std::uint32_t height = previewHeight_;

			octoon::GraphicsTextureDesc textureDesc;
			textureDesc.setSize(width, height);
			textureDesc.setTexDim(octoon::TextureDimension::Texture2D);
			textureDesc.setTexFormat(octoon::GraphicsFormat::R8G8B8A8UNorm);
			auto colorTexture = renderer->getGraphicsDevice()->createTexture(textureDesc);
			if (!colorTexture)
				throw std::runtime_error("createTexture() failed");

			octoon::GraphicsTextureDesc depthTextureDesc;
			depthTextureDesc.setSize(width, height);
			depthTextureDesc.setTexDim(octoon::TextureDimension::Texture2D);
			depthTextureDesc.setTexFormat(octoon::GraphicsFormat::D16UNorm);
			auto depthTexture = renderer->getGraphicsDevice()->createTexture(depthTextureDesc);
			if (!depthTexture)
				throw std::runtime_error("createTexture() failed");

			octoon::GraphicsFramebufferLayoutDesc framebufferLayoutDesc;
			framebufferLayoutDesc.addComponent(octoon::GraphicsAttachmentLayout(0, octoon::GraphicsImageLayout::ColorAttachmentOptimal, octoon::GraphicsFormat::R8G8B8A8UNorm));
			framebufferLayoutDesc.addComponent(octoon::GraphicsAttachmentLayout(1, octoon::GraphicsImageLayout::DepthStencilAttachmentOptimal, octoon::GraphicsFormat::D16UNorm));

			octoon::GraphicsFramebufferDesc framebufferDesc;
			framebufferDesc.setWidth(width);
			framebufferDesc.setHeight(height);
			framebufferDesc.setFramebufferLayout(renderer->getGraphicsDevice()->createFramebufferLayout(framebufferLayoutDesc));
			framebufferDesc.setDepthStencilAttachment(octoon::GraphicsAttachmentBinding(depthTexture, 0, 0));
			framebufferDesc.addColorAttachment(octoon::GraphicsAttachmentBinding(colorTexture, 0, 0));

			framebuffer_ = renderer->getGraphicsDevice()->createFramebuffer(framebufferDesc);
			if (!framebuffer_)
				throw std::runtime_error("createFramebuffer() failed");

			camera_ = std::make_shared<octoon::PerspectiveCamera>(60, 1, 100);
			camera_->setClearColor(octoon::math::float4::Zero);
			camera_->setClearFlags(octoon::ClearFlagBits::AllBit);
			camera_->setFramebuffer(framebuffer_);
			camera_->setTransform(octoon::math::makeLookatRH(octoon::math::float3(0, 0, 1), octoon::math::float3::Zero, octoon::math::float3::UnitY));

			geometry_ = std::make_shared<octoon::Geometry>();
			geometry_->setMesh(std::make_shared<octoon::SphereMesh>(0.5));

			octoon::math::Quaternion q1;
			q1.makeRotation(octoon::math::float3::UnitX, octoon::math::PI / 2.75);
			octoon::math::Quaternion q2;
			q2.makeRotation(octoon::math::float3::UnitY, octoon::math::PI / 4.6);

			directionalLight_ = std::make_shared<octoon::DirectionalLight>();
			directionalLight_->setColor(octoon::math::float3(1, 1, 1));
			directionalLight_->setTransform(octoon::math::float4x4(q1 * q2));

			environmentLight_ = std::make_shared<octoon::EnvironmentLight>();
			environmentLight_->setEnvironmentMap(octoon::PMREMLoader::load("../../system/hdri/Ditch-River_1k.hdr"));

			scene_ = std::make_unique<octoon::RenderScene>();
			scene_->addRenderObject(camera_.get());
			scene_->addRenderObject(directionalLight_.get());
			scene_->addRenderObject(environmentLight_.get());
			scene_->addRenderObject(geometry_.get());
		}
	}
}