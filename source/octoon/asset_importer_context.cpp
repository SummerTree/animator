#include <octoon/asset_importer_context.h>
#include <fstream>

namespace octoon
{
	AssetImporterContext::AssetImporterContext(const std::filesystem::path& path) noexcept
		: assetPath_(path)
	{
	}

	AssetImporterContext::~AssetImporterContext() noexcept
	{
	}

	void
	AssetImporterContext::addRemap(const std::shared_ptr<const Object>& subAsset)
	{
		this->externalObjectMap_.push_back(subAsset);
	}

	const std::vector<std::weak_ptr<const Object>>&
	AssetImporterContext::getExternalObjects() const
	{
		return externalObjectMap_;
	}

	const std::filesystem::path&
	AssetImporterContext::getAssetPath() const noexcept
	{
		return assetPath_;
	}

	nlohmann::json
	AssetImporterContext::loadMetadataAtPath(const std::filesystem::path& path) noexcept(false)
	{
		std::ifstream ifs(std::filesystem::path(path).concat(L".meta"));
		if (ifs)
		{
			auto metaData = nlohmann::json::parse(ifs);
			return metaData;
		}

		return nlohmann::json();
	}
}