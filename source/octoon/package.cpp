#include <octoon/package.h>
#include <octoon/runtime/md5.h>
#include <octoon/runtime/guid.h>
#include <octoon/vmd_importer.h>
#include <octoon/fbx_importer.h>
#include <octoon/texture_importer.h>
#include <octoon/asset_database.h>
#include <octoon/mesh_animation_component.h>

#include <fstream>

namespace octoon
{
	Package::Package(const std::u8string& name) noexcept
		: name_(name)
	{
	}

	Package::~Package() noexcept
	{
		this->close();
	}

	void
	Package::open(const std::filesystem::path& diskPath) noexcept(false)
	{
		this->close();

		this->rootPath_ = diskPath;

		std::ifstream ifs(std::filesystem::path(diskPath).append("manifest.json"), std::ios_base::binary);
		if (ifs)
		{
			auto assetDb = nlohmann::json::parse(ifs);

			for (auto it = assetDb.begin(); it != assetDb.end(); ++it)
			{
				auto uuid = it.value();
				auto path = std::filesystem::path((char8_t*)it.key().c_str());
				AssetDatabase::instance()->importAsset(path);
			}
		}
	}

	void
	Package::close() noexcept
	{
		rootPath_.clear();
	}

	std::filesystem::path
	Package::getAbsolutePath(const std::filesystem::path& assetPath) const noexcept
	{
		return std::filesystem::path(this->rootPath_).append(assetPath.wstring());
	}

	void
	Package::saveAssets() noexcept(false)
	{
		std::ofstream ifs(std::filesystem::path(rootPath_).append("manifest.json"), std::ios_base::binary);
		if (ifs)
		{
			nlohmann::json assetDb;

			for (auto& it : assets_)
			{
				auto path = it.second->getAssetPath();
				if (std::filesystem::exists(AssetDatabase::instance()->getAbsolutePath(path)))
					assetDb[(char*)path.c_str()] = AssetDatabase::instance()->getAssetGuid(path);
			}

			auto dump = assetDb.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}
	}

	std::shared_ptr<Object>
	Package::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		auto assetImporter = AssetImporter::getAtPath(path);
		if (assetImporter)
		{
			assets_[path] = assetImporter;
			return assetImporter->onImportAsset();
		}

		return nullptr;
	}
}