#ifndef OCTOON_ASSET_BUNDLE_H_
#define OCTOON_ASSET_BUNDLE_H_

#include <octoon/game_object.h>
#include <octoon/texture/texture.h>
#include <octoon/material/material.h>
#include <octoon/animation/animation.h>
#include <octoon/pmx_loader.h>
#include <octoon/runtime/uuid.h>
#include <octoon/runtime/singleton.h>
#include <octoon/asset_importer.h>
#include <octoon/asset_database.h>
#include <set>

namespace octoon
{
	class OCTOON_EXPORT AssetBundle final
	{
		OctoonDeclareSingleton(AssetBundle)
	public:
		AssetBundle() noexcept;
		~AssetBundle() noexcept;

		void open(const std::filesystem::path& assetPath) noexcept(false);
		void close() noexcept;

		nlohmann::json importAsset(const std::filesystem::path& path) noexcept(false);
		nlohmann::json importAsset(const std::shared_ptr<Texture>& path) noexcept(false);
		nlohmann::json importAsset(const std::shared_ptr<Animation>& animation) noexcept(false);
		nlohmann::json importAsset(const std::shared_ptr<Material>& material) noexcept(false);
		nlohmann::json importAsset(const std::shared_ptr<GameObject>& gameObject) noexcept(false);

		nlohmann::json createAsset(const std::shared_ptr<Texture>& texture) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<Animation>& animation) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<Material>& material) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<GameObject>& gameObject) noexcept(false);

		bool hasPackage(std::string_view uuid) noexcept;

		nlohmann::json getPackage(std::string_view uuid) noexcept;
		nlohmann::json getPackage(const std::shared_ptr<RttiObject>& asset) noexcept;

		nlohmann::json& getModelList() const noexcept;
		nlohmann::json& getMotionList() const noexcept;
		nlohmann::json& getTextureList() const noexcept;
		nlohmann::json& getHDRiList() const noexcept;
		nlohmann::json& getMaterialList() const noexcept;
		nlohmann::json& getPrefabList() const noexcept;

		void unload() noexcept;
		void saveAssets() noexcept(false);
		void removeAsset(std::string_view uuid) noexcept(false);

		std::shared_ptr<RttiObject> loadAsset(std::string_view uuid, const Rtti& rtti) noexcept(false);
		
		template<typename T, typename = std::enable_if_t<std::is_base_of<RttiObject, T>::value>>
		std::shared_ptr<T> loadAsset(std::string_view uuid) noexcept(false)
		{
			auto asset = loadAsset(uuid, *T::getRtti());
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

		std::shared_ptr<AssetBundle> loadFromFile(const std::filesystem::path& path) noexcept(false);
		std::vector<std::shared_ptr<AssetBundle>> getAllLoadedAssetBundles() const noexcept;

	private:
		nlohmann::json importAsset(const std::shared_ptr<PMX>& pmx) noexcept(false);

		std::shared_ptr<RttiObject> loadAssetAtPackage(const nlohmann::json& package, const Rtti& type) noexcept(false);

		template<typename T, typename = std::enable_if_t<std::is_base_of<RttiObject, T>::value>>
		std::shared_ptr<T> loadAssetAtPackage(const nlohmann::json& package) noexcept(false)
		{
			auto asset = loadAssetAtPackage(package, *T::getRtti());
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

	private:
		AssetBundle(const AssetBundle&) = delete;
		AssetBundle& operator=(const AssetBundle&) = delete;

	private:
		std::filesystem::path assetPath_;
		std::unique_ptr<AssetDatabase> assetDatabase_;

		std::unique_ptr<AssetImporter> modelAsset_;
		std::unique_ptr<AssetImporter> motionAsset_;
		std::unique_ptr<AssetImporter> textureAsset_;
		std::unique_ptr<AssetImporter> materialAsset_;
		std::unique_ptr<AssetImporter> hdriAsset_;
		std::unique_ptr<AssetImporter> prefabAsset_;

		std::map<std::string, std::weak_ptr<RttiObject>> assetCache_;

		std::vector<std::shared_ptr<AssetBundle>> assetBundles_;
	};
}

#endif