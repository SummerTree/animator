#ifndef OCTOON_PREFAB_IMPORTER_H_
#define OCTOON_PREFAB_IMPORTER_H_

#include <octoon/asset_importer.h>
#include <octoon/game_object.h>

namespace octoon
{
	class OCTOON_EXPORT PrefabImporter final : public AssetImporter
	{
		OctoonDeclareSubClass(PrefabImporter, AssetImporter)
	public:
		PrefabImporter() noexcept;
		PrefabImporter(const std::filesystem::path& path) noexcept;
		virtual ~PrefabImporter() noexcept;

		virtual std::shared_ptr<Object> importer() noexcept(false) override;

	private:
		PrefabImporter(const PrefabImporter&) = delete;
		PrefabImporter& operator=(const PrefabImporter&) = delete;
	};
}

#endif