#ifndef OCTOON_ASSET_MANAGER_H_
#define OCTOON_ASSET_MANAGER_H_

#include <octoon/runtime/guid.h>
#include <octoon/runtime/singleton.h>
#include <octoon/runtime/object.h>
#include <map>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT AssetManager final
	{
		OctoonDeclareSingleton(AssetManager)
	public:
		AssetManager() noexcept;
		~AssetManager() noexcept;

		void setAssetPath(const std::shared_ptr<const Object>& object, std::filesystem::path) noexcept;
		std::filesystem::path getAssetPath(const std::shared_ptr<const Object>& object) const noexcept;

	private:
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

	private:
		std::map<std::filesystem::path, std::string> paths_;
		std::map<std::string, std::filesystem::path> uniques_;
		std::map<std::weak_ptr<const Object>, std::filesystem::path, std::owner_less<std::weak_ptr<const Object>>> assetToPath_;
	};
}

#endif