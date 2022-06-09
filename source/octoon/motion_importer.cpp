#include <octoon/motion_importer.h>
#include <octoon/runtime/uuid.h>
#include <octoon/vmd_loader.h>
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
	MotionImporter::createPackage(std::string_view filepath) noexcept(false)
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

			auto uuid = octoon::make_guid();

			nlohmann::json package = this->getPackage(animation);
			if (package.find("uuid") != package.end())
			{
				uuid = package["uuid"].get<nlohmann::json::string_t>();
				for (auto& index : indexList_)
				{
					if (index == uuid)
						return package;
				}
			}

			auto rootPath = std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid);
			auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directories(rootPath);

			octoon::VMDLoader::save(motionPath.string(), *animation);
			std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = uuid + ".vmd";
			package["path"] = (char*)motionPath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->assetPackageCache_[animation] = package;
			this->packageList_[std::string(uuid)] = package;

			return package;
		}

		return nlohmann::json();
	}

	std::shared_ptr<octoon::Animation>
	MotionImporter::loadPackage(const nlohmann::json& package) noexcept(false)
	{
		if (package["path"].is_string())
		{
			auto uuid = package["uuid"].get<nlohmann::json::string_t>();
			auto it = this->assetCache_.find(uuid);
			if (it != this->assetCache_.end())
				return this->assetCache_[uuid]->downcast_pointer<octoon::Animation>();

			auto path = package["path"].get<nlohmann::json::string_t>();
			auto motion = octoon::VMDLoader::load(path.c_str());
			if (motion)
			{
				packageList_[uuid] = package;
				assetCache_[uuid] = motion;
				assetList_[motion] = package;
				return motion;
			}
		}

		return nullptr;
	}
}