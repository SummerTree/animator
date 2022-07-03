#include <octoon/asset_importer.h>
#include <octoon/texture_importer.h>
#include <octoon/vmd_importer.h>
#include <octoon/pmx_importer.h>
#include <octoon/obj_importer.h>
#include <octoon/ass_importer.h>
#include <octoon/fbx_importer.h>
#include <octoon/material_importer.h>
#include <octoon/prefab_importer.h>
#include <octoon/alembic_importer.h>
#include <fstream>

namespace octoon
{
	OctoonImplementSubInterface(AssetImporter, Object, "AssetImporter")

	std::map<std::filesystem::path, std::shared_ptr<AssetImporter>> AssetImporter::assets_;

	AssetImporter::AssetImporter() noexcept
	{
	}

	AssetImporter::AssetImporter(const std::filesystem::path& path) noexcept
		: assetPath_(path)
	{
	}

	AssetImporter::~AssetImporter() noexcept
	{
	}

	void
	AssetImporter::addRemap(const std::shared_ptr<const Object>& subAsset)
	{
		this->externalObjectMap_.push_back(subAsset);
	}

	const std::vector<std::weak_ptr<const Object>>&
	AssetImporter::getExternalObjects() const
	{
		return externalObjectMap_;
	}

	const std::filesystem::path&
	AssetImporter::getAssetPath() const noexcept
	{
		return assetPath_;
	}

	std::shared_ptr<AssetImporter>
	AssetImporter::getAtPath(const std::filesystem::path& path) noexcept
	{
		if (assets_.find(path) != assets_.end())
			return assets_.at(path);

		auto ext = path.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		std::shared_ptr<AssetImporter> assetImporter;
		if (ext == u8".vmd")
			assetImporter = std::make_shared<VMDImporter>(path);
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
			assetImporter = std::make_shared<TextureImporter>(path);
		else if (ext == u8".pmx")
			assetImporter = std::make_shared<PMXImporter>(path);
		else if (ext == u8".obj")
			assetImporter = std::make_shared<OBJImporter>(path);
		else if (ext == u8".fbx")
			assetImporter = std::make_shared<FBXImporter>(path);
		else if (ext == u8".mat")
			assetImporter = std::make_shared<MaterialImporter>(path);
		else if (ext == u8".abc")
			assetImporter = std::make_shared<AlembicImporter>(path);
		else if (ext == u8".prefab")
			assetImporter = std::make_shared<PrefabImporter>(path);

		if (assetImporter)
			assets_[path] = assetImporter;

		return assetImporter;
	}

	nlohmann::json
	AssetImporter::loadMetadataAtPath(const std::filesystem::path& path) noexcept(false)
	{
		std::ifstream ifs(std::filesystem::path(path).concat(L".meta"));
		if (ifs)
		{
			auto metaData = nlohmann::json::parse(ifs);
			return metaData;
		}

		return nlohmann::json();
	}
}