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
		this->close();
	}

	void
	AssetImporter::open(const std::filesystem::path& path) noexcept(false)
	{
		this->assertPath_ = path;

		if (std::filesystem::exists(path))
			this->indexList_ = this->getPackageIndices(this->assertPath_);
	}

	void
	AssetImporter::close() noexcept
	{
		this->assertPath_.clear();
		this->indexList_.clear();
	}

	bool
	AssetImporter::hasPackage(std::string_view uuid_) const noexcept
	{
		std::string uuid(uuid_);

		for (auto& it : this->indexList_)
		{
			if (it == uuid)
				return true;
		}

		return false;
	}

	nlohmann::json
	AssetImporter::getPackage(std::string_view uuid) noexcept
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

	void
	AssetImporter::removeAsset(std::string_view uuid) noexcept(false)
	{
		auto& indexList = indexList_;

		for (auto index = indexList.begin(); index != indexList.end(); ++index)
		{
			if ((*index).get<nlohmann::json::string_t>() == uuid)
			{
				auto packagePath = std::filesystem::path(assertPath_).append(uuid);

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

	const nlohmann::json&
	AssetImporter::getIndexList() const noexcept
	{
		return indexList_;
	}

	void
	AssetImporter::addPackage(const nlohmann::json& uuid)
	{
		indexList_.push_back(uuid);
	}

	const std::filesystem::path&
	AssetImporter::getAssertPath() const
	{
		return assertPath_;
	}

	void
	AssetImporter::saveAssets() const noexcept(false)
	{
		if (!std::filesystem::exists(assertPath_))
			std::filesystem::create_directories(assertPath_);

		std::ofstream ifs(std::filesystem::path(assertPath_).append("index.json"), std::ios_base::binary);
		if (ifs)
		{
			auto data = indexList_.dump();
			ifs.write(data.c_str(), data.size());
		}
	}

	nlohmann::json
	AssetImporter::getPackageIndices(const std::filesystem::path& path) noexcept(false)
	{
		nlohmann::json indexList;

		std::ifstream indexStream(std::filesystem::path(path).append("index.json"));
		if (indexStream)
			indexList = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : indexList)
		{
			if (!std::filesystem::exists(std::filesystem::path(path).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(path))
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

		return indexList;
	}
}