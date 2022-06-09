#ifndef OCTOON_ASSET_DATABASE_H_
#define OCTOON_ASSET_DATABASE_H_

#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT AssetDatabase final
	{
		OctoonDeclareSingleton(AssetDatabase)
	public:
		AssetDatabase() noexcept;
		virtual ~AssetDatabase() noexcept;

		std::string getAssetPath(const std::shared_ptr<RttiObject>& asset) const noexcept;
		std::string getAssetGuid(const std::shared_ptr<RttiObject>& asset) const noexcept;

		std::shared_ptr<RttiObject> loadAssetAtPath(std::string_view path) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(std::string_view path) noexcept(false)
		{
			auto asset = loadAssetAtPath(path);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

	private:
		std::map<std::weak_ptr<octoon::RttiObject>, std::string, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetPathList_;
		std::map<std::weak_ptr<octoon::RttiObject>, std::string, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetGuidList_;
	};
}

#endif