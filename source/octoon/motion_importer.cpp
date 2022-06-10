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
		std::wstring u16_conv = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes((char*)std::string(filepath).data());

		if (std::filesystem::exists(u16_conv))
		{
			auto uuid = octoon::make_guid();
			auto rootPath = std::filesystem::path(assertPath_).append(uuid);
			auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directory(assertPath_);
			std::filesystem::create_directory(rootPath);
			std::filesystem::copy(u16_conv, motionPath);
			std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

			auto filename = std::filesystem::path(u16_conv).filename().u8string();

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = (char*)filename.substr(0, filename.find_last_of('.')).c_str();
			package["path"] = (char*)motionPath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			indexList_.push_back(uuid);

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