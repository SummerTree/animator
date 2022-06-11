#ifndef OCTOON_ASSET_BUNDLE_H_
#define OCTOON_ASSET_BUNDLE_H_

#include <octoon/game_object.h>
#include <octoon/texture/texture.h>
#include <octoon/material/material.h>
#include <octoon/animation/animation.h>
#include <octoon/runtime/singleton.h>
#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT AssetBundle final
	{
		OctoonDeclareSingleton(AssetBundle)
	public:
		AssetBundle() noexcept;
		~AssetBundle() noexcept;

		void open(std::string assetPath) noexcept(false);
		void close() noexcept;

		void unload() noexcept;
		void saveAssets() noexcept(false);

		nlohmann::json importAsset(std::string_view path, bool generateMipmap = false) noexcept(false);

		nlohmann::json createAsset(const std::shared_ptr<Texture>& texture) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<Animation>& animation) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<Material>& material) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<GameObject>& gameObject) noexcept(false);

		nlohmann::json getPackage(std::string_view uuid) noexcept;
		nlohmann::json getPackage(const std::shared_ptr<RttiObject>& asset) const noexcept(false);

		nlohmann::json& getModelList() const noexcept;
		nlohmann::json& getMotionList() const noexcept;
		nlohmann::json& getTextureList() const noexcept;
		nlohmann::json& getMaterialList() const noexcept;

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

		std::shared_ptr<AssetBundle> loadFromFile(std::string_view path) noexcept(false);
		std::vector<std::shared_ptr<AssetBundle>> getAllLoadedAssetBundles() const noexcept;

	private:
		AssetBundle(const AssetBundle&) = delete;
		AssetBundle& operator=(const AssetBundle&) = delete;

	private:
		std::string assetPath_;

		std::unique_ptr<AssetImporter> modelAsset_;
		std::unique_ptr<AssetImporter> motionAsset_;
		std::unique_ptr<AssetImporter> textureAsset_;
		std::unique_ptr<AssetImporter> materialAsset_;

		std::map<std::string, std::shared_ptr<RttiObject>> assetCache_;
		std::map<std::weak_ptr<RttiObject>, nlohmann::json, std::owner_less<std::weak_ptr<RttiObject>>> assetPackageCache_;

		std::vector<std::shared_ptr<AssetBundle>> assetBundles_;
	};
}

#endif