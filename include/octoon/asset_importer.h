#ifndef OCTOON_ASSET_IMPORTER_H_
#define OCTOON_ASSET_IMPORTER_H_

#include <octoon/pmx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/animation/animation.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT AssetImporter final
	{
		OctoonDeclareSingleton(AssetImporter)
	public:
		AssetImporter() noexcept;
		virtual ~AssetImporter() noexcept;

		bool isSubAsset(const std::shared_ptr<const Object>& asset) const noexcept;
		void addObjectToAsset(const std::shared_ptr<const Object>& asset, const std::filesystem::path& path);

		void setAssetPath(const std::shared_ptr<const Object>& asset, const std::filesystem::path& path) noexcept;

		std::filesystem::path getAssetPath(const std::shared_ptr<const Object>& asset) const noexcept;
		std::filesystem::path getAssetExtension(const std::shared_ptr<const Object>& asset, std::string_view defaultExtension = "") const noexcept;

		std::shared_ptr<Object> loadAssetAtPath(const std::filesystem::path& path) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
		{
			auto asset = loadAssetAtPath(path);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

		void unload() noexcept;

	private:
		AssetImporter(const AssetImporter&) = delete;
		AssetImporter& operator=(const AssetImporter&) = delete;

	private:
		std::vector<std::shared_ptr<const Object>> caches_;

		std::map<std::weak_ptr<const Object>, std::filesystem::path, std::owner_less<std::weak_ptr<const Object>>> assetToPath_;
		std::map<std::weak_ptr<const Object>, std::filesystem::path, std::owner_less<std::weak_ptr<const Object>>> subAssetToPath_;
	};
}

#endif