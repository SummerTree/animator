#include <octoon/motion_importer.h>
#include <octoon/runtime/uuid.h>
#include <octoon/vmd_loader.h>
#include <octoon/asset_database.h>
#include <fstream>
#include <filesystem>
#include <codecvt>

namespace octoon
{
	OctoonImplementSingleton(MotionImporter)

	MotionImporter::MotionImporter() noexcept
	{
	}

	MotionImporter::~MotionImporter() noexcept
	{
	}

	nlohmann::json
	MotionImporter::importPackage(std::string_view filepath) noexcept(false)
	{
		auto package = AssetDatabase::instance()->createAsset(filepath, assertPath_);
		if (package.is_object())
		{
			indexList_.push_back(package["uuid"].get<std::string>());
			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	MotionImporter::createPackage(const std::shared_ptr<octoon::Animation>& animation, std::string_view outputPath) noexcept
	{
		if (animation)
		{
			auto it = this->assetPackageCache_.find(animation);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_[animation];

			auto uuid = AssetDatabase::instance()->getAssetGuid(animation);

			nlohmann::json package = AssetDatabase::instance()->getPackage(animation);
			if (package.find("uuid") != package.end())
			{
				for (auto& index : indexList_)
				{
					if (index == uuid)
						return package;
				}
			}

			package = AssetDatabase::instance()->createAsset(*animation, outputPath);
			assetPackageCache_[animation] = package;

			return package;
		}

		return nlohmann::json();
	}
}