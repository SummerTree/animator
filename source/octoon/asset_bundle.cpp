#include <octoon/asset_bundle.h>
#include <octoon/asset_database.h>
#include <octoon/mdl_loader.h>
#include <octoon/io/fstream.h>
#include <octoon/texture/texture.h>
#include <octoon/runtime/uuid.h>
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
	}

	void
	AssetBundle::open(std::string assetPath) noexcept(false)
	{
		auto modelPath = std::filesystem::path(assetPath).append("model").string();
		auto motionPath = std::filesystem::path(assetPath).append("motion").string();
		auto materialPath = std::filesystem::path(assetPath).append("material").string();
		auto texturePath = std::filesystem::path(assetPath).append("texture").string();

		this->modelAsset_ = std::make_unique<AssetImporter>();
		this->modelAsset_->open(modelPath);

		this->motionAsset_ = std::make_unique<AssetImporter>();
		this->motionAsset_->open(motionPath);

		this->materialAsset_ = std::make_unique<AssetImporter>();
		this->materialAsset_->open(materialPath);

		this->textureAsset_ = std::make_unique<AssetImporter>();
		this->textureAsset_->open(texturePath);
	}

	void
	AssetBundle::close() noexcept
	{
	}

	void
	AssetBundle::unload() noexcept
	{
		this->assetCache_.clear();
		this->assetPackageCache_.clear();

		this->modelAsset_->clearCache();
		this->motionAsset_->clearCache();
		this->materialAsset_->clearCache();
		this->motionAsset_->clearCache();
	}

	void
	AssetBundle::saveAssets() noexcept(false)
	{
		this->modelAsset_->saveAssets();
		this->motionAsset_->saveAssets();
		this->materialAsset_->saveAssets();
		this->motionAsset_->saveAssets();
	}

	std::shared_ptr<RttiObject>
	AssetBundle::loadAsset(std::string_view uuid_, const Rtti& type) noexcept(false)
	{
		if (type.isDerivedFrom(Material::getRtti()))
		{
			auto package = materialAsset_->getPackage(uuid_);
			if (package.is_object())
			{
				auto uuid = std::string(uuid_);
				auto it = this->assetCache_.find(uuid);
				if (it != this->assetCache_.end())
					return (*it).second->downcast_pointer<Material>();

				auto asset = AssetDatabase::instance()->loadAssetAtPackage<Material>(package);
				if (asset)
					assetCache_[uuid] = asset;

				return asset;
			}
		}
		else if (type.isDerivedFrom(GameObject::getRtti()))
		{
			auto package = modelAsset_->getPackage(uuid_);
			if (package.is_object())
			{
				auto uuid = std::string(uuid_);
				auto it = this->assetCache_.find(uuid);
				if (it != this->assetCache_.end())
					return (*it).second->downcast_pointer<GameObject>();
			
				auto asset = AssetDatabase::instance()->loadAssetAtPackage<GameObject>(package);
				if (asset)
					assetCache_[uuid] = asset;

				return asset;
			}
		}
		else if (type.isDerivedFrom(Animation::getRtti()))
		{
			auto package = motionAsset_->getPackage(uuid_);
			if (package.is_object())
			{
				auto uuid = std::string(uuid_);
				auto it = this->assetCache_.find(uuid);
				if (it != this->assetCache_.end())
					return (*it).second->downcast_pointer<Animation>();

				auto asset = AssetDatabase::instance()->loadAssetAtPackage<Animation>(package);
				if (asset)
					assetCache_[uuid] = asset;

				return asset;
			}
		}
		else if (type.isDerivedFrom(Texture::getRtti()))
		{
			auto package = textureAsset_->getPackage(uuid_);
			if (package.is_object())
			{
				auto uuid = std::string(uuid_);
				auto it = this->assetCache_.find(uuid);
				if (it != this->assetCache_.end())
					return (*it).second->downcast_pointer<Texture>();

				auto asset = AssetDatabase::instance()->loadAssetAtPackage<Texture>(package);
				if (asset)
					assetCache_[uuid] = asset;

				return asset;
			}
		}

		return nullptr;
	}

	nlohmann::json
	AssetBundle::importAsset(std::string_view path, bool generateMipmap) noexcept(false)
	{
		auto ext = std::string(path.substr(path.find_last_of('.')));
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == ".vmd")
		{
			auto package = AssetDatabase::instance()->createAsset(path, motionAsset_->getAssertPath());
			if (package.is_object())
			{
				motionAsset_->addIndex(package["uuid"].get<std::string>());
				return package;
			}
		}
		else if (ext == ".pmx")
		{
			PMX pmx;

			if (PMX::load(path, pmx))
			{
				if (pmx.description.japanModelLength == 0)
				{
					std::wstring wfilepath = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(std::string(path));
					auto filename = std::filesystem::path(wfilepath).filename().wstring();

					pmx.description.japanCommentName.resize(filename.size() + 1);
					pmx.description.japanModelLength = static_cast<PmxUInt32>(pmx.description.japanCommentName.size() * 2);
					std::memcpy(pmx.description.japanCommentName.data(), filename.data(), pmx.description.japanModelLength);
				}

				auto package = AssetDatabase::instance()->createAsset(pmx, modelAsset_->getAssertPath());
				if (package.is_object())
				{
					modelAsset_->addIndex(package["uuid"].get<std::string>());
					return package;
				}
			}
		}
		else if (ext == ".hdr" || ext == ".bmp" || ext == ".tga" || ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".dds")
		{
			Texture texture;

			if (texture.load(std::string(path)))
			{
				texture.setName(path);
				auto package = AssetDatabase::instance()->createAsset(texture, textureAsset_->getAssertPath());
				textureAsset_->addIndex(package["uuid"].get<std::string>());
				return package;
			}
		}
		else if (ext == ".mdl")
		{
			io::ifstream stream((std::string)path);
			if (stream)
			{
				MDLLoader loader;
				loader.load("resource", stream);

				nlohmann::json items;

				for (auto& mat : loader.getMaterials())
				{
					auto package = AssetDatabase::instance()->createAsset(mat, materialAsset_->getAssertPath());

					items.push_back(package["uuid"]);
					materialAsset_->addIndex(package["uuid"]);
				}

				materialAsset_->saveAssets();

				return items;
			}
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Texture>& texture) noexcept(false)
	{
		if (texture)
		{
			auto it = this->assetPackageCache_.find(texture);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_[texture];

			auto uuid = AssetDatabase::instance()->getAssetGuid(texture);

			nlohmann::json package = AssetDatabase::instance()->getPackage(texture);
			if (package.find("uuid") != package.end())
			{
				for (auto& index : textureAsset_->getIndexList())
				{
					if (index == uuid)
						return package;
				}
			}

			package = AssetDatabase::instance()->createAsset(*texture, textureAsset_->getAssertPath());
			assetPackageCache_[texture] = package;

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Animation>& animation) noexcept(false)
	{
		if (animation)
		{
			auto it = this->assetPackageCache_.find(animation);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_[animation];

			auto uuid = AssetDatabase::instance()->getAssetGuid(animation);

			nlohmann::json package = AssetDatabase::instance()->getPackage(animation);
			if (package.find("uuid") != package.end())
			{
				for (auto& index : motionAsset_->getIndexList())
				{
					if (index == uuid)
						return package;
				}
			}

			package = AssetDatabase::instance()->createAsset(*animation, motionAsset_->getAssertPath());
			assetPackageCache_[animation] = package;

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<Material>& material) noexcept(false)
	{
		if (material)
		{
			auto it = this->assetPackageCache_.find(material);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_[material];

			auto uuid = AssetDatabase::instance()->getAssetGuid(material);

			nlohmann::json package = AssetDatabase::instance()->getPackage(material);
			if (package.find("uuid") != package.end())
			{
				for (auto& index : materialAsset_->getIndexList())
				{
					if (index == uuid)
						return package;
				}
			}

			package = AssetDatabase::instance()->createAsset(material, materialAsset_->getAssertPath());
			this->assetPackageCache_[material] = package;

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createAsset(const std::shared_ptr<GameObject>& gameObject) noexcept(false)
	{
		if (gameObject)
		{
			auto it = this->assetPackageCache_.find(gameObject);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_.at(gameObject);

			auto assetPath = AssetDatabase::instance()->getAssetPath(gameObject);
			if (!assetPath.empty())
			{
				nlohmann::json json;
				json["path"] = assetPath;

				return json;
			}
		}

		return nlohmann::json();
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

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::getPackage(const std::shared_ptr<RttiObject>& asset) const noexcept(false)
	{
		if (this->modelAsset_->hasPackage(asset))
			return this->modelAsset_->getPackage(asset);
		if (this->motionAsset_->hasPackage(asset))
			return this->motionAsset_->getPackage(asset);
		if (this->materialAsset_->hasPackage(asset))
			return this->materialAsset_->getPackage(asset);
		if (this->textureAsset_->hasPackage(asset))
			return this->textureAsset_->getPackage(asset);

		return nlohmann::json();
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
	AssetBundle::getMaterialList() const noexcept
	{
		return materialAsset_->getIndexList();
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
	}

	std::shared_ptr<AssetBundle>
	AssetBundle::loadFromFile(std::string_view path) noexcept(false)
	{
		auto ab = std::make_shared<AssetBundle>();
		ab->open((std::string)path);
		assetBundles_.push_back(ab);

		return ab;
	}

	std::vector<std::shared_ptr<AssetBundle>>
	AssetBundle::getAllLoadedAssetBundles() const noexcept
	{
		return assetBundles_;
	}
}