#include <octoon/asset_bundle.h>
#include <octoon/asset_preview.h>
#include <octoon/asset_database.h>
#include <octoon/vmd_loader.h>
#include <octoon/mdl_loader.h>
#include <octoon/io/fstream.h>
#include <octoon/texture/texture.h>
#include <octoon/runtime/uuid.h>
#include <octoon/mesh_animation_component.h>
#include <fstream>
#include <filesystem>
#include <set>

namespace octoon
{
	OctoonImplementSingleton(AssetBundle)

	AssetBundle::AssetBundle() noexcept
	{
	}

	AssetBundle::~AssetBundle() noexcept
	{
		this->close();
	}

	void
	AssetBundle::open(const std::filesystem::path& assetPath) noexcept(false)
	{
		assetPath_ = assetPath;

		this->modelAsset_ = std::make_unique<AssetImporter>();
		this->modelAsset_->open(std::filesystem::path(assetPath).append("Model"));

		this->motionAsset_ = std::make_unique<AssetImporter>();
		this->motionAsset_->open(std::filesystem::path(assetPath).append("Motion"));

		this->materialAsset_ = std::make_unique<AssetImporter>();
		this->materialAsset_->open(std::filesystem::path(assetPath).append("Material"));

		this->textureAsset_ = std::make_unique<AssetImporter>();
		this->textureAsset_->open(std::filesystem::path(assetPath).append("Texture"));

		this->hdriAsset_ = std::make_unique<AssetImporter>();
		this->hdriAsset_->open(std::filesystem::path(assetPath).append("HDRi"));
	}

	void
	AssetBundle::close() noexcept
	{
		this->unload();

		modelAsset_->close();
		motionAsset_->close();
		materialAsset_->close();
		textureAsset_->close();
		hdriAsset_->close();

		if (this != AssetBundle::instance())
		{
			for (auto it = AssetBundle::instance()->assetBundles_.begin(); it != AssetBundle::instance()->assetBundles_.end(); ++it)
			{
				if ((*it).get() == this)
				{
					AssetBundle::instance()->assetBundles_.erase(it);
					break;
				}
			}
		}
		else
		{
			assetBundles_.clear();
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::filesystem::path& path) noexcept(false)
	{
		auto ext = path.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == u8".hdr")
			return this->importHDRi(path);
		else if (ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
			return this->importTexture(path);
		else if (ext == u8".pmx")
			return this->importPMX(path);
		else if (ext == u8".vmd")
			return this->importVMD(path);
		else if (ext == u8".mdl")
			return this->importMaterial(path);

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<Texture>& texture, std::string_view ext) noexcept(false)
	{
		auto uuid = AssetDatabase::instance()->getAssetGuid(texture);
		auto rootPath = std::filesystem::path(textureAsset_->getAssertPath()).append(uuid);
		
		try
		{
			auto outputPath = std::filesystem::path(rootPath).append(uuid + (std::string)ext);

			AssetDatabase::instance()->createAsset(texture, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = texture->getName();
			package["path"] = (char*)outputPath.u8string().c_str();
			package["visible"] = true;

			std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->textureAsset_->addIndex(std::string(uuid));
			this->assetGuidList_[texture] = uuid;

			return package;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<Animation>& animation) noexcept(false)
	{
		auto uuid = AssetDatabase::instance()->getAssetGuid(animation);
		auto rootPath = std::filesystem::path(motionAsset_->getAssertPath()).append(uuid);

		try
		{
			auto outputPath = std::filesystem::path(rootPath).append(uuid + ".vmd");

			AssetDatabase::instance()->createAsset(animation, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = animation->getName();
			package["path"] = (char*)outputPath.u8string().c_str();
			package["visible"] = true;

			std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->motionAsset_->addIndex(uuid);
			this->assetGuidList_[animation] = uuid;

			return package;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<Material>& material) noexcept(false)
	{
		auto uuid = AssetDatabase::instance()->getAssetGuid(material);
		auto rootPath = std::filesystem::path(materialAsset_->getAssertPath()).append(uuid);

		try
		{
			auto standardMaterial = material->downcast<MeshStandardMaterial>();
			if (standardMaterial->getColorMap())
				this->importAsset(standardMaterial->getColorMap(), ".png");
			if (standardMaterial->getOpacityMap())
				this->importAsset(standardMaterial->getOpacityMap(), ".png");
			if (standardMaterial->getNormalMap())
				this->importAsset(standardMaterial->getNormalMap(), ".png");
			if (standardMaterial->getRoughnessMap())
				this->importAsset(standardMaterial->getRoughnessMap(), ".png");
			if (standardMaterial->getSpecularMap())
				this->importAsset(standardMaterial->getSpecularMap(), ".png");
			if (standardMaterial->getMetalnessMap())
				this->importAsset(standardMaterial->getMetalnessMap(), ".png");
			if (standardMaterial->getEmissiveMap())
				this->importAsset(standardMaterial->getEmissiveMap(), ".png");
			if (standardMaterial->getAnisotropyMap())
				this->importAsset(standardMaterial->getAnisotropyMap(), ".png");
			if (standardMaterial->getClearCoatMap())
				this->importAsset(standardMaterial->getClearCoatMap(), ".png");
			if (standardMaterial->getClearCoatRoughnessMap())
				this->importAsset(standardMaterial->getClearCoatRoughnessMap(), ".png");
			if (standardMaterial->getSubsurfaceMap())
				this->importAsset(standardMaterial->getSubsurfaceMap(), ".png");
			if (standardMaterial->getSubsurfaceColorMap())
				this->importAsset(standardMaterial->getSubsurfaceColorMap(), ".png");
			if (standardMaterial->getSheenMap())
				this->importAsset(standardMaterial->getSheenMap(), ".png");
			if (standardMaterial->getLightMap())
				this->importAsset(standardMaterial->getLightMap(), ".png");

			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(material) + ".mat");

			AssetDatabase::instance()->createAsset(material, outputPath);

			auto writePreview = [this](const std::shared_ptr<Material>& material, std::filesystem::path path)
			{
				auto texture = AssetPreview::instance()->getAssetPreview(material);
				auto previewPath = std::filesystem::path(path).append(make_guid() + ".png");
				AssetDatabase::instance()->createAsset(texture, previewPath);
				return previewPath;
			};

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = material->getName();
			package["path"] = (char*)outputPath.u8string().c_str();
			package["preview"] = (char*)writePreview(material, rootPath).u8string().c_str();
			package["visible"] = true;

			std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->materialAsset_->addIndex(package["uuid"]);
			this->assetGuidList_[material] = uuid;

			return package;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<PMX>& pmx) noexcept(false)
	{
		auto uuid = AssetDatabase::instance()->getAssetGuid(pmx);
		auto rootPath = std::filesystem::path(modelAsset_->getAssertPath()).append(uuid);

		try
		{
			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(pmx) + ".pmx");

			AssetDatabase::instance()->createAsset(pmx, outputPath);

			math::BoundingBox bound;
			for (auto& v : pmx->vertices)
				bound.encapsulate(math::float3(v.position.x, v.position.y, v.position.z));

			auto writePreview = [this](const PMX& pmx, const math::BoundingBox& boundingBox, std::filesystem::path outputPath)
			{
				auto texture = AssetPreview::instance()->getAssetPreview(pmx, boundingBox);
				auto previewPath = std::filesystem::path(outputPath).append(make_guid() + ".png");
				AssetDatabase::instance()->createAsset(texture, previewPath);
				return previewPath;
			};

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = cv.to_bytes(pmx->description.japanModelName.data());
			package["path"] = (char*)outputPath.u8string().c_str();
			package["bound"][0] = bound.box().min.to_array();
			package["bound"][1] = bound.box().max.to_array();
			package["preview"] = (char*)writePreview(*pmx, bound, rootPath).u8string().c_str();

			std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->modelAsset_->addIndex(uuid);
			this->assetGuidList_[pmx] = uuid;

			return package;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<GameObject>& gameObject) noexcept(false)
	{
		auto uuid = AssetDatabase::instance()->getAssetGuid(gameObject);
		auto rootPath = std::filesystem::path(modelAsset_->getAssertPath()).append(uuid);

		try
		{
			auto assetPath = AssetDatabase::instance()->getAssetPath(gameObject);
			if (!assetPath.empty())
			{
				auto ext = assetPath.extension().u8string();
				for (auto& it : ext)
					it = (char)std::tolower(it);

				if (ext == u8".pmx")
				{
					return this->importAsset(assetPath);
				}
				else if (ext == u8".abc")
				{
					nlohmann::json package;
					package["uuid"] = uuid;
					package["path"] = (char*)assetPath.u8string().c_str();

					auto abc = gameObject->getComponent<MeshAnimationComponent>();
					if (abc)
					{
						auto& materials = abc->getMaterials();

						for (auto& pair : materials)
						{
							auto materialPackage = this->createAsset(pair.second);
							if (materialPackage.is_object())
							{
								nlohmann::json materialJson;
								materialJson["data"] = materialPackage["uuid"];
								materialJson["name"] = pair.first;

								package["materials"].push_back(materialJson);
							}
						}
					}

					std::filesystem::create_directories(rootPath);

					std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
					if (ifs)
					{
						auto dump = package.dump();
						ifs.write(dump.c_str(), dump.size());
					}

					assetGuidList_[gameObject] = uuid;
					return package;
				}
				else
				{
					nlohmann::json package;
					package["uuid"] = uuid;
					package["path"] = (char*)assetPath.u8string().c_str();

					std::filesystem::create_directories(rootPath);

					std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
					if (ifs)
					{
						auto dump = package.dump();
						ifs.write(dump.c_str(), dump.size());
					}

					assetGuidList_[gameObject] = uuid;
					return package;
				}
			}

			return nlohmann::json();
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importHDRi(const std::filesystem::path& path) noexcept(false)
	{
		auto texture = std::make_shared<Texture>();
		if (texture->load(path))
		{
			texture->setName((char*)path.filename().u8string().c_str());
			texture->setMipLevel(8);

			auto uuid = AssetDatabase::instance()->getAssetGuid(texture);

			auto rootPath = std::filesystem::path(hdriAsset_->getAssertPath()).append(uuid);
			auto texturePath = std::filesystem::path(rootPath).append(uuid + ".hdr");

			auto width = texture->width();
			auto height = texture->height();
			auto data = (float*)texture->data();

			Texture previewTexutre(Format::R8G8B8SRGB, width, height);

			auto size = width * height * 3;
			auto pixels = previewTexutre.data();

			for (std::size_t i = 0; i < size; i += 3)
			{
				pixels[i] = static_cast<std::uint8_t>(std::clamp(std::pow(data[i], 1.0f / 2.2f) * 255.0f, 0.0f, 255.0f));
				pixels[i + 1] = static_cast<std::uint8_t>(std::clamp(std::pow(data[i + 1], 1.0f / 2.2f) * 255.0f, 0.0f, 255.0f));
				pixels[i + 2] = static_cast<std::uint8_t>(std::clamp(std::pow(data[i + 2], 1.0f / 2.2f) * 255.0f, 0.0f, 255.0f));
			}

			auto preview = std::make_shared<Texture>(previewTexutre.resize(260, 130));
			auto previewName = AssetDatabase::instance()->getAssetGuid(preview) + ".png";
			auto previewPath = std::filesystem::path(rootPath).append(previewName);

			AssetDatabase::instance()->createAsset(texture, texturePath);
			AssetDatabase::instance()->createAsset(preview, previewPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = texture->getName();
			package["path"] = (char*)texturePath.u8string().c_str();
			package["preview"] = (char*)previewPath.u8string().c_str();
			package["visible"] = true;

			std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->hdriAsset_->addIndex(uuid);
			this->assetGuidList_[texture] = uuid;

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importTexture(const std::filesystem::path& path) noexcept(false)
	{
		auto texture = std::make_shared<Texture>();
		if (texture->load(path))
		{
			auto ext = path.extension().u8string();
			for (auto& it : ext)
				it = (char)std::tolower(it);

			texture->setName((char*)path.filename().u8string().c_str());

			return this->importAsset(texture, (char*)ext.c_str());
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importPMX(const std::filesystem::path& path) noexcept(false)
	{
		auto pmx = std::make_shared<PMX>();
		if (PMX::load(path, *pmx))
		{
			if (pmx->description.japanModelLength == 0)
			{
				auto filename = std::filesystem::path(path).filename().wstring();

				pmx->description.japanModelName.resize(filename.size() + 1);
				pmx->description.japanModelLength = static_cast<PmxUInt32>(pmx->description.japanModelName.size() * 2);
				std::memcpy(pmx->description.japanModelName.data(), filename.data(), pmx->description.japanModelLength);
			}

			return this->importAsset(pmx);
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importVMD(const std::filesystem::path& path) noexcept(false)
	{
		auto animation = VMDLoader::load(path);
		if (animation)
		{
			if (animation->getName().empty())
				animation->setName((char*)path.filename().u8string().c_str());

			return this->importAsset(animation);
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importMaterial(const std::filesystem::path& path) noexcept(false)
	{
		try
		{
			nlohmann::json items;

			io::ifstream stream(path.u8string());
			if (stream)
			{
				MDLLoader loader;
				loader.load("resource", stream);

				for (auto& material : loader.getMaterials())
				{
					auto package = this->importAsset(material);
					if (package.is_object())
						items.push_back(package["uuid"]);
				}

				materialAsset_->saveAssets();
			}

			return items;
		}
		catch (const std::exception& e)
		{
			materialAsset_->saveAssets();
			throw e;
		}
	}

	std::shared_ptr<RttiObject>
	AssetBundle::loadAsset(std::string_view uuid, const Rtti& type) noexcept(false)
	{
		if (type.isDerivedFrom(Material::getRtti()) && materialAsset_->hasPackage(uuid))
			return this->loadAssetAtPackage<Material>(materialAsset_->getPackage(uuid));
		else if (type.isDerivedFrom(GameObject::getRtti()) && modelAsset_->hasPackage(uuid))
			return this->loadAssetAtPackage<GameObject>(modelAsset_->getPackage(uuid));
		else if (type.isDerivedFrom(Animation::getRtti()) && motionAsset_->hasPackage(uuid))
			return this->loadAssetAtPackage<Animation>(motionAsset_->getPackage(uuid));
		else if (type.isDerivedFrom(Texture::getRtti()))
		{
			if (hdriAsset_->hasPackage(uuid))
				return this->loadAssetAtPackage<Texture>(hdriAsset_->getPackage(uuid));
			else if (textureAsset_->hasPackage(uuid))
				return  this->loadAssetAtPackage<Texture>(textureAsset_->getPackage(uuid));
		}

		for (auto& ab : assetBundles_)
		{
			auto asset = ab->loadAsset(uuid, type);
			if (asset)
				return asset;
		}

		return nullptr;
	}

	std::shared_ptr<RttiObject>
	AssetBundle::loadAssetAtPackage(const nlohmann::json& package, const Rtti& type) noexcept(false)
	{
		if (package.is_object() && package.contains("uuid") && package.contains("path"))
		{
			auto uuid = package["uuid"].get<std::string>();
			if (this->assetCache_.contains(uuid))
			{
				auto it = this->assetCache_.at(uuid);
				if (!it.expired())
					return it.lock();
			}

			auto path = package["path"].get<std::string>();
			auto filepath = std::filesystem::path((char8_t*)path.c_str());

			std::shared_ptr<RttiObject> asset;
			if (type.isDerivedFrom(Texture::getRtti()))
				asset = AssetDatabase::instance()->loadAssetAtPath<Texture>(filepath);
			else if (type.isDerivedFrom(Animation::getRtti()))
				asset = AssetDatabase::instance()->loadAssetAtPath<Animation>(filepath);
			else if (type.isDerivedFrom(Material::getRtti()))
				asset = AssetDatabase::instance()->loadAssetAtPath<Material>(filepath);
			else if (type.isDerivedFrom(GameObject::getRtti()))
				asset = AssetDatabase::instance()->loadAssetAtPath<GameObject>(filepath);

			if (asset)
			{
				assetCache_[uuid] = asset;
				assetGuidList_[asset] = uuid;
			}

			return asset;
		}

		return nullptr;
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Texture>& texture) noexcept(false)
	{
		if (texture)
		{
			auto uuid = this->getPackageGuid(texture);
			if (!uuid.empty())
			{
				if (this != AssetBundle::instance())
				{
					if (AssetBundle::instance()->textureAsset_->hasPackage(uuid))
						return AssetBundle::instance()->getPackage(uuid);
				}

				if (this->hasPackage(uuid) && !this->needUpdate(uuid))
					return this->getPackage(uuid);
			}

			auto filepath = AssetDatabase::instance()->getAssetPath(texture);

			auto package = this->importAsset(texture, (char*)filepath.extension().u8string().c_str());
			if (package.is_object())
				this->removeUpdateList(package["uuid"].get<std::string>());

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Animation>& animation) noexcept(false)
	{
		if (animation)
		{
			auto uuid = this->getPackageGuid(animation);
			if (!uuid.empty())
			{
				if (this != AssetBundle::instance())
				{
					if (AssetBundle::instance()->motionAsset_->hasPackage(uuid))
						return AssetBundle::instance()->getPackage(uuid);
				}

				if (this->hasPackage(uuid) && !this->needUpdate(uuid))
					return this->getPackage(uuid);
			}

			auto package = this->importAsset(animation);
			if (package.is_object())
				this->removeUpdateList(package["uuid"].get<std::string>());

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Material>& material) noexcept(false)
	{
		if (material)
		{
			auto uuid = this->getPackageGuid(material);
			if (!uuid.empty())
			{
				if (this != AssetBundle::instance())
				{
					if (AssetBundle::instance()->materialAsset_->hasPackage(uuid))
						return AssetBundle::instance()->getPackage(uuid);
				}

				if (this->hasPackage(uuid) && !this->needUpdate(uuid))
					return this->getPackage(uuid);
			}

			auto package = this->importAsset(material);
			if (package.is_object())
				this->removeUpdateList(package["uuid"].get<std::string>());

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<GameObject>& gameObject) noexcept(false)
	{
		if (gameObject)
		{
			auto uuid = this->getPackageGuid(gameObject);
			if (!uuid.empty())
			{
				if (this != AssetBundle::instance())
				{
					if (AssetBundle::instance()->modelAsset_->hasPackage(uuid))
						return AssetBundle::instance()->getPackage(uuid);
				}

				if (this->hasPackage(uuid) && !this->needUpdate(uuid))
					return this->getPackage(uuid);
			}
			else
			{
				uuid = make_guid();
			}

			auto package = this->importAsset(gameObject);
			if (package.is_object())
				this->removeUpdateList(package["uuid"].get<std::string>());

			return package;
		}

		return nlohmann::json();
	}

	bool
	AssetBundle::hasPackage(std::string_view uuid) noexcept
	{
		if (this->modelAsset_->hasPackage(uuid))
			return true;
		if (this->motionAsset_->hasPackage(uuid))
			return true;
		if (this->materialAsset_->hasPackage(uuid))
			return true;
		if (this->textureAsset_->hasPackage(uuid))
			return true;
		if (this->hdriAsset_->hasPackage(uuid))
			return true;

		return false;
	}

	nlohmann::json
	AssetBundle::getPackage(std::string_view uuid) noexcept
	{
		if (this->modelAsset_->hasPackage(uuid))
			return this->modelAsset_->getPackage(uuid);
		if (this->motionAsset_->hasPackage(uuid))
			return this->motionAsset_->getPackage(uuid);
		if (this->materialAsset_->hasPackage(uuid))
			return this->materialAsset_->getPackage(uuid);
		if (this->textureAsset_->hasPackage(uuid))
			return this->textureAsset_->getPackage(uuid);
		if (this->hdriAsset_->hasPackage(uuid))
			return this->hdriAsset_->getPackage(uuid);

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::getPackage(const std::shared_ptr<RttiObject>& asset) noexcept
	{
		auto guid = this->getPackageGuid(asset);
		if (guid.empty())
			return nlohmann::json();
		return this->getPackage(guid);
	}

	std::string
	AssetBundle::getPackageGuid(const std::shared_ptr<RttiObject>& asset) const noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);

		for (auto& ab : assetBundles_)
		{
			auto uuid = ab->getPackageGuid(asset);
			if (!uuid.empty())
				return uuid;
		}

		return std::string();
	}

	nlohmann::json&
	AssetBundle::getModelList() const noexcept
	{
		return modelAsset_->getIndexList();
	}

	nlohmann::json&
	AssetBundle::getMotionList() const noexcept
	{
		return motionAsset_->getIndexList();
	}

	nlohmann::json&
	AssetBundle::getTextureList() const noexcept
	{
		return textureAsset_->getIndexList();
	}

	nlohmann::json&
	AssetBundle::getHDRiList() const noexcept
	{
		return hdriAsset_->getIndexList();
	}

	nlohmann::json&
	AssetBundle::getMaterialList() const noexcept
	{
		return materialAsset_->getIndexList();
	}

	void
	AssetBundle::unload() noexcept
	{
		this->assetCache_.clear();
		this->assetBundles_.clear();
	}

	void
	AssetBundle::saveAssets() noexcept(false)
	{
		if (std::filesystem::exists(assetPath_))
		{
			this->modelAsset_->saveAssets();
			this->motionAsset_->saveAssets();
			this->materialAsset_->saveAssets();
			this->textureAsset_->saveAssets();
			this->hdriAsset_->saveAssets();
		}

		for (auto& ab : assetBundles_)
			ab->saveAssets();
	}

	void
	AssetBundle::removeAsset(std::string_view uuid) noexcept(false)
	{
		if (this->modelAsset_->hasPackage(uuid))
			this->modelAsset_->removeAsset(uuid);
		if (this->motionAsset_->hasPackage(uuid))
			this->motionAsset_->removeAsset(uuid);
		if (this->materialAsset_->hasPackage(uuid))
			this->materialAsset_->removeAsset(uuid);
		if (this->textureAsset_->hasPackage(uuid))
			this->textureAsset_->removeAsset(uuid);
		if (this->hdriAsset_->hasPackage(uuid))
			this->hdriAsset_->removeAsset(uuid);
	}

	std::shared_ptr<AssetBundle>
	AssetBundle::loadFromFile(const std::filesystem::path& path) noexcept(false)
	{
		auto ab = std::make_shared<AssetBundle>();
		ab->open(path);
		assetBundles_.push_back(ab);

		return ab;
	}

	std::vector<std::shared_ptr<AssetBundle>>
	AssetBundle::getAllLoadedAssetBundles() const noexcept
	{
		return assetBundles_;
	}

	void
	AssetBundle::addUpdateList(std::string_view uuid) noexcept(false)
	{
		this->updateList_.insert(std::string(uuid));
	}

	bool
	AssetBundle::needUpdate(std::string_view uuid) const noexcept
	{
		return this->updateList_.find(std::string(uuid)) != this->updateList_.end();
	}

	void
	AssetBundle::removeUpdateList(std::string_view uuid) noexcept(false)
	{
		auto it = this->updateList_.find(std::string(uuid));
		if (it != this->updateList_.end())
			this->updateList_.erase(it);
	}

	void
	AssetBundle::clearUpdate() noexcept
	{
		this->updateList_.clear();
	}

	const std::set<std::string>&
	AssetBundle::getUpdateList() const noexcept
	{
		return this->updateList_;
	}
}