#ifndef UNREAL_ASSET_DATABASE_H_
#define UNREAL_ASSET_DATABASE_H_

#include <octoon/game_object.h>
#include <octoon/runtime/singleton.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT AssetImporter final
	{
	public:
		AssetImporter() noexcept;
		virtual ~AssetImporter() noexcept;

		virtual void open(const std::filesystem::path& indexPath) noexcept(false);
		virtual void close() noexcept;

		virtual bool hasPackage(std::string_view uuid) const noexcept;
		virtual bool hasPackage(const std::shared_ptr<RttiObject>& asset) const noexcept;

		virtual nlohmann::json getPackage(std::string_view uuid) noexcept;
		virtual nlohmann::json getPackage(const std::shared_ptr<RttiObject>& asset) const noexcept(false);

		virtual void removeAsset(std::string_view uuid) noexcept(false);

		virtual nlohmann::json& getIndexList() noexcept;
		virtual const nlohmann::json& getIndexList() const noexcept;

		virtual std::string getPackagePath(const std::shared_ptr<RttiObject>& asset) const noexcept;
		virtual std::string getPackageGuid(const std::shared_ptr<RttiObject>& asset) const noexcept;

		void addIndex(const std::string& uuid)
		{
			indexList_.push_back(uuid);
		}

		const std::filesystem::path& getAssertPath() const {
			return assertPath_;
		}

		virtual void saveAssets() const noexcept(false);
		virtual void clearCache() noexcept;

	private:
		nlohmann::json getPackageIndices(const std::filesystem::path& path) noexcept(false);

	private:
		std::filesystem::path assertPath_;

		nlohmann::json indexList_;

		std::map<std::string, nlohmann::json> packageList_;

		std::map<std::weak_ptr<RttiObject>, nlohmann::json, std::owner_less<std::weak_ptr<RttiObject>>> assetList_;
		std::map<std::weak_ptr<RttiObject>, std::string, std::owner_less<std::weak_ptr<RttiObject>>> assetPathList_;
	};
}

#endif