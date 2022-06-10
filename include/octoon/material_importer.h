#ifndef UNREAL_MATERIAL_IMPORTER_H_
#define UNREAL_MATERIAL_IMPORTER_H_

#include <map>
#include <octoon/video/renderer.h>
#include <octoon/light/directional_light.h>
#include <octoon/light/environment_light.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/texture/texture.h>
#include <octoon/runtime/singleton.h>
#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT MaterialImporter final : public octoon::AssetImporter
	{
		OctoonDeclareSingleton(MaterialImporter)
	public:
		MaterialImporter() noexcept;
		virtual ~MaterialImporter() noexcept;

		bool addMaterial(const std::shared_ptr<octoon::Material>& material);

		nlohmann::json createPackage(std::string_view path) noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::Material>& material) noexcept(false);
		
		std::shared_ptr<octoon::Material> loadPackage(std::string_view uuid, std::string_view outputPath = "") noexcept(false);

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
	};
}

#endif