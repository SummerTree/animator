#ifndef OCTOON_ASSET_IMPORTER_H_
#define OCTOON_ASSET_IMPORTER_H_

#include <octoon/texture/texture.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/animation/animation.h>
#include <octoon/runtime/singleton.h>
#include <filesystem>
#include <map>

namespace octoon
{
	class OCTOON_EXPORT AssetImporter : public Object
	{
		OctoonDeclareSubClass(AssetImporter, Object)
		OctoonDeclareSingleton(AssetImporter)
	public:
		AssetImporter() noexcept;
		virtual ~AssetImporter() noexcept;

		void addRemap(const std::shared_ptr<const Object>& subAsset);

		const std::vector<std::weak_ptr<const Object>>& getExternalObjectMap() const;

		std::shared_ptr<const AssetImporter> getAtPath(const std::filesystem::path& path) const noexcept;

	private:
		friend class Package;
		std::shared_ptr<Object> loadAssetAtPath(const std::filesystem::path& path) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
		{
			auto asset = loadAssetAtPath(path);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

	private:
		AssetImporter(const AssetImporter&) = delete;
		AssetImporter& operator=(const AssetImporter&) = delete;

	protected:
		std::filesystem::path assetPath_;
		std::vector<std::weak_ptr<const Object>> externalObjectMap_;

		static std::map<std::filesystem::path, std::shared_ptr<const AssetImporter>> assets_;
	};
}

#endif