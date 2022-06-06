#include "motion_importer.h"
#include "unreal_behaviour.h"
#include <octoon/runtime/uuid.h>
#include <fstream>
#include <filesystem>
#include <codecvt>

namespace unreal
{
	OctoonImplementSingleton(MotionImporter)

	MotionImporter::MotionImporter() noexcept
	{
	}

	MotionImporter::~MotionImporter() noexcept
	{
	}

	void
	MotionImporter::open(std::u8string_view indexPath) noexcept(false)
	{
		if (std::filesystem::exists(indexPath))
		{
			this->assertPath_ = indexPath;
			this->initPackageIndices();
		}
	}

	void
	MotionImporter::close() noexcept
	{
	}

	std::shared_ptr<octoon::Animation<float>>
	MotionImporter::importMotion(std::u8string_view path) noexcept(false)
	{
		auto ext = path.substr(path.find_last_of('.'));
		if (ext == u8".vmd")
		{
			auto motion = octoon::VMDLoader::loadMotion(path);
			if (motion)
			{
				motionPathList_[motion] = path;
				return motion;
			}
		}

		return nullptr;
	}

	std::shared_ptr<octoon::Animation<float>>
	MotionImporter::importCameraMotion(std::u8string_view path) noexcept(false)
	{
		auto ext = path.substr(path.find_last_of('.'));
		if (ext == u8".vmd")
		{
			auto motion = octoon::VMDLoader::loadCameraMotion(path);
			if (motion)
			{
				motionPathList_[motion] = path;
				return motion;
			}
		}

		return nullptr;
	}

	nlohmann::json
	MotionImporter::createPackage(std::u8string_view filepath, bool blockSignals) noexcept(false)
	{
		std::wstring u16_conv = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes((char*)std::u8string(filepath).data());

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

			nlohmann::json item;
			item["uuid"] = uuid;
			item["name"] = (char*)filename.substr(0, filename.find_last_of('.')).c_str();
			item["path"] = (char*)motionPath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = item.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			indexList_.getValue().push_back(uuid);

			if (!blockSignals)
				indexList_.submit();

			return item;
		}

		return nlohmann::json();
	}

	nlohmann::json
	MotionImporter::createPackage(const std::shared_ptr<octoon::Animation<float>>& animation, std::u8string_view outputPath) noexcept
	{
		if (animation)
		{
			auto it = this->motionList_.find(animation);
			if (it != this->motionList_.end())
				return this->motionList_[animation];

			auto uuid = octoon::make_guid();
			auto rootPath = std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid);
			auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directories(rootPath);

			octoon::VMDLoader::saveMotion(motionPath.string(), *animation);
			std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["name"] = uuid + ".vmd";
			package["path"] = (char*)motionPath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->motionList_[animation] = package;
			this->packageList_[std::string(uuid)] = package;

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	MotionImporter::getPackage(std::string_view uuid) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(assertPath_).append(uuid).append("package.json"));
			if (ifs)
			{
				auto package = nlohmann::json::parse(ifs);
				this->packageList_[std::string(uuid)] = package;
				return package;
			}
			else
			{
				return nlohmann::json();
			}			
		}

		return this->packageList_[std::string(uuid)];
	}

	nlohmann::json
	MotionImporter::getPackage(const std::shared_ptr<octoon::Animation<float>>& animation) const noexcept(false)
	{
		if (animation)
		{
			auto it = motionList_.find(animation);
			if (it != motionList_.end())
				return (*it).second;
		}

		return nlohmann::json();
	}

	void
	MotionImporter::removePackage(std::string_view uuid) noexcept(false)
	{
		auto& indexList = indexList_.getValue();

		for (auto index = indexList.begin(); index != indexList.end(); ++index)
		{
			if (uuid == (*index).get<nlohmann::json::string_t>())
			{
				auto packagePath = std::filesystem::path(assertPath_).append(uuid);

				for (auto& it : std::filesystem::recursive_directory_iterator(packagePath))
					std::filesystem::permissions(it, std::filesystem::perms::owner_write);

				std::filesystem::remove_all(packagePath);

				auto package = this->packageList_.find(std::string(uuid));
				if (package != this->packageList_.end())
					this->packageList_.erase(package);

				indexList.erase(index);
			}
		}
	}

	std::shared_ptr<octoon::Animation<float>>
	MotionImporter::loadPackage(const nlohmann::json& package) noexcept(false)
	{
		if (package["path"].is_string())
		{
			auto path = package["path"].get<nlohmann::json::string_t>();
			auto motion = octoon::VMDLoader::loadMotion((char8_t*)path.c_str());
			if (motion)
			{
				auto uuid = package["uuid"].get<nlohmann::json::string_t>();
				auto it = this->packageList_.find(uuid);
				if (it == this->packageList_.end())
					this->packageList_[uuid] = package;

				motionList_[motion] = package;
				return motion;
			}
		}

		return nullptr;
	}

	MutableLiveData<nlohmann::json>&
	MotionImporter::getIndexList() noexcept
	{
		return indexList_;
	}

	nlohmann::json
	MotionImporter::createMetadata(const std::shared_ptr<octoon::Animation<float>>& animation) const noexcept
	{
		auto it = motionList_.find(animation);
		if (it != motionList_.end())
		{
			auto& package = (*it).second;

			nlohmann::json json;
			json["uuid"] = package["uuid"].get<nlohmann::json::string_t>();

			return json;
		}
		auto path = motionPathList_.find(animation);
		if (path != motionPathList_.end())
		{
			nlohmann::json json;
			json["path"] = (char*)(*path).second.c_str();

			return json;
		}

		return nlohmann::json();
	}

	void
	MotionImporter::save() noexcept(false)
	{
		try
		{
			if (!std::filesystem::exists(assertPath_))
				std::filesystem::create_directory(assertPath_);

			std::ofstream ifs(assertPath_ + u8"/index.json", std::ios_base::binary);
			if (ifs)
			{
				auto data = indexList_.getValue().dump();
				ifs.write(data.c_str(), data.size());
			}
		}
		catch (...)
		{
		}
	}

	void
	MotionImporter::initPackageIndices() noexcept(false)
	{
		std::ifstream indexStream(assertPath_ + u8"/index.json");
		if (indexStream)
			indexList_ = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : indexList_.getValue())
		{		 
			if (!std::filesystem::exists(std::filesystem::path(assertPath_).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(assertPath_))
		{
			if (std::filesystem::is_directory(it))
			{
				auto filepath = it.path();
				auto filename = filepath.filename();

				auto index = indexSet.find(filename.string());
				if (index == indexSet.end())
				{
					if (std::filesystem::exists(filepath.append("package.json")))
					{
						needUpdateIndexFile = true;
						indexSet.insert(filename.string());
					}
				}
			}
		}

		if (needUpdateIndexFile)
		{
			nlohmann::json json;
			for (auto& it : indexSet)
				json += it;

			indexList_ = json;
			this->save();
		}
	}
}