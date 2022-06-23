#include "asset_library.h"
#include <octoon/runtime/guid.h>
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
			try
			{
				this->textureDb_ = nlohmann::json::parse(textureLib);
				for (auto& package : this->textureDb_)
				{
					auto it = package.find("uuid");
					if (it != package.end())
						this->packageCache_[(*it).get<std::string>()] = package;
				}
			}
			catch (...)
			{
			}
		}

		std::ifstream environmentLib(std::filesystem::path(libraryPath).append("EnvironmentDB.json"), std::ios_base::binary);
		if (environmentLib)
		{
			try
			{
				this->hdriDb_ = nlohmann::json::parse(environmentLib);
				for (auto& package : this->hdriDb_)
				{
					auto it = package.find("uuid");
					if (it != package.end())
						this->packageCache_[(*it).get<std::string>()] = package;
				}
			}
			catch (...)
			{
			}
		}

		std::ifstream motionLib(std::filesystem::path(libraryPath).append("MotionDB.json"), std::ios_base::binary);
		if (motionLib)
		{
			try
			{
				this->motionDb_ = nlohmann::json::parse(motionLib);
				for (auto& package : this->motionDb_)
				{
					auto it = package.find("uuid");
					if (it != package.end())
						this->packageCache_[(*it).get<std::string>()] = package;
				}
			}
			catch (...)
			{
			}
		}

		std::ifstream materialLib(std::filesystem::path(libraryPath).append("MaterialDB.json"), std::ios_base::binary);
		if (materialLib)
		{
			try
			{
				this->materialDb_ = nlohmann::json::parse(materialLib);
				for (auto& package : this->materialDb_)
				{
					auto it = package.find("uuid");
					if (it != package.end())
						this->packageCache_[(*it).get<std::string>()] = package;
				}
			}
			catch (...)
			{
			}
		}

		std::ifstream prefabLib(std::filesystem::path(libraryPath).append("PrefabDB.json"), std::ios_base::binary);
		if (prefabLib)
		{
			try
			{
				this->prefabDb_ = nlohmann::json::parse(prefabLib);
				for (auto& package : this->prefabDb_)
				{
					auto it = package.find("uuid");
					if (it != package.end())
						this->packageCache_[(*it).get<std::string>()] = package;
				}
			}
			catch (...)
			{
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
				return this->importAsset(texture, "Assets/Textures");
		}
		if (ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto texture = octoon::AssetLoader::instance()->loadAssetAtPath<octoon::Texture>(path);
			if (texture)
				return this->importAsset(texture, "Assets/Textures");
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
					items.push_back(this->importAsset(material, "Assets/Materials"));

				this->saveAssets();

				return items;
			}
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetLibrary::importAsset(const std::shared_ptr<octoon::Texture>& texture, const std::filesystem::path& relativeFolder) noexcept(false)
	{
		auto guid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(guid.substr(0, 2));

		try
		{
			auto hdr = (texture->format() == octoon::Format::R32G32B32SFloat) ? true : false;
			auto ext = octoon::AssetLoader::instance()->getAssetExtension(texture, hdr ? ".hdr" : ".png").string();
			auto outputPath = std::filesystem::path(relativePath).append(guid + ext);

			this->assetDatabase_->createAsset(texture, outputPath);

			nlohmann::json package;
			package["uuid"] = guid;
			package["hdr"] = hdr;
			package["type"] = texture->type_name();
			package["name"] = texture->getName();
			package["data"] = this->assetDatabase_->getAssetGuid(texture);
			package["visible"] = true;

			auto preview = octoon::AssetPreview::instance()->getAssetPreview(texture);
			if (preview)
			{
				auto uuid = octoon::make_guid();
				auto previewPath = std::filesystem::path("Assets/Thumbnails").append(uuid.substr(0, 2)).append(uuid + ".png");
				this->assetDatabase_->createAsset(preview, previewPath);
				package["preview"] = this->assetDatabase_->getAssetGuid(previewPath);
			}

			if (hdr)
				this->hdriDb_.push_back(package);
			else
				this->textureDb_.push_back(package);

			this->packageCache_[guid] = package;
			this->saveAssets();

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
		auto guid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(guid.substr(0, 2));

		try
		{
			auto ext = octoon::AssetLoader::instance()->getAssetExtension(animation, ".vmd").string();
			auto outputPath = std::filesystem::path(relativePath).append(guid + ext);

			this->assetDatabase_->createAsset(animation, outputPath);

			nlohmann::json package;
			package["uuid"] = guid;
			package["type"] = animation->type_name();
			package["name"] = (char*)filename.u8string().c_str();
			package["data"] = this->assetDatabase_->getAssetGuid(animation);
			package["visible"] = true;

			this->motionDb_.push_back(package);
			this->packageCache_[guid] = package;
			this->saveAssets();

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
		auto guid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(guid.substr(0, 2));

		try
		{
			auto standardMaterial = material->downcast<octoon::MeshStandardMaterial>();
			if (standardMaterial->getColorMap() && !this->assetDatabase_->contains(standardMaterial->getColorMap()))
				this->importAsset(standardMaterial->getColorMap(), "Assets/Textures");
			if (standardMaterial->getOpacityMap() && !this->assetDatabase_->contains(standardMaterial->getOpacityMap()))
				this->importAsset(standardMaterial->getOpacityMap(), "Assets/Textures");
			if (standardMaterial->getNormalMap() && !this->assetDatabase_->contains(standardMaterial->getNormalMap()))
				this->importAsset(standardMaterial->getNormalMap(), "Assets/Textures");
			if (standardMaterial->getRoughnessMap() && !this->assetDatabase_->contains(standardMaterial->getRoughnessMap()))
				this->importAsset(standardMaterial->getRoughnessMap(), "Assets/Textures");
			if (standardMaterial->getSpecularMap() && !this->assetDatabase_->contains(standardMaterial->getSpecularMap()))
				this->importAsset(standardMaterial->getSpecularMap(), "Assets/Textures");
			if (standardMaterial->getMetalnessMap() && !this->assetDatabase_->contains(standardMaterial->getMetalnessMap()))
				this->importAsset(standardMaterial->getMetalnessMap(), "Assets/Textures");
			if (standardMaterial->getEmissiveMap() && !this->assetDatabase_->contains(standardMaterial->getEmissiveMap()))
				this->importAsset(standardMaterial->getEmissiveMap(), "Assets/Textures");
			if (standardMaterial->getAnisotropyMap() && !this->assetDatabase_->contains(standardMaterial->getAnisotropyMap()))
				this->importAsset(standardMaterial->getAnisotropyMap(), "Assets/Textures");
			if (standardMaterial->getClearCoatMap() && !this->assetDatabase_->contains(standardMaterial->getClearCoatMap()))
				this->importAsset(standardMaterial->getClearCoatMap(), "Assets/Textures");
			if (standardMaterial->getClearCoatRoughnessMap() && !this->assetDatabase_->contains(standardMaterial->getClearCoatRoughnessMap()))
				this->importAsset(standardMaterial->getClearCoatRoughnessMap(), "Assets/Textures");
			if (standardMaterial->getSubsurfaceMap() && !this->assetDatabase_->contains(standardMaterial->getSubsurfaceMap()))
				this->importAsset(standardMaterial->getSubsurfaceMap(), "Assets/Textures");
			if (standardMaterial->getSubsurfaceColorMap() && !this->assetDatabase_->contains(standardMaterial->getSubsurfaceColorMap()))
				this->importAsset(standardMaterial->getSubsurfaceColorMap(), "Assets/Textures");
			if (standardMaterial->getSheenMap() && !this->assetDatabase_->contains(standardMaterial->getSheenMap()))
				this->importAsset(standardMaterial->getSheenMap(), "Assets/Textures");
			if (standardMaterial->getLightMap() && !this->assetDatabase_->contains(standardMaterial->getLightMap()))
				this->importAsset(standardMaterial->getLightMap(), "Assets/Textures");

			this->assetDatabase_->createAsset(material, std::filesystem::path(relativePath).append(guid + ".mat"));

			nlohmann::json package;
			package["uuid"] = guid;
			package["type"] = material->type_name();
			package["name"] = material->getName();
			package["data"] = this->assetDatabase_->getAssetGuid(material);
			package["visible"] = true;

			auto preview = octoon::AssetPreview::instance()->getAssetPreview(material);
			if (preview)
			{
				auto uuid = octoon::make_guid();
				auto previewPath = std::filesystem::path("Assets/Thumbnails").append(uuid.substr(0, 2)).append(uuid + ".png");
				this->assetDatabase_->createAsset(preview, previewPath);
				package["preview"] = this->assetDatabase_->getAssetGuid(previewPath);
			}

			this->materialDb_.push_back(package);
			this->packageCache_[guid] = package;

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
		auto guid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(guid);

		try
		{
			this->assetDatabase_->createAsset(pmx, std::filesystem::path(relativePath).append(guid + ".pmx"));

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

			nlohmann::json package;
			package["uuid"] = guid;
			package["visible"] = true;
			package["type"] = pmx->type_name();
			package["name"] = cv.to_bytes(pmx->description.japanModelName.data());
			package["data"] = this->assetDatabase_->getAssetGuid(pmx);

			auto texture = octoon::AssetPreview::instance()->getAssetPreview(*pmx);
			if (texture)
			{
				auto uuid = octoon::make_guid();
				auto previewPath = std::filesystem::path("Assets/Thumbnails").append(uuid.substr(0, 2)).append(uuid + ".png");
				this->assetDatabase_->createAsset(texture, previewPath);
				package["preview"] = this->assetDatabase_->getAssetGuid(previewPath);
			}

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
		auto guid = octoon::make_guid();
		auto relativePath = std::filesystem::path(relativeFolder).append(guid);

		try
		{
			this->assetDatabase_->createAsset(gameObject, std::filesystem::path(relativePath).append(guid + ".prefab"));

			nlohmann::json package;
			package["uuid"] = guid;
			package["visible"] = true;
			package["type"] = gameObject->type_name();
			package["name"] = gameObject->getName();
			package["filename"] = (char*)filename.u8string().c_str();
			package["data"] = this->assetDatabase_->getAssetGuid(gameObject);

			auto texture = octoon::AssetPreview::instance()->getAssetPreview(gameObject);
			if (texture)
			{
				auto uuid = octoon::make_guid();
				auto previewPath = std::filesystem::path("Assets/Thumbnails").append(uuid.substr(0, 2)).append(uuid + ".png");
				this->assetDatabase_->createAsset(texture, previewPath);
				package["preview"] = this->assetDatabase_->getAssetGuid(previewPath);
			}

			this->prefabDb_.push_back(package);
			this->packageCache_[guid] = package;

			this->saveAssets();

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
		if (packageCache_.contains(uuid))
		{
			if (type.isDerivedFrom(octoon::Material::getRtti()))
				return this->loadAssetAtPackage<octoon::Material>(packageCache_.at(uuid));
			else if (type.isDerivedFrom(octoon::Animation::getRtti()))
				return this->loadAssetAtPackage<octoon::Animation>(packageCache_.at(uuid));
			else if (type.isDerivedFrom(octoon::GameObject::getRtti()))
				return this->loadAssetAtPackage<octoon::GameObject>(packageCache_.at(uuid));
			else if (type.isDerivedFrom(octoon::Texture::getRtti()))
				return this->loadAssetAtPackage<octoon::Texture>(packageCache_.at(uuid));
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
				assetGuidCache_[asset] = uuid;
			}

			return asset;
		}

		return nullptr;
	}

	bool
	AssetLibrary::hasPackage(const std::string& uuid) noexcept
	{
		return packageCache_.contains(uuid);
	}

	nlohmann::json
	AssetLibrary::getPackage(const std::string& uuid) const noexcept
	{
		if (packageCache_.contains(uuid))
			return packageCache_.at(uuid);
		return nlohmann::json();
	}

	nlohmann::json
	AssetLibrary::getPackage(const std::shared_ptr<octoon::RttiObject>& asset) const noexcept
	{
		if (assetGuidCache_.contains(asset))
			return this->getPackage(assetGuidCache_.at(asset));
		return nlohmann::json();
	}

	std::filesystem::path
	AssetLibrary::getAssetPath(const std::string& uuid, bool absolutePath) const noexcept
	{
		auto path = this->assetDatabase_->getAssetPath(uuid);
		if (absolutePath && !path.empty())
			return std::filesystem::path(this->assetPath_).append(path.wstring());
		return path;
	}

	std::filesystem::path
	AssetLibrary::getAssetPath(const std::shared_ptr<const octoon::RttiObject>& asset, bool absolutePath) const noexcept
	{
		auto path = this->assetDatabase_->getAssetPath(asset);
		if (absolutePath && !path.empty())
			return std::filesystem::path(this->assetPath_).append(path.wstring());
		return path;
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

		std::ofstream textureStream(std::filesystem::path(libraryPath).append("TextureDB.json"), std::ios_base::binary);
		if (textureStream)
		{
			auto dump = textureDb_.dump();
			textureStream.write(dump.c_str(), dump.size());
			textureStream.close();
		}

		std::ofstream materialStream(std::filesystem::path(libraryPath).append("MaterialDB.json"), std::ios_base::binary);
		if (materialStream)
		{
			auto dump = materialDb_.dump();
			materialStream.write(dump.c_str(), dump.size());
			materialStream.close();
		}

		std::ofstream environmentStream(std::filesystem::path(libraryPath).append("EnvironmentDB.json"), std::ios_base::binary);
		if (environmentStream)
		{
			auto dump = hdriDb_.dump();
			environmentStream.write(dump.c_str(), dump.size());
			environmentStream.close();
		}

		std::ofstream animationStream(std::filesystem::path(libraryPath).append("MotionDB.json"), std::ios_base::binary);
		if (animationStream)
		{
			auto dump = motionDb_.dump();
			animationStream.write(dump.c_str(), dump.size());
			animationStream.close();
		}

		std::ofstream prefabStream(std::filesystem::path(libraryPath).append("PrefabDB.json"), std::ios_base::binary);
		if (prefabStream)
		{
			auto dump = prefabDb_.dump();
			prefabStream.write(dump.c_str(), dump.size());
			prefabStream.close();
		}
	}

	void
	AssetLibrary::removeAsset(const std::string& uuid) noexcept(false)
	{
		auto cache = packageCache_.find(uuid);
		if (cache == packageCache_.end())
			return;

		auto package = (*cache).second;
		packageCache_.erase(cache);

		auto deleteAsset = [this](const std::string& uuid)
		{
			try
			{
				auto path = this->getAssetPath(uuid);
				if (!path.empty())
				{
					if (this->assetDatabase_)
						this->assetDatabase_->deleteAsset(path);

					auto parent_path = std::filesystem::path(this->assetPath_).append(path.parent_path().wstring());
					if (std::filesystem::is_empty(parent_path))
						std::filesystem::remove(parent_path);
				}
			}
			catch (...)
			{
			}
		};

		if (package.contains("data"))
			deleteAsset(package["data"].get<std::string>());

		if (package.contains("preview"))
			deleteAsset(package["preview"].get<std::string>());

		if (package.contains("type"))
		{
			auto type = package["type"].get<std::string>();
			if (type == octoon::Texture::getRtti()->type_name())
			{
				auto hdr = package.contains("hdr") ? package["hdr"].get<bool>() : false;
				if (hdr)
				{
					for (auto it = this->hdriDb_.begin(); it != this->hdriDb_.end(); ++it)
					{
						auto uuid_ = (*it)["uuid"].get<std::string>();
						if (uuid_ == uuid)
						{
							this->hdriDb_.erase(it);
							break;
						}
					}
				}
				else
				{
					for (auto it = this->hdriDb_.begin(); it != this->hdriDb_.end(); ++it)
					{
						auto uuid_ = (*it)["uuid"].get<std::string>();
						if (uuid_ == uuid)
						{
							this->hdriDb_.erase(it);
							break;
						}
					}
				}
			}
			else if (type == octoon::Animation::getRtti()->type_name())
			{
				for (auto it = this->motionDb_.begin(); it != this->motionDb_.end(); ++it)
				{
					auto uuid_ = (*it)["uuid"].get<std::string>();
					if (uuid_ == uuid)
					{
						this->motionDb_.erase(it);
						break;
					}
				}
			}
			else if (type == octoon::Material::getRtti()->type_name())
			{
				for (auto it = this->materialDb_.begin(); it != this->materialDb_.end(); ++it)
				{
					auto uuid_ = (*it)["uuid"].get<std::string>();
					if (uuid_ == uuid)
					{
						this->materialDb_.erase(it);
						break;
					}
				}
			}
			else if (type == octoon::GameObject::getRtti()->type_name())
			{
				for (auto it = this->prefabDb_.begin(); it != this->prefabDb_.end(); ++it)
				{
					auto uuid_ = (*it)["uuid"].get<std::string>();
					if (uuid_ == uuid)
					{
						this->prefabDb_.erase(it);
						break;
					}
				}
			}
		}
	}

	const nlohmann::json&
	AssetLibrary::getMotionList() const noexcept
	{
		return motionDb_;
	}

	const nlohmann::json&
	AssetLibrary::getTextureList() const noexcept
	{
		return textureDb_;
	}

	const nlohmann::json&
	AssetLibrary::getHDRiList() const noexcept
	{
		return hdriDb_;
	}

	const nlohmann::json&
	AssetLibrary::getMaterialList() const noexcept
	{
		return materialDb_;
	}

	const nlohmann::json&
	AssetLibrary::getPrefabList() const noexcept
	{
		return prefabDb_;
	}
}