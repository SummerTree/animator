#ifndef UNREAL_ASSET_DATABASE_H_
#define UNREAL_ASSET_DATABASE_H_

#include <octoon/game_object.h>
#include <octoon/runtime/singleton.h>
#include <filesystem>

namespace unreal
{
	class AssetImporter final
	{
	public:
		AssetImporter() noexcept;
		virtual ~AssetImporter() noexcept;

		void open(const std::filesystem::path& indexPath) noexcept(false);
		void close() noexcept;

		void addPackage(const nlohmann::json& uuid);
		bool hasPackage(std::string_view uuid) const noexcept;

		nlohmann::json getPackage(std::string_view uuid) noexcept;

		void removeAsset(std::string_view uuid) noexcept(false);

		nlohmann::json& getIndexList() noexcept;
		const nlohmann::json& getIndexList() const noexcept;

		const std::filesystem::path& getAssertPath() const;

		virtual void saveAssets() const noexcept(false);

	private:
		nlohmann::json getPackageIndices(const std::filesystem::path& path) noexcept(false);

	private:
		std::filesystem::path assertPath_;

		nlohmann::json indexList_;

		std::map<std::string, nlohmann::json> packageList_;
		std::map<std::string, std::filesystem::path> uuidToPathList_;
	};
}

#endif