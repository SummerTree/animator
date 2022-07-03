#ifndef OCTOON_ASSET_IMPORTER_H_
#define OCTOON_ASSET_IMPORTER_H_

#include <octoon/runtime/object.h>
#include <octoon/runtime/singleton.h>
#include <octoon/runtime/json.h>
#include <filesystem>
#include <map>

namespace octoon
{
	class OCTOON_EXPORT AssetImporter : public Object
	{
		OctoonDeclareSubInterface(AssetImporter, Object)
	public:
		AssetImporter() noexcept;
		AssetImporter(const std::filesystem::path& path) noexcept;
		virtual ~AssetImporter() noexcept;

		void addRemap(const std::shared_ptr<const Object>& subAsset);
		const std::vector<std::weak_ptr<const Object>>& getExternalObjectMap() const;

		const std::filesystem::path& getAssetPath() const noexcept;

		virtual std::shared_ptr<Object> importer() noexcept(false) = 0;

		static std::shared_ptr<AssetImporter> getAtPath(const std::filesystem::path& path) noexcept;

	protected:
		nlohmann::json loadMetadataAtPath(const std::filesystem::path& path) noexcept(false);

	private:
		AssetImporter(const AssetImporter&) = delete;
		AssetImporter& operator=(const AssetImporter&) = delete;

	protected:
		std::filesystem::path assetPath_;
		std::vector<std::weak_ptr<const Object>> externalObjectMap_;

		static std::map<std::filesystem::path, std::shared_ptr<AssetImporter>> assets_;
	};
}

#endif