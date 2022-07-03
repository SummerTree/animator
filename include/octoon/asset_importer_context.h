#ifndef OCTOON_ASSET_IMPORTER_CONTEXT_H_
#define OCTOON_ASSET_IMPORTER_CONTEXT_H_

#include <octoon/runtime/object.h>
#include <octoon/runtime/json.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT AssetImporterContext final
	{
	public:
		AssetImporterContext(const std::filesystem::path& path) noexcept;
		virtual ~AssetImporterContext() noexcept;

		void addRemap(const std::shared_ptr<const Object>& subAsset);
		const std::vector<std::weak_ptr<const Object>>& getExternalObjects() const;

		const std::filesystem::path& getAssetPath() const noexcept;

		nlohmann::json loadMetadataAtPath(const std::filesystem::path& path) noexcept(false);

	private:
		AssetImporterContext(const AssetImporterContext&) = delete;
		AssetImporterContext& operator=(const AssetImporterContext&) = delete;

	private:
		std::filesystem::path assetPath_;
		std::vector<std::weak_ptr<const Object>> externalObjectMap_;
	};
}

#endif