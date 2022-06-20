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
	AssetBundle::importHDRi(const std::filesystem::path& path) noexcept(false)
	{
		auto texture = std::make_shared<Texture>();
		if (texture->load(path))
		{
			texture->setName((char*)path.filename().u8string().c_str());
			texture->setMipLevel(8);

			auto uuid = make_guid();

			auto rootPath = std::filesystem::path(hdriAsset_->getAssertPath()).append(uuid);
			auto texturePath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(texture) + ".hdr");

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
	AssetBundle::importTexture(const std::shared_ptr<Texture>& texture, std::string_view ext, std::string_view uuid) noexcept(false)
	{
		auto rootPath = std::filesystem::path(textureAsset_->getAssertPath()).append(uuid);
		
		try
		{
			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(texture) + (std::string)ext);

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
	AssetBundle::importTexture(const std::filesystem::path& path) noexcept(false)
	{
		auto texture = std::make_shared<Texture>();
		if (texture->load(path))
		{
			auto ext = path.extension().u8string();
			for (auto& it : ext)
				it = (char)std::tolower(it);

			texture->setName((char*)path.filename().u8string().c_str());

			return this->importTexture(texture, (char*)ext.c_str());
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importPMX(const std::filesystem::path& path) noexcept(false)
	{
		auto uuid = make_guid();
		auto rootPath = std::filesystem::path(modelAsset_->getAssertPath()).append(uuid);

		try
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

				auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(pmx) + ".pmx");

				AssetDatabase::instance()->createAsset(pmx, outputPath);

				math::BoundingBox bound;
				for (auto& v : pmx->vertices)
					bound.encapsulate(math::float3(v.position.x, v.position.y, v.position.z));

				auto writePreview = [this](const PMX& pmx, const math::BoundingBox& boundingBox, std::filesystem::path outputPath)
				{
					auto texture = AssetPreview::instance()->getAssetPreview(pmx, boundingBox);
					auto previewPath = std::filesystem::path(outputPath).append(make_guid() + ".png");
					texture->save(previewPath, "png");
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
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importVMD(const std::filesystem::path& path) noexcept(false)
	{
		auto uuid = make_guid();
		auto rootPath = std::filesystem::path(motionAsset_->getAssertPath()).append(uuid);

		try
		{
			auto animation = VMDLoader::load(path);
			if (animation)
			{
				if (animation->getName().empty())
					animation->setName((char*)path.filename().u8string().c_str());

				auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(animation) + ".vmd");

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
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
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
					auto standardMaterial = material->downcast<MeshStandardMaterial>();
					if (standardMaterial->getColorMap())
						this->importTexture(standardMaterial->getColorMap(), ".png");
					if (standardMaterial->getOpacityMap())
						this->importTexture(standardMaterial->getOpacityMap(), ".png");
					if (standardMaterial->getNormalMap())
						this->importTexture(standardMaterial->getNormalMap(), ".png");
					if (standardMaterial->getRoughnessMap())
						this->importTexture(standardMaterial->getRoughnessMap(), ".png");
					if (standardMaterial->getSpecularMap())
						this->importTexture(standardMaterial->getSpecularMap(), ".png");
					if (standardMaterial->getMetalnessMap())
						this->importTexture(standardMaterial->getMetalnessMap(), ".png");
					if (standardMaterial->getEmissiveMap())
						this->importTexture(standardMaterial->getEmissiveMap(), ".png");
					if (standardMaterial->getAnisotropyMap())
						this->importTexture(standardMaterial->getAnisotropyMap(), ".png");
					if (standardMaterial->getClearCoatMap())
						this->importTexture(standardMaterial->getClearCoatMap(), ".png");
					if (standardMaterial->getClearCoatRoughnessMap())
						this->importTexture(standardMaterial->getClearCoatRoughnessMap(), ".png");
					if (standardMaterial->getSubsurfaceMap())
						this->importTexture(standardMaterial->getSubsurfaceMap(), ".png");
					if (standardMaterial->getSubsurfaceColorMap())
						this->importTexture(standardMaterial->getSubsurfaceColorMap(), ".png");
					if (standardMaterial->getSheenMap())
						this->importTexture(standardMaterial->getSheenMap(), ".png");
					if (standardMaterial->getLightMap())
						this->importTexture(standardMaterial->getLightMap(), ".png");

					auto uuid = make_guid();
					auto rootPath = std::filesystem::path(materialAsset_->getAssertPath()).append(uuid);

					try
					{
						auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(material) + ".mat");

						AssetDatabase::instance()->createAsset(material, outputPath);

						auto writePreview = [this](const std::shared_ptr<Material>& material, std::filesystem::path path)
						{
							auto texture = AssetPreview::instance()->getAssetPreview(material);
							auto previewPath = std::filesystem::path(path).append(make_guid() + ".png");
							texture->save(previewPath, "png");
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

						items.push_back(package["uuid"]);

						this->materialAsset_->addIndex(package["uuid"]);
						this->assetGuidList_[material] = uuid;
					}
					catch (const std::exception& e)
					{
						if (std::filesystem::exists(rootPath))
							std::filesystem::remove_all(rootPath);

						throw e;
					}
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

				if (this->hasPackage(uuid))
				{
					if (!AssetDatabase::instance()->needUpdate(uuid))
						return this->getPackage(uuid);
				}
			}
			else
			{
				uuid = make_guid();
			}

			auto filepath = AssetDatabase::instance()->getAssetPath(texture);
			auto rootPath = std::filesystem::path(hdriAsset_->getAssertPath()).append(uuid);
			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(texture) + (char*)filepath.extension().u8string().c_str());

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

			AssetDatabase::instance()->removeUpdateList(uuid);
			hdriAsset_->addIndex(package["uuid"].get<std::string>());

			this->assetGuidList_[texture] = uuid;

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

				if (this->hasPackage(uuid))
				{
					if (!AssetDatabase::instance()->needUpdate(uuid))
						return this->getPackage(uuid);
				}
			}
			else
			{
				uuid = make_guid();
			}

			auto rootPath = std::filesystem::path(motionAsset_->getAssertPath()).append(uuid);
			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(animation) + ".vmd");

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

			AssetDatabase::instance()->removeUpdateList(uuid);
			motionAsset_->addIndex(package["uuid"].get<std::string>());

			this->assetGuidList_[animation] = uuid;

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

				if (this->hasPackage(uuid))
				{
					if (!AssetDatabase::instance()->needUpdate(uuid))
						return this->getPackage(uuid);
				}
			}
			else
			{
				uuid = make_guid();
			}

			auto standardMaterial = material->downcast<MeshStandardMaterial>();
			if (standardMaterial->getColorMap())
				this->createAsset(standardMaterial->getColorMap());
			if (standardMaterial->getOpacityMap())
				this->createAsset(standardMaterial->getOpacityMap());
			if (standardMaterial->getNormalMap())
				this->createAsset(standardMaterial->getNormalMap());
			if (standardMaterial->getRoughnessMap())
				this->createAsset(standardMaterial->getRoughnessMap());
			if (standardMaterial->getSpecularMap())
				this->createAsset(standardMaterial->getSpecularMap());
			if (standardMaterial->getMetalnessMap())
				this->createAsset(standardMaterial->getMetalnessMap());
			if (standardMaterial->getEmissiveMap())
				this->createAsset(standardMaterial->getEmissiveMap());
			if (standardMaterial->getAnisotropyMap())
				this->createAsset(standardMaterial->getAnisotropyMap());
			if (standardMaterial->getClearCoatMap())
				this->createAsset(standardMaterial->getClearCoatMap());
			if (standardMaterial->getClearCoatRoughnessMap())
				this->createAsset(standardMaterial->getClearCoatRoughnessMap());
			if (standardMaterial->getSubsurfaceMap())
				this->createAsset(standardMaterial->getSubsurfaceMap());
			if (standardMaterial->getSubsurfaceColorMap())
				this->createAsset(standardMaterial->getSubsurfaceColorMap());
			if (standardMaterial->getSheenMap())
				this->createAsset(standardMaterial->getSheenMap());
			if (standardMaterial->getLightMap())
				this->createAsset(standardMaterial->getLightMap());

			auto rootPath = std::filesystem::path(materialAsset_->getAssertPath()).append(uuid);
			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->getAssetGuid(material) + ".mat");

			AssetDatabase::instance()->createAsset(material, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = material->getName();
			package["path"] = (char*)outputPath.u8string().c_str();
			package["visible"] = true;

			std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			AssetDatabase::instance()->removeUpdateList(uuid);
			materialAsset_->addIndex(package["uuid"].get<std::string>());

			this->assetGuidList_[material] = uuid;

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
			}
			else
			{
				uuid = make_guid();
			}

			auto assetPath = AssetDatabase::instance()->getAssetPath(gameObject);
			if (!assetPath.empty())
			{
				auto ext = assetPath.extension().u8string();
				for (auto& it : ext)
					it = (char)std::tolower(it);

				if (ext == u8".pmx")
				{
					for (auto& index : modelAsset_->getIndexList())
					{
						if (index == uuid)
						{
							if (!AssetDatabase::instance()->needUpdate(uuid))
								return modelAsset_->getPackage(uuid);
						}
					}

					auto package = this->importAsset(assetPath);
					if (package.is_object())
					{
						AssetDatabase::instance()->removeUpdateList(uuid);

						assetGuidList_[gameObject] = uuid;
						return package;
					}
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

					if (package.is_object())
					{
						auto modelPath = this->modelAsset_->getAssertPath();
						auto packagePath = std::filesystem::path(modelPath).append(uuid).append("package.json");

						std::filesystem::create_directories(packagePath.parent_path());

						std::ofstream ifs(packagePath, std::ios_base::binary);
						if (ifs)
						{
							auto dump = package.dump();
							ifs.write(dump.c_str(), dump.size());
						}

						AssetDatabase::instance()->removeUpdateList(uuid);

						assetGuidList_[gameObject] = uuid;
						return package;
					}
				}
				else
				{
					nlohmann::json package;
					package["uuid"] = uuid;
					package["path"] = (char*)assetPath.u8string().c_str();

					if (package.is_object())
					{
						auto modelPath = this->modelAsset_->getAssertPath();
						auto packagePath = std::filesystem::path(modelPath).append(uuid).append("package.json");

						std::filesystem::create_directories(packagePath.parent_path());

						std::ofstream ifs(packagePath, std::ios_base::binary);
						if (ifs)
						{
							auto dump = package.dump();
							ifs.write(dump.c_str(), dump.size());
						}

						AssetDatabase::instance()->removeUpdateList(uuid);

						assetGuidList_[gameObject] = uuid;
						return package;
					}
				}
			}
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
		if (guid.empty()) return nlohmann::json();
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
}