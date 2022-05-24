#include "model_component.h"
#include "unreal_behaviour.h"
#include <octoon/image/image.h>
#include <fstream>
#include <filesystem>
#include <quuid.h>

namespace unreal
{
	ModelComponent::ModelComponent() noexcept
	{
	}

	ModelComponent::~ModelComponent() noexcept
	{
	}

	nlohmann::json
	ModelComponent::importModel(std::string_view filepath) noexcept(false)
	{
		return nlohmann::json();
	}

	nlohmann::json
	ModelComponent::getPackage(std::string_view uuid) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(this->getModel()->modelPath).append(uuid).append("package.json"));
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

	const nlohmann::json&
	ModelComponent::getIndexList() const noexcept
	{
		return this->indexList_;
	}

	void
	ModelComponent::save() noexcept(false)
	{
		try
		{
			if (!std::filesystem::exists(this->getModel()->modelPath))
				std::filesystem::create_directory(this->getModel()->modelPath);

			std::ofstream ifs(this->getModel()->modelPath + "/index.json", std::ios_base::binary);
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
	ModelComponent::initPackageIndices() noexcept(false)
	{
		std::ifstream indexStream(this->getModel()->modelPath + "/index.json");
		if (indexStream)
			this->indexList_ = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : this->indexList_)
		{		 
			if (!std::filesystem::exists(std::filesystem::path(this->getModel()->modelPath).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(this->getModel()->modelPath))
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
	ModelComponent::onEnable() noexcept
	{
		if (std::filesystem::exists(this->getModel()->modelPath))
			this->initPackageIndices();
		else
			std::filesystem::create_directory(this->getModel()->modelPath);
	}

	void
	ModelComponent::onDisable() noexcept
	{
	}
}