#include <octoon/alembic_importer.h>
#include <octoon/asset_database.h>
#include <octoon/mesh_animation_component.h>

namespace octoon
{
	OctoonImplementSubClass(AlembicImporter, AssetImporter, "AlembicImporter")

	AlembicImporter::AlembicImporter() noexcept
	{
	}

	AlembicImporter::AlembicImporter(const std::filesystem::path& path) noexcept
		: AssetImporter(path)
	{
	}

	AlembicImporter::~AlembicImporter()
	{
	}

	std::shared_ptr<Object>
	AlembicImporter::onImportAsset() noexcept(false)
	{
		auto filepath = AssetDatabase::instance()->getAbsolutePath(this->getAssetPath());

		auto model = std::make_shared<GameObject>();
		if (model)
		{
			auto alembic = model->addComponent<MeshAnimationComponent>();
			alembic->setFilePath(filepath);
			return model;
		}

		return nullptr;
	}
}