#include <octoon/asset_importer.h>
#include <octoon/texture_importer.h>
#include <octoon/vmd_importer.h>
#include <octoon/pmx_importer.h>
#include <octoon/obj_importer.h>
#include <octoon/ass_importer.h>
#include <octoon/fbx_importer.h>
#include <octoon/mesh_animation_component.h>
#include <fstream>

namespace octoon
{
	OctoonImplementSubClass(AssetImporter, Object, "AssetImporter")

	std::map<std::string, std::filesystem::path> AssetImporter::uniques_;
	std::map<std::filesystem::path, std::string> AssetImporter::paths_;
	std::map<std::filesystem::path, std::shared_ptr<AssetImporter>> AssetImporter::assets_;

	AssetImporter::AssetImporter() noexcept
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
	AssetImporter::getExternalObjectMap() const
	{
		return externalObjectMap_;
	}

	std::shared_ptr<AssetImporter>
	AssetImporter::getAtPath(const std::filesystem::path& path) noexcept
	{
		if (assets_.find(path) != assets_.end())
			return assets_.at(path);
		return nullptr;
	}

	nlohmann::json
	AssetImporter::loadMetadataAtPath(const std::filesystem::path& path) noexcept(false)
	{
		std::ifstream ifs(std::filesystem::path(path).concat(L".meta"));
		if (ifs)
		{
			auto metaData = nlohmann::json::parse(ifs);
			if (metaData.contains("uuid"))
			{
				auto guid = metaData["uuid"].get<std::string>();
				paths_[path] = guid;
				uniques_[guid] = path;

				return metaData;
			}
		}

		return nlohmann::json();
	}

	std::shared_ptr<Object>
	AssetImporter::loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
	{
		auto ext = path.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == u8".vmd")
		{
			auto motionImporter = std::make_shared<VMDImporter>();
			auto motion = motionImporter->load(path);
			if (motion)
			{
				assets_[path] = motionImporter;
				return motion;
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto textureImporter = std::make_shared<TextureImporter>();
			auto texture = textureImporter->load(path);
			if (texture)
			{
				assets_[path] = textureImporter;
				return texture;
			}
		}
		else if (ext == u8".pmx")
		{
			auto modelImporter = std::make_shared<PMXImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				assets_[path] = modelImporter;
				return model;
			}
		}
		else if (ext == u8".obj")
		{
			auto modelImporter = std::make_shared<OBJImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				assets_[path] = modelImporter;
				return model;
			}
		}
		else if (ext == u8".fbx")
		{
			auto modelImporter = std::make_shared<FBXImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				assets_[path] = modelImporter;
				return model;
			}
		}
		else if (ext == u8".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				auto alembic = model->addComponent<MeshAnimationComponent>();
				alembic->setFilePath(path);
				return model;
			}
		}

		return nullptr;
	}
}