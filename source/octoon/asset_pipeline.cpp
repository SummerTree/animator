#include <octoon/asset_pipeline.h>
#include <octoon/asset_manager.h>
#include <octoon/runtime/md5.h>
#include <octoon/runtime/guid.h>
#include <octoon/texture_importer.h>
#include <octoon/vmd_importer.h>
#include <octoon/pmx_importer.h>
#include <octoon/obj_importer.h>
#include <octoon/ass_importer.h>
#include <octoon/fbx_importer.h>
#include <octoon/material_importer.h>
#include <octoon/prefab_importer.h>
#include <octoon/alembic_importer.h>
#include <octoon/asset_database.h>
#include <octoon/mesh_animation_component.h>

#include <fstream>

namespace octoon
{
	AssetPipeline::AssetPipeline(const std::u8string& name) noexcept
		: name_(name)
	{
	}

	AssetPipeline::~AssetPipeline() noexcept
	{
		this->close();
	}

	void
	AssetPipeline::open(const std::filesystem::path& diskPath) noexcept(false)
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
	AssetPipeline::close() noexcept
	{
		rootPath_.clear();
	}

	const std::u8string&
	AssetPipeline::getName() const noexcept
	{
		return name_;
	}

	bool
	AssetPipeline::isValidPath(const std::filesystem::path& assetPath) const noexcept
	{
		if (name_.empty())
			return assetPath.is_absolute();
		else
		{
			auto path = std::filesystem::path(assetPath).u8string();
			for (auto& it : path)
			{
				if (it == '\\')
					it = '/';
			}

			auto length = name_.length();
			if (path.length() > length)
				return std::strncmp((char*)path.data(), (char*)name_.data(), length) == 0 ? true : false;

			return false;
		}
	}

	std::filesystem::path
	AssetPipeline::getAbsolutePath(const std::filesystem::path& assetPath) const noexcept
	{
		return std::filesystem::path(this->rootPath_).append(this->getRelativePath(assetPath).wstring());
	}

	std::filesystem::path
	AssetPipeline::getRelativePath(const std::filesystem::path& assetPath) const noexcept(false)
	{
		if (assetPath.is_absolute())
			return assetPath;
		else
		{
			auto path = std::filesystem::path(assetPath).u8string();
			for (auto& it : path)
			{
				if (it == '\\')
					it = '/';
			}

			auto length = name_.length();
			if (path.size() > length)
			{
				if (name_ == path.substr(0, length))
					return path.substr(length);
			}

			throw std::runtime_error(std::string("Invalid path ") + (char*)assetPath.u8string().c_str() + " failed.");
		}
	}

	void
	AssetPipeline::saveAssets() noexcept(false)
	{
		std::ofstream ifs(std::filesystem::path(rootPath_).append("manifest.json"), std::ios_base::binary);
		if (ifs)
		{
			nlohmann::json assetDb;

			for (auto& it : assets_)
			{
				auto path = it;
				if (std::filesystem::exists(AssetDatabase::instance()->getAbsolutePath(path)))
					assetDb[(char*)path.c_str()] = AssetDatabase::instance()->getAssetGuid(path);
			}

			auto dump = assetDb.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}
	}

	std::shared_ptr<Object>
	AssetPipeline::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		auto ext = path.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		std::shared_ptr<AssetImporter> assetImporter;
		if (ext == u8".vmd")
			assetImporter = std::make_shared<VMDImporter>();
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
			assetImporter = std::make_shared<TextureImporter>();
		else if (ext == u8".pmx")
			assetImporter = std::make_shared<PMXImporter>();
		else if (ext == u8".obj")
			assetImporter = std::make_shared<OBJImporter>();
		else if (ext == u8".fbx")
			assetImporter = std::make_shared<FBXImporter>();
		else if (ext == u8".mat")
			assetImporter = std::make_shared<MaterialImporter>();
		else if (ext == u8".abc")
			assetImporter = std::make_shared<AlembicImporter>();
		else if (ext == u8".prefab")
			assetImporter = std::make_shared<PrefabImporter>();

		if (assetImporter)
		{
			auto context = std::make_shared<AssetImporterContext>(path);
			assetImporter->onImportAsset(*context);

			auto mainObject = context->getMainObject();
			if (mainObject)
			{
				AssetManager::instance()->setAssetPath(mainObject, context->getAssetPath());

				for (auto& asset : context->getSubAssets())
					AssetManager::instance()->setAssetPath(asset, context->getAssetPath());

				AssetDatabase::instance()->importAsset(path);
				return mainObject;
			}
		}

		return nullptr;
	}
}