#ifndef OCTOON_ASSET_IMPORTER_H_
#define OCTOON_ASSET_IMPORTER_H_

#include <octoon/texture/texture.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/animation/animation.h>
#include <octoon/runtime/singleton.h>
#include <octoon/runtime/json.h>
#include <filesystem>
#include <map>

namespace octoon
{
	class OCTOON_EXPORT AssetImporter : public Object
	{
		OctoonDeclareSubClass(AssetImporter, Object)
	public:
		AssetImporter() noexcept;
		virtual ~AssetImporter() noexcept;

		void addRemap(const std::shared_ptr<const Object>& subAsset);
		const std::vector<std::weak_ptr<const Object>>& getExternalObjectMap() const;

		static std::shared_ptr<AssetImporter> getAtPath(const std::filesystem::path& path) noexcept;
		static std::shared_ptr<Object> loadAssetAtPath(const std::filesystem::path& path) noexcept(false);

		template<typename T>
		static std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
		{
			auto asset = loadAssetAtPath(path);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

	protected:
		nlohmann::json loadMetadataAtPath(const std::filesystem::path& path) noexcept(false);

	private:
		AssetImporter(const AssetImporter&) = delete;
		AssetImporter& operator=(const AssetImporter&) = delete;

	protected:
		std::filesystem::path assetPath_;
		std::vector<std::weak_ptr<const Object>> externalObjectMap_;

		static std::map<std::string, std::filesystem::path> uniques_;
		static std::map<std::filesystem::path, std::string> paths_;
		static std::map<std::filesystem::path, std::shared_ptr<AssetImporter>> assets_;
	};
}

#endif