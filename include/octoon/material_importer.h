#ifndef UNREAL_MATERIAL_IMPORTER_H_
#define UNREAL_MATERIAL_IMPORTER_H_

#include <map>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/runtime/singleton.h>

namespace octoon
{
	class OCTOON_EXPORT MaterialImporter final
	{
		OctoonDeclareSingleton(MaterialImporter)
	public:
		MaterialImporter() noexcept;
		virtual ~MaterialImporter() noexcept;

		bool addMaterial(const std::shared_ptr<octoon::Material>& material);
		std::shared_ptr<octoon::Material> getMaterial(std::string_view uuid) noexcept(false);

		nlohmann::json& getSceneList() noexcept;
		const nlohmann::json& getSceneList() const noexcept;

	private:
		MaterialImporter(const MaterialImporter&) = delete;
		MaterialImporter& operator=(const MaterialImporter&) = delete;

	private:
		std::string texturePath_;
		std::string materialPath_;

		nlohmann::json sceneList_;

		std::map<std::string, std::shared_ptr<octoon::Material>> materials_;
		std::map<std::shared_ptr<octoon::Material>, std::string> materialUUIDs_;
	};
}

#endif