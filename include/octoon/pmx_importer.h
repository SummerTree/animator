#ifndef OCTOON_PMX_IMPORTER_H_
#define OCTOON_PMX_IMPORTER_H_

#include <octoon/game_object.h>
#include <octoon/geometry/geometry.h>
#include <octoon/pmx.h>
#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT PMXImporter final : public AssetImporter
	{
		OctoonDeclareSubClass(PMXImporter, AssetImporter)
	public:
		PMXImporter() noexcept;
		virtual ~PMXImporter() noexcept;

		std::shared_ptr<GameObject> load(const std::filesystem::path& path) noexcept(false);

		static bool save(const GameObject& gameObject, PMX& pmx, const std::filesystem::path& path) noexcept(false);
		static bool save(const GameObject& gameObject, const std::filesystem::path& path) noexcept(false);
	};
}

#endif