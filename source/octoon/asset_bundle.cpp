#include <octoon/asset_bundle.h>
#include <octoon/asset_preview.h>
#include <octoon/asset_database.h>
#include <octoon/vmd_loader.h>
#include <octoon/mdl_loader.h>
#include <octoon/io/fstream.h>
#include <octoon/texture/texture.h>
#include <octoon/runtime/uuid.h>
#include <octoon/mesh_renderer_component.h>
#include <octoon/skinned_mesh_renderer_component.h>
#include <octoon/mesh_animation_component.h>
#include <octoon/animator_component.h>
#include <octoon/transform_component.h>
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
		this->motionAsset_ = std::make_unique<AssetImporter>();
		this->materialAsset_ = std::make_unique<AssetImporter>();
		this->textureAsset_ = std::make_unique<AssetImporter>();
		this->hdriAsset_ = std::make_unique<AssetImporter>();
		this->prefabAsset_ = std::make_unique<AssetImporter>();

		this->modelAsset_->open(std::filesystem::path(assetPath).append("Models"));
		this->motionAsset_->open(std::filesystem::path(assetPath).append("Motions"));
		this->materialAsset_->open(std::filesystem::path(assetPath).append("Materials"));
		this->textureAsset_->open(std::filesystem::path(assetPath).append("Textures"));
		this->hdriAsset_->open(std::filesystem::path(assetPath).append("HDRIs"));
		this->prefabAsset_->open(std::filesystem::path(assetPath).append("Prefabs"));
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
		prefabAsset_->close();

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

		if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(path);
			if (texture)
				return this->importAsset(texture);
		}
		else if (ext == u8".vmd")
		{
			auto animation = AssetDatabase::instance()->loadAssetAtPath<Animation>(path);
			if (animation)
				return this->importAsset(animation);
		}
		else if (ext == u8".pmx" || ext == u8".obj" || ext == u8".fbx")
		{
			auto gameObject = AssetDatabase::instance()->loadAssetAtPath<GameObject>(path);
			if (gameObject)
				return this->importAsset(gameObject);
		}
		else if (ext == u8".mdl")
		{
			std::ifstream stream(path);
			if (stream)
			{
				nlohmann::json items;

				MDLLoader loader;
				loader.load("resource", stream);

				for (auto& material : loader.getMaterials())
				{
					auto package = this->importAsset(material);
					if (package.is_object())
						items.push_back(package["uuid"]);
				}

				materialAsset_->saveAssets();

				return items;
			}
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<Texture>& texture) noexcept(false)
	{
		auto hdr = (texture->format() == Format::R32G32B32SFloat) ? true : false;
		auto uuid = AssetDatabase::instance()->assetToGuid(texture);
		auto rootPath = std::filesystem::path(hdr ? hdriAsset_->getAssertPath() : textureAsset_->getAssertPath()).append(uuid);
		
		try
		{
			std::string ext = hdr ? ".hdr" : ".png";

			auto assetPath = AssetDatabase::instance()->getAssetPath(texture);
			if (!assetPath.empty())
				ext = assetPath.extension().string();

			auto outputPath = std::filesystem::path(rootPath).append(uuid + ext);

			AssetDatabase::instance()->createAsset(texture, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = texture->getName();
			package["path"] = (char*)outputPath.u8string().c_str();
			package["visible"] = true;

			auto preview = AssetPreview::instance()->getAssetPreview(texture);
			if (preview)
			{
				auto previewName = AssetDatabase::instance()->assetToGuid(preview) + ".png";
				auto previewPath = std::filesystem::path(rootPath).append(previewName);
				AssetDatabase::instance()->createAsset(preview, previewPath);
				package["preview"] = (char*)previewPath.u8string().c_str();
			}

			std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			if (hdr)
				this->hdriAsset_->addPackage(uuid);
			else
				this->textureAsset_->addPackage(uuid);

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
		auto uuid = AssetDatabase::instance()->assetToGuid(animation);
		auto rootPath = std::filesystem::path(motionAsset_->getAssertPath()).append(uuid);

		try
		{
			std::string ext = ".vmd";

			auto assetPath = AssetDatabase::instance()->getAssetPath(animation);
			if (!assetPath.empty())
				ext = assetPath.extension().string();

			auto outputPath = std::filesystem::path(rootPath).append(uuid + ext);

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

			this->motionAsset_->addPackage(uuid);

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
		auto uuid = AssetDatabase::instance()->assetToGuid(material);
		auto rootPath = std::filesystem::path(materialAsset_->getAssertPath()).append(uuid);

		try
		{
			auto standardMaterial = material->downcast<MeshStandardMaterial>();
			if (standardMaterial->getColorMap())
				this->importAsset(standardMaterial->getColorMap());
			if (standardMaterial->getOpacityMap())
				this->importAsset(standardMaterial->getOpacityMap());
			if (standardMaterial->getNormalMap())
				this->importAsset(standardMaterial->getNormalMap());
			if (standardMaterial->getRoughnessMap())
				this->importAsset(standardMaterial->getRoughnessMap());
			if (standardMaterial->getSpecularMap())
				this->importAsset(standardMaterial->getSpecularMap());
			if (standardMaterial->getMetalnessMap())
				this->importAsset(standardMaterial->getMetalnessMap());
			if (standardMaterial->getEmissiveMap())
				this->importAsset(standardMaterial->getEmissiveMap());
			if (standardMaterial->getAnisotropyMap())
				this->importAsset(standardMaterial->getAnisotropyMap());
			if (standardMaterial->getClearCoatMap())
				this->importAsset(standardMaterial->getClearCoatMap());
			if (standardMaterial->getClearCoatRoughnessMap())
				this->importAsset(standardMaterial->getClearCoatRoughnessMap());
			if (standardMaterial->getSubsurfaceMap())
				this->importAsset(standardMaterial->getSubsurfaceMap());
			if (standardMaterial->getSubsurfaceColorMap())
				this->importAsset(standardMaterial->getSubsurfaceColorMap());
			if (standardMaterial->getSheenMap())
				this->importAsset(standardMaterial->getSheenMap());
			if (standardMaterial->getLightMap())
				this->importAsset(standardMaterial->getLightMap());

			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->assetToGuid(material) + ".mat");

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

			this->materialAsset_->addPackage(package["uuid"]);

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
		auto uuid = AssetDatabase::instance()->assetToGuid(pmx);
		auto rootPath = std::filesystem::path(modelAsset_->getAssertPath()).append(uuid);

		try
		{
			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->assetToGuid(pmx) + ".pmx");

			AssetDatabase::instance()->createAsset(pmx, outputPath);

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

			auto writePreview = [this](const PMX& pmx, std::filesystem::path outputPath)
			{
				auto texture = AssetPreview::instance()->getAssetPreview(pmx);
				auto previewPath = std::filesystem::path(outputPath).append(make_guid() + ".png");
				AssetDatabase::instance()->createAsset(texture, previewPath);
				return previewPath;
			};

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = cv.to_bytes(pmx->description.japanModelName.data());
			package["path"] = (char*)outputPath.u8string().c_str();
			package["preview"] = (char*)writePreview(*pmx, rootPath).u8string().c_str();

			std::ofstream ifs(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->modelAsset_->addPackage(uuid);

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
		auto uuid = AssetDatabase::instance()->assetToGuid(gameObject);
		auto rootPath = std::filesystem::path(prefabAsset_->getAssertPath()).append(uuid);

		try
		{
			auto outputPath = std::filesystem::path(rootPath).append(AssetDatabase::instance()->assetToGuid(gameObject) + ".prefab");

			std::filesystem::create_directories(rootPath);

			nlohmann::json prefab;

			auto assetPath = AssetDatabase::instance()->getAssetPath(gameObject);
			if (!assetPath.empty())
			{
				auto ext = assetPath.extension().u8string();
				for (auto& it : ext)
					it = (char)std::tolower(it);

				if (ext == u8".pmx")
				{
					auto pmx = std::make_shared<PMX>();
					if (PMX::load(assetPath, *pmx))
					{
						if (pmx->description.japanModelLength == 0)
						{
							auto filename = std::filesystem::path(assetPath).filename().wstring();

							pmx->description.japanModelName.resize(filename.size() + 1);
							pmx->description.japanModelLength = static_cast<PmxUInt32>(pmx->description.japanModelName.size() * 2);
							std::memcpy(pmx->description.japanModelName.data(), filename.data(), pmx->description.japanModelLength);
						}

						auto package = this->importAsset(pmx);

						prefab["model"]["uuid"] = package["uuid"];
					}
				}
				else
				{
					prefab["model"]["path"] = (char*)assetPath.u8string().c_str();
				}
			}

			auto transform = gameObject->getComponent<octoon::TransformComponent>();
			if (transform)
				transform->save(prefab["transform"]);

			for (auto& component : gameObject->getComponents())
			{
				if (!component->isA<AnimatorComponent>())
					continue;
				
				auto animation = component->downcast<AnimatorComponent>();
				if (animation->getAnimation())
				{
					auto package = this->createAsset(animation->getAnimation());
					if (package.is_object())
					{
						nlohmann::json animationJson;
						animationJson["data"] = package["uuid"].get<std::string>();
						animationJson["type"] = animation->getAvatar().empty() ? 1 : 0;

						prefab["animation"].push_back(animationJson);
					}
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
					auto package = this->createAsset(materials[i]);
					if (package.is_object())
					{
						nlohmann::json materialJson;
						materialJson["data"] = package["uuid"].get<std::string>();
						materialJson["index"] = i;

						prefab["meshRenderer"]["materials"].push_back(materialJson);
					}
				}
			}

			auto abc = gameObject->getComponent<MeshAnimationComponent>();
			if (abc)
			{
				prefab["alembic"]["path"] = (char*)assetPath.u8string().c_str();

				for (auto& pair : abc->getMaterials())
				{
					auto materialPackage = this->createAsset(pair.second);
					if (materialPackage.is_object())
					{
						nlohmann::json materialJson;
						materialJson["data"] = materialPackage["uuid"];
						materialJson["name"] = pair.first;

						prefab["alembic"]["materials"].push_back(materialJson);
					}
				}
			}

			std::ofstream ifs(outputPath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = prefab.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			auto writePreview = [this](const std::shared_ptr<GameObject>& gameObject, std::filesystem::path outputPath)
			{
				auto texture = AssetPreview::instance()->getAssetPreview(gameObject);
				if (texture)
				{
					auto previewPath = std::filesystem::path(outputPath).append(make_guid() + ".png");
					AssetDatabase::instance()->createAsset(texture, previewPath);
					return previewPath;
				}

				return std::filesystem::path();
			};

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = gameObject->getName();
			package["path"] = (char*)outputPath.u8string().c_str();
			package["preview"] = (char*)writePreview(gameObject, rootPath).u8string().c_str();	

			std::ofstream ifs2(std::filesystem::path(rootPath).append("package.json"), std::ios_base::binary);
			if (ifs2)
			{
				auto dump = package.dump();
				ifs2.write(dump.c_str(), dump.size());
			}

			this->prefabAsset_->addPackage(uuid);

			return package;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(rootPath))
				std::filesystem::remove_all(rootPath);

			throw e;
		}
	}

	std::shared_ptr<RttiObject>
	AssetBundle::loadAsset(std::string_view uuid, const Rtti& type) noexcept(false)
	{
		if (type.isDerivedFrom(Material::getRtti()))
		{
			if (materialAsset_->hasPackage(uuid))
				return this->loadAssetAtPackage<Material>(materialAsset_->getPackage(uuid));
		}
		else if (type.isDerivedFrom(Animation::getRtti()))
		{
			if (motionAsset_->hasPackage(uuid))
				return this->loadAssetAtPackage<Animation>(motionAsset_->getPackage(uuid));
		}
		else if (type.isDerivedFrom(GameObject::getRtti()))
		{
			if (modelAsset_->hasPackage(uuid))
				return this->loadAssetAtPackage(modelAsset_->getPackage(uuid), *PMX::getRtti())->downcast_pointer<GameObject>();
			if (prefabAsset_->hasPackage(uuid))
				return this->loadAssetAtPackage<GameObject>(prefabAsset_->getPackage(uuid));
		}
		else if (type.isDerivedFrom(Texture::getRtti()))
		{
			if (hdriAsset_->hasPackage(uuid))
				return this->loadAssetAtPackage<Texture>(hdriAsset_->getPackage(uuid));
			if (textureAsset_->hasPackage(uuid))
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
			else if (type.isDerivedFrom(PMX::getRtti()))
				asset = AssetDatabase::instance()->loadAssetAtPath<GameObject>(filepath);
			else if (type.isDerivedFrom(GameObject::getRtti()))
			{
				std::ifstream ifs(filepath);
				if (ifs)
				{
					auto prefab = nlohmann::json::parse(ifs);
					
					GameObjectPtr object;

					if (prefab.contains("model"))
					{
						auto modelJson = prefab["model"];
						if (modelJson.contains("uuid"))
							object = AssetBundle::instance()->loadAsset<GameObject>(modelJson["uuid"].get<std::string>());
						else if (modelJson.contains("path"))
							object = AssetDatabase::instance()->loadAssetAtPath<GameObject>(modelJson["path"].get<std::string>());
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

						auto animation = octoon::AssetBundle::instance()->loadAsset<octoon::Animation>(animationJson["data"].get<std::string>());
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
							auto material = octoon::AssetBundle::instance()->loadAsset<octoon::Material>(data);

							materials[index] = std::move(material);
						}

						auto meshRenderer = object->getComponent<octoon::MeshRendererComponent>();
						if (meshRenderer)
							meshRenderer->setMaterials(std::move(materials));
					}

					asset = object;
				}				
			}

			if (asset)
				assetCache_[uuid] = asset;

			return asset;
		}

		return nullptr;
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Texture>& texture) noexcept(false)
	{
		if (texture)
		{
			auto uuid = AssetDatabase::instance()->assetToGuid(texture);
			if (!uuid.empty())
			{
				if (this != AssetBundle::instance())
				{
					if (AssetBundle::instance()->textureAsset_->hasPackage(uuid))
						return AssetBundle::instance()->textureAsset_->getPackage(uuid);
				}

				if (this->hasPackage(uuid) && !this->needUpdate(texture))
					return this->getPackage(uuid);
			}

			auto package = this->importAsset(texture);
			if (package.is_object())
				this->removeUpdateList(texture);

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Animation>& animation) noexcept(false)
	{
		if (animation)
		{
			auto uuid = AssetDatabase::instance()->assetToGuid(animation);
			if (!uuid.empty())
			{
				if (this != AssetBundle::instance())
				{
					if (AssetBundle::instance()->motionAsset_->hasPackage(uuid))
						return AssetBundle::instance()->motionAsset_->getPackage(uuid);
				}

				if (this->hasPackage(uuid) && !this->needUpdate(animation))
					return this->getPackage(uuid);
			}

			auto package = this->importAsset(animation);
			if (package.is_object())
				this->removeUpdateList(animation);

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Material>& material) noexcept(false)
	{
		if (material)
		{
			auto uuid = AssetDatabase::instance()->assetToGuid(material);
			if (!uuid.empty())
			{
				if (this != AssetBundle::instance())
				{
					if (AssetBundle::instance()->materialAsset_->hasPackage(uuid))
						return AssetBundle::instance()->materialAsset_->getPackage(uuid);
				}

				if (this->hasPackage(uuid) && !this->needUpdate(material))
					return this->getPackage(uuid);
			}

			auto package = this->importAsset(material);
			if (package.is_object())
				this->removeUpdateList(material);

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<GameObject>& gameObject) noexcept(false)
	{
		if (gameObject)
		{
			auto uuid = AssetDatabase::instance()->assetToGuid(gameObject);
			if (!uuid.empty())
			{
				if (this != AssetBundle::instance())
				{
					if (AssetBundle::instance()->modelAsset_->hasPackage(uuid))
						return AssetBundle::instance()->modelAsset_->getPackage(uuid);
				}

				if (this->hasPackage(uuid) && !this->needUpdate(gameObject))
					return this->getPackage(uuid);
			}
			else
			{
				uuid = make_guid();
			}

			auto package = this->importAsset(gameObject);
			if (package.is_object())
				this->removeUpdateList(gameObject);

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
		if (this->prefabAsset_->hasPackage(uuid))
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
		if (this->prefabAsset_->hasPackage(uuid))
			return this->prefabAsset_->getPackage(uuid);
		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::getPackage(const std::shared_ptr<RttiObject>& asset) noexcept
	{
		auto guid = AssetDatabase::instance()->assetToGuid(asset);
		if (guid.empty())
			return nlohmann::json();
		return this->getPackage(guid);
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

	nlohmann::json&
	AssetBundle::getPrefabList() const noexcept
	{
		return prefabAsset_->getIndexList();
	}

	void
	AssetBundle::unload() noexcept
	{
		this->assetCache_.clear();
	}

	void
	AssetBundle::saveAssets() noexcept(false)
	{
		for (auto& it : updateList_)
		{
			if (!it.expired())
			{
				auto item = it.lock();
				if (item->isInstanceOf<Material>())
					this->createAsset(item->downcast_pointer<Material>());
			}
		}

		if (std::filesystem::exists(assetPath_))
		{
			this->modelAsset_->saveAssets();
			this->motionAsset_->saveAssets();
			this->materialAsset_->saveAssets();
			this->textureAsset_->saveAssets();
			this->hdriAsset_->saveAssets();
			this->prefabAsset_->saveAssets();
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
		if (this->prefabAsset_->hasPackage(uuid))
			this->prefabAsset_->removeAsset(uuid);
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
	AssetBundle::addUpdateList(const std::shared_ptr<RttiObject>& object) noexcept(false)
	{
		this->updateList_.insert(object);
	}

	bool
	AssetBundle::needUpdate() const noexcept
	{
		return !this->updateList_.empty();
	}

	bool
	AssetBundle::needUpdate(const std::shared_ptr<RttiObject>& object) const noexcept
	{
		return this->updateList_.contains(object);
	}

	void
	AssetBundle::removeUpdateList(const std::shared_ptr<RttiObject>& object) noexcept(false)
	{
		auto it = this->updateList_.find(object);
		if (it != this->updateList_.end())
			this->updateList_.erase(it);
	}

	void
	AssetBundle::clearUpdate() noexcept
	{
		this->updateList_.clear();
	}
}