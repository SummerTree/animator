#ifndef OCTOON_ASSET_DATABASE_H_
#define OCTOON_ASSET_DATABASE_H_

#include <octoon/asset_importer.h>
#include <octoon/texture/texture.h>
#include <octoon/animation/animation.h>

namespace octoon
{
	class OCTOON_EXPORT AssetDatabase final
	{
		OctoonDeclareSingleton(AssetDatabase)
	public:
		AssetDatabase() noexcept;
		virtual ~AssetDatabase() noexcept;

		nlohmann::json createAsset(const octoon::Texture& texture, std::string_view outputPath) noexcept(false);
		nlohmann::json createAsset(const octoon::Animation& animation, std::string_view outputPath) noexcept(false);

		std::string getAssetPath(const std::shared_ptr<RttiObject>& asset) const noexcept;

		std::string getAssetGuid(const std::shared_ptr<RttiObject>& asset) noexcept;
		std::string getAssetGuid(const std::shared_ptr<RttiObject>& asset) const noexcept;

		nlohmann::json getPackage(std::string_view uuid, std::string_view outputPath = "") noexcept;
		nlohmann::json getPackage(const std::shared_ptr<octoon::RttiObject>& asset) const noexcept(false);

		std::shared_ptr<RttiObject> loadAssetAtPath(std::string_view path) noexcept(false);
		std::shared_ptr<RttiObject> loadAssetAtPackage(const nlohmann::json& package, const Rtti& type) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(std::string_view path) noexcept(false)
		{
			auto asset = loadAssetAtPath(path);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of<RttiObject, T>::value>>
		std::shared_ptr<T> loadAssetAtPackage(const nlohmann::json& package) noexcept(false)
		{
			auto asset = loadAssetAtPackage(package, *T::getRtti());
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

	private:
		std::map<std::string, nlohmann::json> packageList_;

		std::map<std::string, std::shared_ptr<octoon::RttiObject>> assetCache_;
		std::map<std::weak_ptr<octoon::RttiObject>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetList_;
		std::map<std::weak_ptr<octoon::RttiObject>, std::string, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetPathList_;
		std::map<std::weak_ptr<octoon::RttiObject>, std::string, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetGuidList_;
	};
}

#endif