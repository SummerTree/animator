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

		if (ext == u8".hdr")
		{
			auto texture = AssetLoader::instance()->loadAssetAtPath<Texture>(path);
			if (texture)
				return this->importAsset(texture, "Assets/HDRis");
		}
		if (ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = AssetLoader::instance()->loadAssetAtPath<Texture>(path);
			if (texture)
				return this->importAsset(texture, "Assets/HDRis");
		}
		else if (ext == u8".vmd")
		{
			auto animation = AssetLoader::instance()->loadAssetAtPath<Animation>(path);
			if (animation)
				return this->importAsset(animation, "Assets/Motions");
		}
		else if (ext == u8".pmx" || ext == u8".obj" || ext == u8".fbx")
		{
			auto gameObject = AssetLoader::instance()->loadAssetAtPath<GameObject>(path);
			if (gameObject)
				return this->importAsset(gameObject, "Assets/Models");
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
					auto package = this->importAsset(material, "Assets/Materials");
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
	AssetBundle::importAsset(const std::shared_ptr<Texture>& texture, const std::filesystem::path& relativeFolder) noexcept(false)
	{
		auto hdr = (texture->format() == Format::R32G32B32SFloat) ? true : false;
		auto uuid = make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid);
		
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
	AssetBundle::importAsset(const std::shared_ptr<Animation>& animation, const std::filesystem::path& relativeFolder) noexcept(false)
	{
		auto uuid = make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid);

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
	AssetBundle::importAsset(const std::shared_ptr<Material>& material, const std::filesystem::path& relativeFolder) noexcept(false)
	{
		auto uuid = make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid);

		try
		{
			this->assetDatabase_->createAsset(material, std::filesystem::path(relativePath).append(uuid + ".mat"));

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
	AssetBundle::importAsset(const std::shared_ptr<PMX>& pmx, const std::filesystem::path& relativeFolder) noexcept(false)
	{
		auto uuid = make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid);

		try
		{
			this->assetDatabase_->createAsset(pmx, std::filesystem::path(relativePath).append(uuid + ".pmx"));

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
			package["preview"] = (char*)writePreview(*pmx, relativePath).u8string().c_str();

			this->packageList_[uuid] = std::move(package);
			this->assetDatabase_->saveAssets();

			return package;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(relativePath))
				std::filesystem::remove_all(relativePath);

			throw e;
		}
	}

	nlohmann::json
	AssetBundle::importAsset(const std::shared_ptr<GameObject>& gameObject, const std::filesystem::path& relativeFolder) noexcept(false)
	{
		auto uuid = make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid);

		try
		{
			this->assetDatabase_->createAsset(gameObject, std::filesystem::path(relativePath).append(uuid + ".prefab"));

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
			package["preview"] = (char*)writePreview(gameObject, relativePath).u8string().c_str();

			this->packageList_[uuid] = std::move(package);
			this->assetDatabase_->saveAssets();

			return package;
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(relativePath))
				std::filesystem::remove_all(relativePath);

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

	nlohmann::json
	AssetBundle::getPackageList(const Rtti& rtti) const noexcept(false)
	{
		nlohmann::json result;

		for (auto& package : packageList_)
		{
			if (package.second.contains("type"))
			{
				if (package.second["type"] == rtti.type_name())
					result.push_back(package.second);
			}
		}

		return result;
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