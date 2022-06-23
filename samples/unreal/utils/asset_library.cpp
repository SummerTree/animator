#include "asset_library.h"
#include <octoon/runtime/uuid.h>
#include <octoon/asset_loader.h>
#include <octoon/asset_preview.h>
#include <octoon/mdl_loader.h>
#include <fstream>
#include <filesystem>
#include <set>

namespace unreal
{
	OctoonImplementSingleton(AssetLibrary)

	AssetLibrary::AssetLibrary() noexcept
	{
	}

	AssetLibrary::~AssetLibrary() noexcept
	{
		this->close();
	}

	void
	AssetLibrary::open(const std::filesystem::path& path) noexcept(false)
	{
		this->assetPath_ = path;

		this->assetDatabase_ = std::make_unique<octoon::AssetDatabase>();
		this->assetDatabase_->open(path);

		auto libraryPath = std::filesystem::path(path).append("Library");

		std::ifstream textureLib(std::filesystem::path(libraryPath).append("TextureDB.json"), std::ios_base::binary);
		if (textureLib)
		{
			try {
				this->textureList_ = nlohmann::json::parse(textureLib);
			}
			catch (...){
			}
		}

		std::ifstream environmentLib(std::filesystem::path(libraryPath).append("EnvironmentDB.json"), std::ios_base::binary);
		if (environmentLib)
		{
			try {
				this->hdriList_ = nlohmann::json::parse(environmentLib);
			}
			catch (...) {
			}
		}

		std::ifstream motionLib(std::filesystem::path(libraryPath).append("MotionDB.json"), std::ios_base::binary);
		if (motionLib)
		{
			try {
				this->motionList_ = nlohmann::json::parse(motionLib);
			}
			catch (...) {
			}
		}

		std::ifstream materialLib(std::filesystem::path(libraryPath).append("MaterialDB.json"), std::ios_base::binary);
		if (materialLib)
		{
			try {
				this->materialList_ = nlohmann::json::parse(materialLib);
			}
			catch (...) {
			}
		}

		std::ifstream prefabLib(std::filesystem::path(libraryPath).append("PrefabDB.json"), std::ios_base::binary);
		if (prefabLib)
		{
			try {
				this->prefabList_ = nlohmann::json::parse(prefabLib);
			}
			catch (...) {
			}
		}
	}

	void
	AssetLibrary::close() noexcept
	{
		this->unload();
		this->assetPath_.clear();
	}

	nlohmann::json
	AssetLibrary::importAsset(const std::filesystem::path& path) noexcept(false)
	{
		auto ext = path.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == u8".hdr")
		{
			auto texture = octoon::AssetLoader::instance()->loadAssetAtPath<octoon::Texture>(path);
			if (texture)
				return this->importAsset(texture, "Assets/HDRis", path.filename());
		}
		if (ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = octoon::AssetLoader::instance()->loadAssetAtPath<octoon::Texture>(path);
			if (texture)
				return this->importAsset(texture, "Assets/Textures", path.filename());
		}
		else if (ext == u8".vmd")
		{
			auto animation = octoon::AssetLoader::instance()->loadAssetAtPath<octoon::Animation>(path);
			if (animation)
				return this->importAsset(animation, "Assets/Motions", path.filename());
		}
		else if (ext == u8".pmx" || ext == u8".obj" || ext == u8".fbx")
		{
			auto gameObject = octoon::AssetLoader::instance()->loadAssetAtPath<octoon::GameObject>(path);
			if (gameObject)
				return this->importAsset(gameObject, "Assets/Models", path.filename());
		}
		else if (ext == u8".mdl")
		{
			std::ifstream stream(path);
			if (stream)
			{
				nlohmann::json items;

				octoon::MDLLoader loader;
				loader.load("resource", stream);

				for (auto& material : loader.getMaterials())
				{
					this->importAsset(material, "Assets/Materials");
				}

				this->saveAssets();

				return items;
			}
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetLibrary::importAsset(const std::shared_ptr<octoon::Texture>& texture, const std::filesystem::path& relativeFolder, const std::filesystem::path& filename) noexcept(false)
	{
		auto uuid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid.substr(0, 2));

		try
		{
			auto hdr = (texture->format() == octoon::Format::R32G32B32SFloat) ? true : false;
			auto ext = octoon::AssetLoader::instance()->getAssetExtension(texture, hdr ? ".hdr" : ".png").string();
			auto outputPath = std::filesystem::path(relativePath).append(uuid + ext);

			this->assetDatabase_->createAsset(texture, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["type"] = texture->type_name();
			package["name"] = texture->getName();
			package["filename"] = (char*)filename.u8string().c_str();
			package["data"] = this->assetDatabase_->getAssetGuid(texture);
			package["visible"] = true;

			auto preview = octoon::AssetPreview::instance()->getAssetPreview(texture);
			if (preview)
			{
				auto previewPath = std::filesystem::path(relativePath).append(octoon::make_guid() + ".png");
				this->assetDatabase_->createAsset(preview, previewPath);
				package["preview"] = (char*)std::filesystem::path(this->assetPath_).append(previewPath.u8string()).u8string().c_str();
			}

			if (hdr)
				this->hdriList_[uuid] = package; 
			else
				this->textureList_[uuid] = package;

			this->assetDatabase_->saveAssets();

			return std::move(package);
		}
		catch (const std::exception& e)
		{
			this->assetDatabase_->deleteAsset(relativePath);
			throw e;
		}
	}

	nlohmann::json
	AssetLibrary::importAsset(const std::shared_ptr<octoon::Animation>& animation, const std::filesystem::path& relativeFolder, const std::filesystem::path& filename) noexcept(false)
	{
		auto uuid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid);

		try
		{
			auto ext = octoon::AssetLoader::instance()->getAssetExtension(animation, ".vmd").string();
			auto outputPath = std::filesystem::path(relativePath).append(uuid + ext);

			this->assetDatabase_->createAsset(animation, outputPath);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["type"] = animation->type_name();
			package["name"] = animation->getName();
			package["filename"] = (char*)filename.u8string().c_str();
			package["data"] = this->assetDatabase_->getAssetGuid(animation);
			package["visible"] = true;

			this->motionList_.push_back(package);
			this->assetDatabase_->saveAssets();

			return std::move(package);
		}
		catch (const std::exception& e)
		{
			this->assetDatabase_->deleteAsset(relativePath);
			throw e;
		}
	}

	nlohmann::json
	AssetLibrary::importAsset(const std::shared_ptr<octoon::Material>& material, const std::filesystem::path& relativeFolder) noexcept(false)
	{
		auto uuid = octoon::make_guid();
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

			auto preview = octoon::AssetPreview::instance()->getAssetPreview(material);
			if (preview)
			{
				auto previewPath = std::filesystem::path(relativePath).append(octoon::make_guid() + ".png");
				this->assetDatabase_->createAsset(preview, previewPath);
				package["preview"] = (char*)std::filesystem::path(this->assetPath_).append(previewPath.u8string()).u8string().c_str();
			}

			this->materialList_.push_back(package);
			this->assetDatabase_->saveAssets();

			return std::move(package);
		}
		catch (const std::exception& e)
		{
			this->assetDatabase_->deleteAsset(relativePath);
			throw e;
		}
	}

	nlohmann::json
	AssetLibrary::importAsset(const std::shared_ptr<octoon::PMX>& pmx, const std::filesystem::path& relativeFolder) noexcept(false)
	{
		auto uuid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid);

		try
		{
			this->assetDatabase_->createAsset(pmx, std::filesystem::path(relativePath).append(uuid + ".pmx"));

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

			auto writePreview = [this](const octoon::PMX& pmx, const std::filesystem::path& outputPath)
			{
				auto texture = octoon::AssetPreview::instance()->getAssetPreview(pmx);
				auto previewPath = std::filesystem::path(outputPath).append(octoon::make_guid() + ".png");
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

			this->modelList_.push_back(package);
			this->assetDatabase_->saveAssets();

			return std::move(package);
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(relativePath))
				std::filesystem::remove_all(relativePath);

			throw e;
		}
	}

	nlohmann::json
	AssetLibrary::importAsset(const std::shared_ptr<octoon::GameObject>& gameObject, const std::filesystem::path& relativeFolder, const std::filesystem::path& filename) noexcept(false)
	{
		auto uuid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(uuid);

		try
		{
			this->assetDatabase_->createAsset(gameObject, std::filesystem::path(relativePath).append(uuid + ".prefab"));

			auto writePreview = [this](const std::shared_ptr<octoon::GameObject>& gameObject, const std::filesystem::path& outputPath)
			{
				auto texture = octoon::AssetPreview::instance()->getAssetPreview(gameObject);
				if (texture)
				{
					auto previewPath = std::filesystem::path(outputPath).append(octoon::make_guid() + ".png");
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
			package["filename"] = (char*)filename.u8string().c_str();
			package["data"] = this->assetDatabase_->getAssetGuid(gameObject);
			package["preview"] = (char*)writePreview(gameObject, relativePath).u8string().c_str();

			this->prefabList_.push_back(package);
			this->assetDatabase_->saveAssets();

			return std::move(package);
		}
		catch (const std::exception& e)
		{
			if (std::filesystem::exists(relativePath))
				std::filesystem::remove_all(relativePath);

			throw e;
		}
	}

	std::shared_ptr<octoon::RttiObject>
	AssetLibrary::loadAsset(const std::string& uuid, const octoon::Rtti& type) noexcept(false)
	{
		if (type.isDerivedFrom(octoon::Material::getRtti()))
		{
			if (materialList_.contains(uuid))
				return this->loadAssetAtPackage<octoon::Material>(materialList_.at(uuid));
		}
		else if (type.isDerivedFrom(octoon::Animation::getRtti()))
		{
			if (motionList_.contains(uuid))
				return this->loadAssetAtPackage<octoon::Animation>(motionList_.at(uuid));
		}
		else if (type.isDerivedFrom(octoon::GameObject::getRtti()))
		{
			if (prefabList_.contains(uuid))
				return this->loadAssetAtPackage<octoon::GameObject>(prefabList_.at(uuid));
		}
		else if (type.isDerivedFrom(octoon::Texture::getRtti()))
		{
			if (hdriList_.contains(uuid))
				return this->loadAssetAtPackage<octoon::Texture>(hdriList_.at(uuid));
			if (textureList_.contains(uuid))
				return this->loadAssetAtPackage<octoon::Texture>(textureList_.at(uuid));
		}

		return nullptr;
	}

	std::shared_ptr<octoon::RttiObject>
	AssetLibrary::loadAssetAtPackage(const nlohmann::json& package, const octoon::Rtti& type) noexcept(false)
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

			std::shared_ptr<octoon::RttiObject> asset;
			if (type.isDerivedFrom(octoon::Texture::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<octoon::Texture>(this->assetDatabase_->getAssetPath(data));
			else if (type.isDerivedFrom(octoon::Animation::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<octoon::Animation>(this->assetDatabase_->getAssetPath(data));
			else if (type.isDerivedFrom(octoon::Material::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<octoon::Material>(this->assetDatabase_->getAssetPath(data));
			else if (type.isDerivedFrom(octoon::PMX::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<octoon::GameObject>(this->assetDatabase_->getAssetPath(data));
			else if (type.isDerivedFrom(octoon::GameObject::getRtti()))
				asset = this->assetDatabase_->loadAssetAtPath<octoon::GameObject>(this->assetDatabase_->getAssetPath(data));

			if (asset)
			{
				assetCache_[uuid] = asset;
				packageCache_[asset] = uuid;
			}

			return asset;
		}

		return nullptr;
	}

	bool
	AssetLibrary::hasPackage(const std::string& uuid) noexcept
	{
		if (materialList_.contains(uuid)) return true;
		if (motionList_.contains(uuid)) return true;
		if (prefabList_.contains(uuid)) return true;
		if (hdriList_.contains(uuid)) return true;
		if (textureList_.contains(uuid)) return true;
		return false;
	}

	nlohmann::json
	AssetLibrary::getPackage(const std::string& uuid) const noexcept
	{
		if (materialList_.contains(uuid)) return materialList_.at(uuid);
		if (motionList_.contains(uuid)) return motionList_.at(uuid);
		if (prefabList_.contains(uuid)) return prefabList_.at(uuid);
		if (hdriList_.contains(uuid)) return hdriList_.at(uuid);
		if (textureList_.contains(uuid)) return textureList_.at(uuid);
		return nlohmann::json();
	}

	nlohmann::json
	AssetLibrary::getPackage(const std::shared_ptr<octoon::RttiObject>& asset) const noexcept
	{
		if (packageCache_.contains(asset))
			return this->getPackage(packageCache_.at(asset));
		return nlohmann::json();
	}

	void
	AssetLibrary::unload() noexcept
	{
		this->assetCache_.clear();
	}

	void
	AssetLibrary::saveAssets() noexcept(false)
	{
		if (this->assetDatabase_)
			this->assetDatabase_->saveAssets();

		auto libraryPath = std::filesystem::path(this->assetPath_).append("Library");
		std::filesystem::create_directories(libraryPath);

		std::ofstream textureStream(std::filesystem::path(libraryPath).append("TextireDB.json"), std::ios_base::binary);
		if (textureStream)
		{
			auto dump = textureList_.dump();
			textureStream.write(dump.c_str(), dump.size());
			textureStream.close();
		}

		std::ofstream materialStream(std::filesystem::path(libraryPath).append("MaterialDB.json"), std::ios_base::binary);
		if (materialStream)
		{
			auto dump = materialList_.dump();
			materialStream.write(dump.c_str(), dump.size());
			materialStream.close();
		}

		std::ofstream environmentStream(std::filesystem::path(libraryPath).append("EnvironmentDB.json"), std::ios_base::binary);
		if (environmentStream)
		{
			auto dump = hdriList_.dump();
			environmentStream.write(dump.c_str(), dump.size());
			environmentStream.close();
		}

		std::ofstream animationStream(std::filesystem::path(libraryPath).append("AnimationDB.json"), std::ios_base::binary);
		if (animationStream)
		{
			auto dump = motionList_.dump();
			animationStream.write(dump.c_str(), dump.size());
			animationStream.close();
		}

		std::ofstream prefabStream(std::filesystem::path(libraryPath).append("PrefabDB.json"), std::ios_base::binary);
		if (prefabStream)
		{
			auto dump = prefabList_.dump();
			prefabStream.write(dump.c_str(), dump.size());
			prefabStream.close();
		}
	}

	void
	AssetLibrary::removeAsset(const std::string& uuid) noexcept(false)
	{
		if (this->hasPackage(uuid))
		{
			if (this->assetDatabase_)
				this->assetDatabase_->deleteAsset(this->assetDatabase_->getAssetPath(uuid));
		}
	}

	const nlohmann::json&
	AssetLibrary::getModelList() const noexcept
	{
		return modelList_;
	}

	const nlohmann::json&
	AssetLibrary::getMotionList() const noexcept
	{
		return motionList_;
	}

	const nlohmann::json&
	AssetLibrary::getTextureList() const noexcept
	{
		return textureList_;
	}

	const nlohmann::json&
	AssetLibrary::getHDRiList() const noexcept
	{
		return hdriList_;
	}

	const nlohmann::json&
	AssetLibrary::getMaterialList() const noexcept
	{
		return materialList_;
	}

	const nlohmann::json&
	AssetLibrary::getPrefabList() const noexcept
	{
		return prefabList_;
	}
}