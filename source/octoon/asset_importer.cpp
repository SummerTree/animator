#include <octoon/asset_importer.h>
#include <octoon/runtime/uuid.h>
#include <fstream>
#include <filesystem>
#include <set>

namespace octoon
{
	AssetImporter::AssetImporter() noexcept
	{
	}

	AssetImporter::~AssetImporter() noexcept
	{
	}

	void
	AssetImporter::open(std::string indexPath) noexcept(false)
	{
		if (std::filesystem::exists(indexPath))
		{
			this->assertPath_ = indexPath;
			this->initPackageIndices();
		}
	}

	void
	AssetImporter::close() noexcept
	{
	}

	nlohmann::json
	AssetImporter::getPackage(std::string_view uuid, std::string_view outputPath) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid).append("package.json"));
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
	AssetImporter::getPackage(const std::shared_ptr<RttiObject>& object) const noexcept(false)
	{
		if (object)
		{
			auto it = assetList_.find(object);
			if (it != assetList_.end())
			{
				auto& package = (*it).second;
				auto path = package["path"].get<nlohmann::json::string_t>();
				if (std::filesystem::exists(path))
					return package;
			}
		}

		return nlohmann::json();
	}

	void
	AssetImporter::removePackage(std::string_view uuid, std::string_view outputPath) noexcept(false)
	{
		auto& indexList = indexList_;

		for (auto index = indexList.begin(); index != indexList.end(); ++index)
		{
			if ((*index).get<nlohmann::json::string_t>() == uuid)
			{
				auto packagePath = std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid);

				for (auto& it : std::filesystem::recursive_directory_iterator(packagePath))
					std::filesystem::permissions(it, std::filesystem::perms::owner_write);

				std::filesystem::remove_all(packagePath);

				auto package = this->packageList_.find(std::string(uuid));
				if (package != this->packageList_.end())
					this->packageList_.erase(package);

				indexList.erase(index);
				break;
			}
		}
	}

	nlohmann::json&
	AssetImporter::getIndexList() noexcept
	{
		return indexList_;
	}

	std::string
	AssetImporter::getPackagePath(const std::shared_ptr<octoon::RttiObject>& asset) const noexcept
	{
		if (assetPathList_.contains(asset))
			return assetPathList_.at(asset);
		return std::string();
	}

	void
	AssetImporter::saveAssets() noexcept(false)
	{
		if (!std::filesystem::exists(assertPath_))
			std::filesystem::create_directory(assertPath_);

		std::ofstream ifs(assertPath_ + "/index.json", std::ios_base::binary);
		if (ifs)
		{
			auto data = indexList_.dump();
			ifs.write(data.c_str(), data.size());
		}
	}

	void
	AssetImporter::clearCache() noexcept
	{
		assetCache_.clear();
		assetPackageCache_.clear();
	}

	void
	AssetImporter::initPackageIndices() noexcept(false)
	{
		auto& indexList = indexList_;

		std::ifstream indexStream(assertPath_ + "/index.json");
		if (indexStream)
			indexList = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : indexList)
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
			this->saveAssets();
		}
	}
}