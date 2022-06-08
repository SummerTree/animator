#ifndef UNREAL_MATERIAL_IMPORTER_H_
#define UNREAL_MATERIAL_IMPORTER_H_

#include <map>
#include <qpixmap.h>
#include <octoon/video/renderer.h>
#include <octoon/light/directional_light.h>
#include <octoon/light/environment_light.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/runtime/singleton.h>
#include <octoon/asset_importer.h>
#include "../viewmodel/mutable_live_data.h"

namespace unreal
{
	class MaterialImporter final : public octoon::AssetImporter
	{
		OctoonDeclareSingleton(MaterialImporter)
	public:
		MaterialImporter() noexcept;
		virtual ~MaterialImporter() noexcept;

		void open(std::string indexPath) noexcept(false);
		void close() noexcept;

		bool addMaterial(const std::shared_ptr<octoon::Material>& material);

		nlohmann::json createPackage(std::string_view path, std::string_view outputPath = "") noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::Material>& material, std::string_view materialPath = "", std::string_view texturePath = "") noexcept(false);
		
		std::shared_ptr<octoon::Material> loadPackage(std::string_view uuid, std::string_view outputPath = "") noexcept(false);
		std::shared_ptr<octoon::Material> loadPackage(const nlohmann::json& package) noexcept(false);

		MutableLiveData<nlohmann::json>& getSceneList() noexcept;
		const MutableLiveData<nlohmann::json>& getSceneList() const noexcept;

		void createMaterialPreview(const std::shared_ptr<octoon::Material>& material, QPixmap& pixmap, int w, int h);

	private:
		void initMaterialScene() noexcept(false);

	private:
		MaterialImporter(const MaterialImporter&) = delete;
		MaterialImporter& operator=(const MaterialImporter&) = delete;

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		MutableLiveData<nlohmann::json> sceneList_;

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