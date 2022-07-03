#include <octoon/asset_importer_context.h>
#include <octoon/asset_database.h>
#include <fstream>

namespace octoon
{
	AssetImporterContext::AssetImporterContext(const std::filesystem::path& path) noexcept
		: assetPath_(path)
	{
		auto filepath = AssetDatabase::instance()->getAbsolutePath(assetPath_);
		std::ifstream ifs(std::filesystem::path(filepath).concat(L".meta"));
		if (ifs)
			this->metaData_ = nlohmann::json::parse(ifs);
	}

	AssetImporterContext::~AssetImporterContext() noexcept
	{
	}

	void
	AssetImporterContext::setMainObject(const std::shared_ptr<Object>& object) noexcept
	{
		this->mainObject = object;
	}

	const std::shared_ptr<Object>&
	AssetImporterContext::getMainObject() const noexcept
	{
		return this->mainObject;
	}

	void
	AssetImporterContext::addObjectToAsset(const std::shared_ptr<const Object>& subAsset)
	{
		this->subAssets_.push_back(subAsset);
	}

	const std::vector<std::shared_ptr<const Object>>&
	AssetImporterContext::getSubAssets() const
	{
		return subAssets_;
	}

	const std::filesystem::path&
	AssetImporterContext::getAssetPath() const noexcept
	{
		return assetPath_;
	}

	nlohmann::json
	AssetImporterContext::getMetadata() const noexcept(false)
	{
		return metaData_;
	}
}