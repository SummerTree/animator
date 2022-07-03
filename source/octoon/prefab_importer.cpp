#include <octoon/prefab_importer.h>
#include <octoon/asset_database.h>
#include <fstream>

namespace octoon
{
	OctoonImplementSubClass(PrefabImporter, AssetImporter, "PrefabImporter")

	PrefabImporter::PrefabImporter() noexcept
	{
	}

	PrefabImporter::PrefabImporter(const std::filesystem::path& path) noexcept
		: AssetImporter(path)
	{
	}

	PrefabImporter::~PrefabImporter()
	{
	}

	std::shared_ptr<Object>
	PrefabImporter::importer() noexcept(false)
	{
		auto filepath = this->getAssetPath();

		std::ifstream ifs(filepath);
		if (ifs)
		{
			auto prefab = nlohmann::json::parse(ifs);
			auto object = std::make_shared<GameObject>();
			object->load(prefab);

			return object;
		}
	}
}