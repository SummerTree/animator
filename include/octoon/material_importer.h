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

		void open(std::string indexPath) noexcept(false);
		void close() noexcept;

		void setTexturePath(std::string_view path);
		void setMaterialPath(std::string_view path);

		bool addMaterial(const std::shared_ptr<octoon::Material>& material);

		nlohmann::json createPackage(std::string_view path, std::string_view outputPath = "") noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::Material>& material) noexcept(false);
		
		std::shared_ptr<octoon::Material> loadPackage(std::string_view uuid, std::string_view outputPath = "") noexcept(false);
		std::shared_ptr<octoon::Material> loadPackage(const nlohmann::json& package) noexcept(false);

		nlohmann::json& getSceneList() noexcept;
		const nlohmann::json& getSceneList() const noexcept;

		std::shared_ptr<octoon::GraphicsTexture> createMaterialPreview(const std::shared_ptr<octoon::Material>& material);

	private:
		void initMaterialScene() noexcept(false);
		void createMaterialPreview(const std::shared_ptr<octoon::Material>& material, octoon::Texture& texture);

	private:
		MaterialImporter(const MaterialImporter&) = delete;
		MaterialImporter& operator=(const MaterialImporter&) = delete;

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		std::string texturePath_;
		std::string materialPath_;

		nlohmann::json sceneList_;

		std::map<std::string, std::shared_ptr<octoon::Material>> materials_;

		std::shared_ptr<octoon::PerspectiveCamera> camera_;
		std::shared_ptr<octoon::Geometry> geometry_;
		std::shared_ptr<octoon::DirectionalLight> directionalLight_;
		std::shared_ptr<octoon::EnvironmentLight> environmentLight_;
		std::shared_ptr<octoon::RenderScene> scene_;
		std::shared_ptr<octoon::GraphicsFramebuffer> framebuffer_;
	};
}

#endif