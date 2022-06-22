#include <octoon/asset_bundle.h>
#include <octoon/asset_loader.h>
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

	std::vector<std::shared_ptr<AssetBundle>> AssetBundle::assetBundles_;

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

		this->assetDatabase_ = std::make_unique<AssetDatabase>();
		this->assetDatabase_->open(assetPath);

		std::ifstream ifs(std::filesystem::path(this->assetPath_).append("Library").append("AssetBundle.json"), std::ios_base::binary);
		if (ifs)
		{
			try
			{
				auto assetDb = nlohmann::json::parse(ifs);
				for (auto it = assetDb.begin(); it != assetDb.end(); ++it)
					packageList_[it.key()] = it.value();
			}
			catch (...)
			{
			}
		}
	}

	void
	AssetBundle::close() noexcept
	{
		this->unload();

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
			auto texture = AssetLoader::instance()->loadAssetAtPath<Texture>(path);
			if (texture)
				return this->importAsset(texture);
		}
		else if (ext == u8".vmd")
		{
			auto animation = AssetLoader::instance()->loadAssetAtPath<Animation>(path);
			if (animation)
				return this->importAsset(animation);
		}
		else if (ext == u8".pmx" || ext == u8".obj" || ext == u8".fbx")
		{
			auto gameObject = AssetLoader::instance()->loadAssetAtPath<GameObject>(path);
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

				this->saveAssets();

				return items;
			}
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<Texture>& texture) noexcept(false)
	{
		auto hdr = (texture->format() == Format::R32G32B32SFloat) ? true : false;
		auto uuid = make_guid();
		auto relativePath = (hdr ? "Assets/HDRis/" : "Assets/Textures/") + uuid;
		
		try
		{
			auto ext = AssetLoader::instance()->getAssetExtension(texture, hdr ? ".hdr" : ".png").string();
			auto outputPath = std::filesystem::path(relativePath).append(uuid + ext);

			this->assetDatabase_->createAsset(texture, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["type"] = texture->type_name();
			package["name"] = texture->getName();
			package["data"] = this->assetDatabase_->getAssetGuid(texture);
			package["visible"] = true;

			auto preview = AssetPreview::instance()->getAssetPreview(texture);
			if (preview)
			{
				auto previewPath = std::filesystem::path(relativePath).append(make_guid() + ".png");
				this->assetDatabase_->createAsset(preview, previewPath);
				package["preview"] = (char*)std::filesystem::path(this->assetPath_).append(previewPath.u8string()).u8string().c_str();
			}

			this->packageList_[uuid] = std::move(package);
			this->assetDatabase_->saveAssets();

			return package;
		}
		catch (const std::exception& e)
		{
			this->assetDatabase_->deleteAsset(relativePath);
			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<Animation>& animation) noexcept(false)
	{
		auto uuid = make_guid();
		auto relativePath = "Assets/Motions/" + uuid;

		try
		{
			auto ext = AssetLoader::instance()->getAssetExtension(animation, ".vmd").string();
			auto outputPath = std::filesystem::path(relativePath).append(uuid + ext);

			this->assetDatabase_->createAsset(animation, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["type"] = animation->type_name();
			package["name"] = animation->getName();
			package["data"] = this->assetDatabase_->getAssetGuid(animation);
			package["visible"] = true;

			this->packageList_[uuid] = std::move(package);
			this->assetDatabase_->saveAssets();

			return package;
		}
		catch (const std::exception& e)
		{
			this->assetDatabase_->deleteAsset(relativePath);
			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<Material>& material) noexcept(false)
	{
		auto uuid = make_guid();
		auto relativePath = "Assets/Materials/" + uuid;

		try
		{
			auto outputPath = std::filesystem::path(relativePath).append(uuid + ".mat");

			this->assetDatabase_->createAsset(material, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["type"] = material->type_name();
			package["name"] = material->getName();
			package["data"] = this->assetDatabase_->getAssetGuid(material);
			package["visible"] = true;

			auto preview = AssetPreview::instance()->getAssetPreview(material);
			if (preview)
			{
				auto previewPath = std::filesystem::path(relativePath).append(make_guid() + ".png");
				this->assetDatabase_->createAsset(preview, previewPath);
				package["preview"] = (char*)std::filesystem::path(this->assetPath_).append(previewPath.u8string()).u8string().c_str();
			}

			this->packageList_[uuid] = std::move(package);
			this->assetDatabase_->saveAssets();

			return package;
		}
		catch (const std::exception& e)
		{
			this->assetDatabase_->deleteAsset(relativePath);
			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<PMX>& pmx) noexcept(false)
	{
		auto uuid = this->assetDatabase_->getAssetGuid(pmx);
		auto rootPath = "Assets/Models/" + uuid;

		try
		{
			auto outputPath = std::filesystem::path(rootPath).append(this->assetDatabase_->getAssetGuid(pmx) + ".pmx");

			this->assetDatabase_->createAsset(pmx, outputPath);

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

			auto writePreview = [this](const PMX& pmx, const std::filesystem::path& outputPath)
			{
				auto texture = AssetPreview::instance()->getAssetPreview(pmx);
				auto previewPath = std::filesystem::path(outputPath).append(make_guid() + ".png");
				this->assetDatabase_->createAsset(texture, previewPath);
				return previewPath;
			};

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["type"] = pmx->type_name();
			package["name"] = cv.to_bytes(pmx->description.japanModelName.data());
			package["data"] = this->assetDatabase_->getAssetGuid(pmx);
			package["preview"] = (char*)writePreview(*pmx, rootPath).u8string().c_str();

			this->packageList_[uuid] = std::move(package);
			this->assetDatabase_->saveAssets();

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
		auto uuid = this->assetDatabase_->getAssetGuid(gameObject);
		auto rootPath = "Assets/Prefabs/" + uuid;

		try
		{
			auto outputPath = std::filesystem::path(rootPath).append(this->assetDatabase_->getAssetGuid(gameObject) + ".prefab");

			std::filesystem::create_directories(rootPath);

			nlohmann::json prefab;

			auto assetPath = AssetLoader::instance()->getAssetPath(gameObject);
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
					nlohmann::json  package;// = this->createAsset(animation->getAnimation());
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
					nlohmann::json  package;// = this->createAsset(materials[i]);
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
					nlohmann::json materialPackage;// = this->createAsset(pair.second);
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

			auto writePreview = [this](const std::shared_ptr<GameObject>& gameObject, const std::filesystem::path& outputPath)
			{
				auto texture = AssetPreview::instance()->getAssetPreview(gameObject);
				if (texture)
				{
					auto previewPath = std::filesystem::path(outputPath).append(make_guid() + ".png");
					this->assetDatabase_->createAsset(texture, previewPath);
					return previewPath;
				}

				return std::filesystem::path();
			};

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["type"] = gameObject->type_name();
			package["name"] = gameObject->getName();
			package["data"] = this->assetDatabase_->getAssetGuid(gameObject);
			package["preview"] = (char*)writePreview(gameObject, rootPath).u8string().c_str();	

			this->packageList_[uuid] = std::move(package);
			this->assetDatabase_->saveAssets();

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
	AssetBundle::loadAsset(const std::string& uuid, const Rtti& type) noexcept(false)
	{
		if (packageList_.contains(uuid))
		{
			if (type.isDerivedFrom(Material::getRtti()))
				return this->loadAssetAtPackage<Material>(packageList_.at(uuid));
			else if (type.isDerivedFrom(Animation::getRtti()))
				return this->loadAssetAtPackage<Animation>(packageList_.at(uuid));
			else if (type.isDerivedFrom(GameObject::getRtti()))
				return this->loadAssetAtPackage<GameObject>(packageList_.at(uuid));
			else if (type.isDerivedFrom(Texture::getRtti()))
				return this->loadAssetAtPackage<Texture>(packageList_.at(uuid));
		}

		if (this == AssetBundle::instance())
		{
			for (auto& ab : assetBundles_)
			{
				auto asset = ab->loadAsset(uuid, type);
				if (asset)
					return asset;
			}
		}

		return nullptr;
	}

	std::shared_ptr<RttiObject>
	AssetBundle::loadAssetAtPackage(const nlohmann::json& package, const Rtti& type) noexcept(false)
	{
		if (package.is_object() && package.contains("uuid") && package.contains("data"))
		{
			auto uuid = package["uuid"].get<std::string>();
			if (this->assetCache_.contains(uuid))
			{
				auto it = this->assetCache_.at(uuid);
				if (!it.expired())
					return it.lock();
			}

			auto data = package["data"].get<std::string>();

			std::shared_ptr<RttiObject> asset;
			if (type.isDerivedFrom(Texture::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<Texture>(this->assetDatabase_->getAssetPath(data));
			else if (type.isDerivedFrom(Animation::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<Animation>(this->assetDatabase_->getAssetPath(data));
			else if (type.isDerivedFrom(Material::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<Material>(this->assetDatabase_->getAssetPath(data));
			else if (type.isDerivedFrom(PMX::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<GameObject>(this->assetDatabase_->getAssetPath(data));
			else if (type.isDerivedFrom(GameObject::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<GameObject>(this->assetDatabase_->getAssetPath(data));

			if (asset)
				assetCache_[uuid] = asset;

			return asset;
		}

		return nullptr;


		/*else if (type.isDerivedFrom(GameObject::getRtti()))
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
						object = this->assetDatabase_->loadAssetAtPath<GameObject>(modelJson["path"].get<std::string>());
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
		}*/
	}

	bool
	AssetBundle::hasPackage(const std::string& uuid) noexcept
	{
		return this->packageList_.contains(uuid);
	}

	nlohmann::json
	AssetBundle::getPackage(const std::string& uuid) noexcept
	{
		auto it = this->packageList_.find(uuid);
		if (it != this->packageList_.end())
			return *(it);
		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::getPackage(const std::shared_ptr<RttiObject>& asset) noexcept
	{
		auto guid = this->assetDatabase_->getAssetGuid(asset);
		if (guid.empty())
			return nlohmann::json();
		return this->getPackage(guid);
	}

	const nlohmann::json&
	AssetBundle::getModelList() const noexcept
	{
		auto it = packageList_.find("type");
		if (it != packageList_.end())
			return (*it).second;
		return nlohmann::json();
	}

	const nlohmann::json&
	AssetBundle::getMotionList() const noexcept
	{
		auto it = packageList_.find("type");
		if (it != packageList_.end())
			return (*it).second;
		return nlohmann::json();
	}

	const nlohmann::json&
	AssetBundle::getTextureList() const noexcept
	{
		auto it = packageList_.find("type");
		if (it != packageList_.end())
			return (*it).second;
		return nlohmann::json();
	}

	const nlohmann::json&
	AssetBundle::getHDRiList() const noexcept
	{
		auto it = packageList_.find("type");
		if (it != packageList_.end())
			return (*it).second;
		return nlohmann::json();
	}

	const nlohmann::json&
	AssetBundle::getMaterialList() const noexcept
	{
		auto it = packageList_.find("type");
		if (it != packageList_.end())
			return (*it).second;
		return nlohmann::json();
	}

	const nlohmann::json&
	AssetBundle::getPrefabList() const noexcept
	{
		auto it = packageList_.find("type");
		if (it != packageList_.end())
			return (*it).second;
		return nlohmann::json();
	}

	void
	AssetBundle::unload() noexcept
	{
		this->assetCache_.clear();
	}

	void
	AssetBundle::saveAssets() noexcept(false)
	{
		nlohmann::json bundleDb;

		for (auto& it : this->packageList_)
			bundleDb[it.first] = it.second;

		auto bundleRoot = std::filesystem::path(this->assetPath_).append("Library");
		std::filesystem::create_directories(bundleRoot);

		std::ofstream ifs(std::filesystem::path(bundleRoot).append("AssetBundle.json"), std::ios_base::binary);
		if (ifs)
		{
			auto dump = bundleDb.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}

		if (this == AssetBundle::instance())
		{
			for (auto& ab : assetBundles_)
				ab->saveAssets();
		}
	}

	void
	AssetBundle::removeAsset(const std::string& uuid) noexcept(false)
	{
		if (this->hasPackage(uuid))
			this->assetDatabase_->deleteAsset(this->assetDatabase_->getAssetPath(uuid));
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