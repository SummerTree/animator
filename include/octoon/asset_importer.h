#ifndef UNREAL_ASSET_DATABASE_H_
#define UNREAL_ASSET_DATABASE_H_

#include <octoon/game_object.h>
#include <octoon/runtime/singleton.h>

namespace octoon
{
	class OCTOON_EXPORT AssetImporter final
	{
	public:
		AssetImporter() noexcept;
		virtual ~AssetImporter() noexcept;

		virtual void open(std::string indexPath) noexcept(false);
		virtual void close() noexcept;

		virtual bool hasPackage(std::string_view uuid) const noexcept;
		virtual bool hasPackage(const std::shared_ptr<RttiObject>& asset) const noexcept;

		virtual nlohmann::json getPackage(std::string_view uuid) noexcept;
		virtual nlohmann::json getPackage(const std::shared_ptr<RttiObject>& asset) const noexcept(false);

		virtual void removeAsset(std::string_view uuid, std::string_view outputPath = "") noexcept(false);

		virtual nlohmann::json& getIndexList() noexcept;
		virtual const nlohmann::json& getIndexList() const noexcept;

		virtual std::string getPackagePath(const std::shared_ptr<RttiObject>& asset) const noexcept;
		virtual std::string getPackageGuid(const std::shared_ptr<RttiObject>& asset) const noexcept;

		void addIndex(const std::string& uuid)
		{
			indexList_.push_back(uuid);
		}

		const std::string& getAssertPath() const {
			return assertPath_;
		}

		virtual void saveAssets() noexcept(false);

		virtual void clearCache() noexcept;

	private:
		nlohmann::json getPackageIndices(std::string_view indexPath) noexcept(false);

	protected:
		std::string assertPath_;

		nlohmann::json indexList_;

		std::map<std::string, nlohmann::json> packageList_;

		std::map<std::string, std::shared_ptr<RttiObject>> assetCache_;
		std::map<std::weak_ptr<RttiObject>, nlohmann::json, std::owner_less<std::weak_ptr<RttiObject>>> assetList_;
		std::map<std::weak_ptr<RttiObject>, std::string, std::owner_less<std::weak_ptr<RttiObject>>> assetPathList_;
	};
}

#endif