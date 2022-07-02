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

		void addRemap(const std::shared_ptr<const Object>& asset, const std::shared_ptr<const Object>& subAsset);

	public:
		bool isSubAsset(const std::shared_ptr<const Object>& asset) const noexcept;

		void setAssetPath(const std::shared_ptr<const Object>& asset, const std::filesystem::path& path) noexcept;

		std::shared_ptr<const AssetImporter> getAtPath(const std::filesystem::path& path) const noexcept;

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

	private:
		AssetImporter(const AssetImporter&) = delete;
		AssetImporter& operator=(const AssetImporter&) = delete;

	protected:
		std::filesystem::path assetPath_;
		std::vector<std::weak_ptr<const Object>> externalObjectMap_;

	private:
		static std::map<std::weak_ptr<const Object>, std::filesystem::path, std::owner_less<std::weak_ptr<const Object>>> assetToPath_;
		static std::map<std::weak_ptr<const Object>, std::filesystem::path, std::owner_less<std::weak_ptr<const Object>>> subAssetToPath_;

		static std::map<std::filesystem::path, std::shared_ptr<const AssetImporter>> assets_;
	};
}

#endif