#include <octoon/asset_manager.h>

namespace octoon
{
	OctoonImplementSingleton(AssetManager)

	AssetManager::AssetManager() noexcept
	{
	}

	AssetManager::~AssetManager() noexcept
	{
	}

	void
	AssetManager::setAssetPath(const std::shared_ptr<const Object>& object, std::filesystem::path path) noexcept
	{
		assetToPath_[object] = path;
	}

	std::filesystem::path
	AssetManager::getAssetPath(const std::shared_ptr<const Object>& object) const noexcept
	{
		auto asset = assetToPath_.find(object);
		if (asset != assetToPath_.end())
			return asset->second;

		return std::filesystem::path();
	}
}