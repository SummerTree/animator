#ifndef UNREAL_ASSET_DATABASE_H_
#define UNREAL_ASSET_DATABASE_H_

#include <octoon/game_object.h>
#include <octoon/runtime/singleton.h>

namespace octoon
{
	class OCTOON_EXPORT AssetImporter
	{
	public:
		AssetImporter() noexcept;
		~AssetImporter() noexcept;

		virtual void open(std::string indexPath) noexcept(false);
		virtual void close() noexcept;

		virtual nlohmann::json getPackage(std::string_view uuid, std::string_view outputPath = "") noexcept;
		virtual nlohmann::json getPackage(const std::shared_ptr<octoon::RttiObject>& asset) const noexcept(false);

		virtual void removePackage(std::string_view uuid, std::string_view outputPath = "") noexcept(false);

		virtual nlohmann::json& getIndexList() noexcept;

		virtual std::string getPackagePath(const std::shared_ptr<octoon::RttiObject>& asset) const noexcept;

		virtual void saveAssets() noexcept(false);

		virtual void clearCache() noexcept;

	private:
		void initPackageIndices() noexcept(false);

	protected:
		std::string assertPath_;

		nlohmann::json indexList_;

		std::map<std::string, nlohmann::json> packageList_;

		std::map<std::string, std::shared_ptr<octoon::RttiObject>> assetCache_;
		std::map<std::weak_ptr<octoon::RttiObject>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetPackageCache_;
		std::map<std::weak_ptr<octoon::RttiObject>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetList_;
		std::map<std::weak_ptr<octoon::RttiObject>, std::string, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetPathList_;
	};
}

#endif