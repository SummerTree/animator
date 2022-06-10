#include <octoon/asset_bundle.h>
#include <octoon/texture/texture.h>
#include <octoon/asset_database.h>
#include <octoon/runtime/uuid.h>
#include <fstream>
#include <filesystem>
#include <set>

namespace octoon
{
	OctoonImplementSingleton(AssetBundle)

	AssetBundle::AssetBundle() noexcept
	{
	}

	AssetBundle::~AssetBundle() noexcept
	{
	}

	nlohmann::json
	AssetBundle::importPackage(std::string_view filepath, bool generateMipmap) noexcept(false)
	{
		octoon::Texture texture;

		if (texture.load(std::string(filepath)))
		{
			texture.setName(filepath);
			auto package = AssetDatabase::instance()->createAsset(texture, assertPath_);
			indexList_.push_back(package["uuid"].get<std::string>());
			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetBundle::createPackage(const std::shared_ptr<octoon::Texture>& texture, std::string_view outputPath) noexcept(false)
	{
		if (texture)
		{
			auto it = this->assetPackageCache_.find(texture);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_[texture];

			auto uuid = AssetDatabase::instance()->getAssetGuid(texture);

			nlohmann::json package = AssetDatabase::instance()->getPackage(texture);
			if (package.find("uuid") != package.end())
			{
				for (auto& index : indexList_)
				{
					if (index == uuid)
						return package;
				}
			}

			package = AssetDatabase::instance()->createAsset(*texture, outputPath);
			assetPackageCache_[texture] = package;

			return package;
		}

		return nlohmann::json();
	}
}