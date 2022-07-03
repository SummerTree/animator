#include <octoon/asset_importer_context.h>
#include <octoon/asset_database.h>
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
	AssetImporterContext::getMetadata() noexcept(false)
	{
		auto filepath = AssetDatabase::instance()->getAbsolutePath(this->getAssetPath());
		std::ifstream ifs(std::filesystem::path(filepath).concat(L".meta"));
		if (ifs)
		{
			auto metaData = nlohmann::json::parse(ifs);
			return metaData;
		}

		return nlohmann::json();
	}
}