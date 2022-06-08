#include "material_importer.h"

#include <octoon/mdl_loader.h>
#include <octoon/PMREM_loader.h>
#include <octoon/texture_loader.h>
#include <octoon/texture_importer.h>
#include <octoon/video_feature.h>
#include <octoon/environment_light_component.h>
#include <octoon/runtime/uuid.h>

#include <filesystem>
#include <fstream>

#include <qimage.h>

#include "../unreal_profile.h"
#include "../unreal_behaviour.h"

namespace unreal
{
	OctoonImplementSingleton(MaterialImporter)

	MaterialImporter::MaterialImporter() noexcept
		: previewWidth_(256)
		, previewHeight_(256)
	{
	}

	MaterialImporter::~MaterialImporter() noexcept
	{
	}

	void
	MaterialImporter::open(std::string indexPath) noexcept(false)
	{
		this->initMaterialScene();

		if (std::filesystem::exists(indexPath))
		{
			this->defaultPath_ = indexPath;
			this->initPackageIndices();
		}
	}

	void
	MaterialImporter::close() noexcept
	{
		camera_.reset();
		geometry_.reset();
		directionalLight_.reset();
		environmentLight_.reset();
		scene_.reset();
	}

	nlohmann::json
	MaterialImporter::createPackage(std::string_view path, std::string_view outputPath) noexcept(false)
	{
		octoon::io::ifstream stream((std::string)path);
		if (stream)
		{
			octoon::MDLLoader loader;
			loader.load("resource", stream);

			nlohmann::json items;

			for (auto& mat : loader.getMaterials())
			{
				auto package = this->createPackage(mat, outputPath);

				items.push_back(package["uuid"]);
				this->indexList_.getValue().push_back(package["uuid"]);
			}

			this->save();

			return items;
		}

		return nlohmann::json();
	}

	nlohmann::json
	MaterialImporter::createPackage(const std::shared_ptr<octoon::Material>& mat, std::string_view materialPath, std::string_view texturePath) noexcept(false)
	{
		auto it = this->materialPackageCache_.find(mat);
		if (it != this->materialPackageCache_.end())
			return this->materialPackageCache_[mat];

		auto uuid = octoon::make_guid();

		nlohmann::json package = this->getPackage(mat);
		if (package.find("uuid") != package.end())
		{
			uuid = package["uuid"].get<nlohmann::json::string_t>();
			for (auto& index : indexList_.getValue())
			{
				if (index == uuid)
					return package;
			}
		}

		auto outputPath = std::filesystem::path(materialPath.empty() ? defaultPath_ : materialPath).append(uuid);
		auto outputTexturePath = std::filesystem::path(texturePath.empty() ? defaultPath_ : texturePath);

		std::filesystem::create_directories(outputPath);

		try
		{
			auto writePreview = [this](const std::shared_ptr<octoon::Material> material, std::filesystem::path path) -> nlohmann::json
			{
				QPixmap pixmap;
				auto uuid = octoon::make_guid();
				auto previewPath = std::filesystem::path(path).append(uuid + ".png");
				this->createMaterialPreview(material, pixmap, previewWidth_, previewHeight_);
				pixmap.save(QString::fromStdString(previewPath.string()), "png");
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

			auto standardMaterial = mat->downcast<octoon::MeshStandardMaterial>();

			package["uuid"] = uuid;
			package["name"] = mat->getName();
			package["visible"] = true;
			package["preview"] = writePreview(mat, outputPath);
			package["colorMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getColorMap(), outputTexturePath.string());
			package["opacityMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getOpacityMap(), outputTexturePath.string());
			package["normalMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getNormalMap(), outputTexturePath.string());
			package["roughnessMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getRoughnessMap(), outputTexturePath.string());
			package["specularMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getSpecularMap(), outputTexturePath.string());
			package["metalnessMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getMetalnessMap(), outputTexturePath.string());
			package["emissiveMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getEmissiveMap(), outputTexturePath.string());
			package["anisotropyMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getAnisotropyMap(), outputTexturePath.string());
			package["clearCoatMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getClearCoatMap(), outputTexturePath.string());
			package["clearCoatRoughnessMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getClearCoatRoughnessMap(), outputTexturePath.string());
			package["subsurfaceMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getSubsurfaceMap(), outputTexturePath.string());
			package["subsurfaceColorMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getSubsurfaceColorMap(), outputTexturePath.string());
			package["sheenMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getSheenMap(), outputTexturePath.string());
			package["lightMap"] = octoon::TextureImporter::instance()->createPackage(standardMaterial->getLightMap(), outputTexturePath.string());
			package["emissiveIntensity"] = standardMaterial->getEmissiveIntensity();
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

			std::ofstream ifs(std::filesystem::path(outputPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			this->materialPackageCache_[mat] = package;
			this->packageList_[std::string(uuid)] = package;

			return package;
		}
		catch (std::exception& e)
		{
			std::filesystem::remove_all(outputPath);
			throw e;
		}
	}

	MutableLiveData<nlohmann::json>&
	MaterialImporter::getIndexList() noexcept
	{
		return this->indexList_;
	}

	MutableLiveData<nlohmann::json>&
	MaterialImporter::getSceneList() noexcept
	{
		return this->sceneList_;
	}

	const MutableLiveData<nlohmann::json>&
	MaterialImporter::getIndexList() const noexcept
	{
		return this->indexList_;
	}

	const MutableLiveData<nlohmann::json>&
	MaterialImporter::getSceneList() const noexcept
	{
		return this->sceneList_;
	}

	nlohmann::json
	MaterialImporter::getPackage(std::string_view uuid, std::string_view rootPath) noexcept(false)
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(rootPath.empty() ? defaultPath_ : rootPath).append(uuid).append("package.json"));
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

	std::shared_ptr<octoon::Material>
	MaterialImporter::loadPackage(std::string_view uuid, std::string_view rootPath) noexcept(false)
	{
		if (materials_.find(std::string(uuid)) != materials_.end())
			return materials_[std::string(uuid)];

		auto package = MaterialImporter::instance()->getPackage(uuid, rootPath);
		if (package.is_object())
			return loadPackage(package);

		return nullptr;
	}

	std::shared_ptr<octoon::Material>
	MaterialImporter::loadPackage(const nlohmann::json& package) noexcept(false)
	{
		auto uuid = package["uuid"].get<nlohmann::json::string_t>();
		auto it = this->materialCache_.find(uuid);
		if (it != this->materialCache_.end())
			return this->materialCache_[uuid];

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
			material->setColorMap(octoon::TextureImporter::instance()->loadPackage(*colorMap));
		if (opacityMap != package.end() && (*opacityMap).is_object())
			material->setOpacityMap(octoon::TextureImporter::instance()->loadPackage(*opacityMap));
		if (normalMap != package.end() && (*normalMap).is_object())
			material->setNormalMap(octoon::TextureImporter::instance()->loadPackage(*normalMap));
		if (roughnessMap != package.end() && (*roughnessMap).is_object())
			material->setRoughnessMap(octoon::TextureImporter::instance()->loadPackage(*roughnessMap));
		if (specularMap != package.end() && (*specularMap).is_object())
			material->setSpecularMap(octoon::TextureImporter::instance()->loadPackage(*specularMap));
		if (metalnessMap != package.end() && (*metalnessMap).is_object())
			material->setMetalnessMap(octoon::TextureImporter::instance()->loadPackage(*metalnessMap));
		if (emissiveMap != package.end() && (*emissiveMap).is_object())
			material->setEmissiveMap(octoon::TextureImporter::instance()->loadPackage(*emissiveMap));
		if (anisotropyMap != package.end() && (*anisotropyMap).is_object())
			material->setAnisotropyMap(octoon::TextureImporter::instance()->loadPackage(*anisotropyMap));
		if (clearCoatMap != package.end() && (*clearCoatMap).is_object())
			material->setClearCoatMap(octoon::TextureImporter::instance()->loadPackage(*clearCoatMap));
		if (clearCoatRoughnessMap != package.end() && (*clearCoatRoughnessMap).is_object())
			material->setClearCoatRoughnessMap(octoon::TextureImporter::instance()->loadPackage(*clearCoatRoughnessMap));
		if (subsurfaceMap != package.end() && (*subsurfaceMap).is_object())
			material->setSubsurfaceMap(octoon::TextureImporter::instance()->loadPackage(*subsurfaceMap));
		if (subsurfaceColorMap != package.end() && (*subsurfaceColorMap).is_object())
			material->setSubsurfaceColorMap(octoon::TextureImporter::instance()->loadPackage(*subsurfaceColorMap));
		if (sheenMap != package.end() && (*sheenMap).is_object())
			material->setSheenMap(octoon::TextureImporter::instance()->loadPackage(*sheenMap));
		if (lightMap != package.end() && (*lightMap).is_object())
			material->setLightMap(octoon::TextureImporter::instance()->loadPackage(*lightMap));

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

		this->packageList_[uuid] = package;
		this->materialCache_[uuid] = material;
		this->materialList_[material] = package;

		return material;
	}

	void
	MaterialImporter::removePackage(std::string_view uuid) noexcept(false)
	{
		auto& indexList = indexList_.getValue();

		for (auto index = indexList.begin(); index != indexList.end(); ++index)
		{
			if ((*index).get<nlohmann::json::string_t>() == uuid)
			{
				auto packagePath = std::filesystem::path(defaultPath_).append(uuid);

				for (auto& it : std::filesystem::recursive_directory_iterator(packagePath))
					std::filesystem::permissions(it, std::filesystem::perms::owner_write);

				std::filesystem::remove_all(packagePath);

				auto package = this->packageList_.find(std::string(uuid));
				if (package != this->packageList_.end())
					this->packageList_.erase(package);

				indexList.erase(index);
				break;
			}
		}
	}

	bool
	MaterialImporter::addMaterial(const std::shared_ptr<octoon::Material>& mat)
	{
		auto it = this->materialList_.find(mat);
		if (it == this->materialList_.end())
		{
			auto standard = mat->downcast_pointer<octoon::MeshStandardMaterial>();
			auto uuid = octoon::make_guid();

			auto& color = standard->getColor();

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = mat->getName();
			package["color"] = { color.x, color.y, color.z };

			this->sceneList_.getValue().push_back(uuid);
			this->packageList_[uuid] = package;
			this->materials_[uuid] = mat;
			this->materialList_[mat] = package;

			return true;
		}
		else
		{
			auto uuid = (*it).second["uuid"].get<nlohmann::json::string_t>();
			auto& sceneList = sceneList_.getValue();

			bool found = false;
			for (auto index = sceneList.begin(); index != sceneList.end(); ++index)
			{
				if ((*index).get<nlohmann::json::string_t>() == uuid)
				{
					found = true;
					break;
				}
			}

			if (!found)
				sceneList.push_back(uuid);

			this->materials_[uuid] = mat;

			return true;
		}

		return false;
	}

	nlohmann::json
	MaterialImporter::getPackage(const std::shared_ptr<octoon::Material>& material) const noexcept(false)
	{
		if (material)
		{
			auto it = materialList_.find(material);
			if (it != materialList_.end())
			{
				auto& package = (*it).second;
				return package;
			}
		}
		return nlohmann::json();
	}

	void
	MaterialImporter::createMaterialPreview(const std::shared_ptr<octoon::Material>& material, QPixmap& pixmap, int w, int h)
	{
		assert(material);

		if (scene_)
		{
			auto renderer = octoon::Renderer::instance();
			if (renderer)
			{
				geometry_->setMaterial(material);
				renderer->render(scene_);
				material->setDirty(true);
			}

			auto framebufferDesc = framebuffer_->getFramebufferDesc();
			auto width = framebufferDesc.getWidth();
			auto height = framebufferDesc.getHeight();

			auto colorTexture = framebufferDesc.getColorAttachment(0).getBindingTexture();

			std::uint8_t* data;
			if (colorTexture->map(0, 0, framebufferDesc.getWidth(), framebufferDesc.getHeight(), 0, (void**)&data))
			{
				QImage image(width, height, QImage::Format_RGB888);

				constexpr auto size = 16;

				for (std::uint32_t y = 0; y < height; y++)
				{
					for (std::uint32_t x = 0; x < width; x++)
					{
						auto n = (y * height + x) * 4;

						std::uint8_t u = x / size % 2;
						std::uint8_t v = y / size % 2;
						std::uint8_t bg = (u == 0 && v == 0 || u == v) ? 200u : 255u;

						auto r = octoon::math::lerp(bg, data[n], data[n + 3] / 255.f);
						auto g = octoon::math::lerp(bg, data[n + 1], data[n + 3] / 255.f);
						auto b = octoon::math::lerp(bg, data[n + 2], data[n + 3] / 255.f);

						image.setPixelColor((int)x, (int)y, QColor::fromRgb(r, g, b));
					}
				}

				colorTexture->unmap();

				pixmap.convertFromImage(image);
				pixmap = pixmap.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			}
		}
	}

	void
	MaterialImporter::initMaterialScene() noexcept(false)
	{
		auto renderer = octoon::Renderer::instance();
		if (renderer)
		{
			std::uint32_t width = previewWidth_ * 2;
			std::uint32_t height = previewHeight_ * 2;

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
			geometry_->setMesh(octoon::SphereMesh::create(0.5));

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

	void
	MaterialImporter::clearCache() noexcept
	{
		materialCache_.clear();
		materialPackageCache_.clear();
	}

	void
	MaterialImporter::initPackageIndices() noexcept(false)
	{
		std::ifstream indexStream(defaultPath_ + "/index.json");
		if (indexStream)
			this->indexList_ = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : this->indexList_.getValue())
		{
			if (!std::filesystem::exists(std::filesystem::path(defaultPath_).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(defaultPath_))
		{
			if (std::filesystem::is_directory(it))
			{
				auto filepath = it.path();
				auto filename = filepath.filename();

				auto index = indexSet.find(filename.string());
				if (index == indexSet.end())
				{
					if (std::filesystem::exists(filepath.append("package.json")))
					{
						needUpdateIndexFile = true;
						indexSet.insert(filename.string());
					}
				}
			}
		}

		if (needUpdateIndexFile)
		{
			nlohmann::json json;
			for (auto& it : indexSet)
				json += it;

			this->indexList_ = json;
			this->save();
		}
	}

	void
	MaterialImporter::save() const noexcept(false)
	{
		if (!std::filesystem::exists(defaultPath_))
			std::filesystem::create_directory(defaultPath_);

		std::ofstream ifs(defaultPath_ + "/index.json", std::ios_base::binary);
		if (ifs)
		{
			auto data = indexList_.getValue().dump();
			ifs.write(data.c_str(), data.size());
		}
	}
}