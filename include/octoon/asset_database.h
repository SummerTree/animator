#ifndef OCTOON_ASSET_DATABASE_H_
#define OCTOON_ASSET_DATABASE_H_

#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT AssetDatabase final : public octoon::AssetImporter
	{
		OctoonDeclareSingleton(AssetDatabase)
	public:
		AssetDatabase() noexcept;
		virtual ~AssetDatabase() noexcept;

		std::shared_ptr<octoon::RttiObject> loadAssetAtPath(std::string_view path) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(std::string_view path) noexcept(false)
		{
			auto asset = loadAssetAtPath(path);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}
	};
}

#endif