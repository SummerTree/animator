#include <octoon/asset_importer.h>
#include <octoon/texture_importer.h>
#include <octoon/vmd_importer.h>
#include <octoon/pmx_importer.h>
#include <octoon/obj_importer.h>
#include <octoon/ass_importer.h>
#include <octoon/fbx_importer.h>
#include <octoon/mesh_animation_component.h>

namespace octoon
{
	OctoonImplementSingleton(AssetImporter)
	OctoonImplementSubClass(AssetImporter, Object, "AssetImporter")

	std::map<std::filesystem::path, std::shared_ptr<const AssetImporter>> AssetImporter::assets_;

	AssetImporter::AssetImporter() noexcept
	{
	}

	AssetImporter::~AssetImporter() noexcept
	{
	}

	std::shared_ptr<const AssetImporter>
	AssetImporter::getAtPath(const std::filesystem::path& path) const noexcept
	{
		if (assets_.find(path) != assets_.end())
			return assets_.at(path);
		return nullptr;
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
				this->assets_[path] = motionImporter;
				return std::move(motion);
			}
		}
		else if (ext == u8".hdr" || ext == u8".bmp" || ext == u8".tga" || ext == u8".jpg" || ext == u8".png" || ext == u8".jpeg" || ext == u8".dds")
		{
			auto textureImporter = std::make_shared<TextureImporter>();
			auto texture = textureImporter->load(path);
			if (texture)
			{
				this->assets_[path] = textureImporter;
				return texture;
			}
		}
		else if (ext == u8".pmx")
		{
			auto modelImporter = std::make_shared<PMXImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				this->assets_[path] = modelImporter;
				return model;
			}
		}
		else if (ext == u8".obj")
		{
			auto modelImporter = std::make_shared<OBJImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				this->assets_[path] = modelImporter;
				return std::move(model);
			}
		}
		else if (ext == u8".fbx")
		{
			auto modelImporter = std::make_shared<FBXImporter>();
			auto model = modelImporter->load(path);
			if (model)
			{
				this->assets_[path] = modelImporter;
				return std::move(model);
			}
		}
		else if (ext == u8".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				auto alembic = model->addComponent<MeshAnimationComponent>();
				alembic->setFilePath(path);
				return std::move(model);
			}
		}

		return nullptr;
	}
}