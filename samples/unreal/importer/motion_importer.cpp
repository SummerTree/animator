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
	MotionImporter::open(std::string indexPath) noexcept(false)
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
	MotionImporter::importMotion(std::string_view path) noexcept(false)
	{
		auto ext = path.substr(path.find_last_of("."));
		if (ext == ".vmd")
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
	MotionImporter::importCameraMotion(std::string_view path) noexcept(false)
	{
		auto ext = path.substr(path.find_last_of("."));
		if (ext == ".vmd")
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
	MotionImporter::importPackage(std::string_view filepath) noexcept(false)
	{
		std::wstring u16_conv = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(std::string(filepath));

		if (std::filesystem::exists(u16_conv))
		{
			auto uuid = octoon::make_guid();
			auto rootPath = std::filesystem::path(assertPath_).append(uuid);
			auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directory(assertPath_);
			std::filesystem::create_directory(rootPath);
			std::filesystem::copy(u16_conv, motionPath);

			auto filename = std::filesystem::path(u16_conv).filename().u8string();

			nlohmann::json item;
			item["uuid"] = uuid;
			item["name"] = filename.substr(0, filename.find_last_of('.'));
			item["path"] = motionPath.u8string();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = item.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			indexList_.getValue().push_back(uuid);
			indexList_.submit();

			return item;
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

	bool
	MotionImporter::removePackage(std::string_view uuid) noexcept
	{
		try
		{
			auto& indexList = indexList_.getValue();

			for (auto it = indexList.begin(); it != indexList.end(); ++it)
			{
				if ((*it).get<nlohmann::json::string_t>() == uuid)
				{
					auto packagePath = std::filesystem::path(assertPath_).append(uuid);
					std::filesystem::remove_all(packagePath);

					auto package = this->packageList_.find(std::string(uuid));
					if (package != this->packageList_.end())
						this->packageList_.erase(package);

					indexList.erase(it);
					return true;
				}
			}
		}
		catch (...)
		{
		}

		return false;
	}

	std::shared_ptr<octoon::Animation<float>>
	MotionImporter::loadPackage(const nlohmann::json& package) noexcept(false)
	{
		if (package["path"].is_string())
		{
			auto path = package["path"].get<nlohmann::json::string_t>();
			auto motion = octoon::VMDLoader::loadMotion(path);
			if (motion)
			{
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

	std::shared_ptr<octoon::Animation<float>>
	MotionImporter::loadMetaData(const nlohmann::json& metadata) noexcept
	{
		if (metadata.find("uuid") != metadata.end())
		{
			auto uuid = metadata["uuid"].get<nlohmann::json::string_t>();
			auto package = this->getPackage(uuid);
			if (package.is_object())
				return this->loadPackage(package);
		
		}
		if (metadata.find("path") != metadata.end())
		{
			auto path = metadata["path"].get<nlohmann::json::string_t>();
			return this->importMotion(path);
		}

		return nullptr;
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
			json["path"] = (*path).second;

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

			std::ofstream ifs(assertPath_ + "/index.json", std::ios_base::binary);
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
		std::ifstream indexStream(assertPath_ + "/index.json");
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