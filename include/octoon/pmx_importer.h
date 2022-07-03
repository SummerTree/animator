#ifndef OCTOON_PMX_IMPORTER_H_
#define OCTOON_PMX_IMPORTER_H_

#include <octoon/pmx.h>
#include <octoon/game_object.h>
#include <octoon/material/material.h>
#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT PMXImporter final : public AssetImporter
	{
		OctoonDeclareSubClass(PMXImporter, AssetImporter)
	public:
		PMXImporter() noexcept;
		PMXImporter(const std::filesystem::path& path) noexcept;
		virtual ~PMXImporter() noexcept;

		virtual std::shared_ptr<Object> onImportAsset() noexcept(false) override;

		static bool save(const GameObject& gameObject, PMX& pmx, const std::filesystem::path& path) noexcept(false);
		static bool save(const GameObject& gameObject, const std::filesystem::path& path) noexcept(false);

	private:
		void createBones(const PMX& pmx, GameObjects& bones) noexcept(false);
		void createClothes(const PMX& pmx, GameObjectPtr& meshes, const GameObjects& bones) noexcept(false);
		void createColliders(const PMX& pmx, GameObjects& bones) noexcept(false);
		void createRigidbodies(const PMX& pmx, GameObjects& bones) noexcept(false);
		void createJoints(const PMX& pmx, GameObjects& bones) noexcept(false);
		void createMorph(const PMX& pmx, GameObjectPtr& mesh) noexcept(false);
		void createMeshes(const PMX& pmx, GameObjectPtr& object, const GameObjects& bones) noexcept(false);
		void createMaterials(const PMX& pmx, GameObjectPtr& object, Materials& materials) noexcept(false);
	};
}

#endif