#include "motion_component.h"
#include "unreal_behaviour.h"
#include <fstream>
#include <filesystem>
#include <quuid.h>

namespace unreal
{
	MotionComponent::MotionComponent() noexcept
	{
	}

	MotionComponent::~MotionComponent() noexcept
	{
	}

	nlohmann::json
	MotionComponent::importMotion(std::string_view filepath) noexcept(false)
	{
		if (std::filesystem::exists(filepath))
		{
			auto id = QUuid::createUuid().toString();
			auto uuid = id.toStdString().substr(1, id.length() - 2);

			auto rootPath = std::filesystem::path(this->getModel()->motionPath).append(uuid);
			auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directory(this->getModel()->motionPath);
			std::filesystem::create_directory(rootPath);
			std::filesystem::copy(filepath, motionPath);

			nlohmann::json item;
			item["uuid"] = uuid;
			item["name"] = std::filesystem::path(filepath).filename().string();
			item["path"] = motionPath.string();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = item.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			indexList_.push_back(uuid);

			return item;
		}

		return nlohmann::json();
	}

	nlohmann::json
	MotionComponent::getPackage(std::string_view uuid) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(this->getModel()->motionPath).append(uuid).append("package.json"));
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
	MotionComponent::removePackage(std::string_view uuid) noexcept
	{
		auto it = this->indexList_.find(std::string(uuid));
		if (it != this->indexList_.end())
		{
			auto packagePath = std::filesystem::path(this->getModel()->hdriPath).append(uuid);
			std::filesystem::remove_all(packagePath);

			indexList_.erase(it);
		}

		return false;
	}

	const nlohmann::json&
	MotionComponent::getIndexList() const noexcept
	{
		return this->indexList_;
	}

	void
	MotionComponent::save() noexcept(false)
	{
		try
		{
			if (!std::filesystem::exists(this->getModel()->motionPath))
				std::filesystem::create_directory(this->getModel()->motionPath);

			std::ofstream ifs(this->getModel()->motionPath + "/index.json", std::ios_base::binary);
			if (ifs)
			{
				auto data = indexList_.dump();
				ifs.write(data.c_str(), data.size());
			}
		}
		catch (...)
		{
		}
	}

	void
	MotionComponent::initPackageIndices() noexcept(false)
	{
		std::ifstream indexStream(this->getModel()->motionPath + "/index.json");
		if (indexStream)
			this->indexList_ = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : this->indexList_)
		{		 
			if (!std::filesystem::exists(std::filesystem::path(this->getModel()->motionPath).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(this->getModel()->motionPath))
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

			this->indexList_ = json;
			this->save();
		}
	}

	void
	MotionComponent::onEnable() noexcept
	{
		if (std::filesystem::exists(this->getModel()->motionPath))
			this->initPackageIndices();
		else
			std::filesystem::create_directory(this->getModel()->motionPath);
	}

	void
	MotionComponent::onDisable() noexcept
	{
	}
}