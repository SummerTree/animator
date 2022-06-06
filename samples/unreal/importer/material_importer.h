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

#include "../unreal_component.h"
#include "../module/resource_module.h"

namespace unreal
{
	class MaterialImporter final
	{
		OctoonDeclareSingleton(MaterialImporter)
	public:
		MaterialImporter() noexcept;
		virtual ~MaterialImporter() noexcept;

		void open(std::string indexPath) noexcept(false);
		void close() noexcept;

		bool addMaterial(const std::shared_ptr<octoon::Material>& material);

		nlohmann::json createPackage(std::string_view path, std::string_view outputPath = "") noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::MeshStandardMaterial>& material, std::string_view materialPath = "", std::string_view texturePath = "") noexcept(false);
		
		nlohmann::json getPackage(std::string_view uuid, std::string_view outputPath = "") noexcept(false);
		nlohmann::json getPackage(const std::shared_ptr<octoon::Material>& material) const noexcept(false);

		std::shared_ptr<octoon::MeshStandardMaterial> loadPackage(std::string_view uuid, std::string_view outputPath = "") noexcept(false);
		std::shared_ptr<octoon::MeshStandardMaterial> loadPackage(const nlohmann::json& package) noexcept(false);
		void removePackage(std::string_view uuid) noexcept(false);

		MutableLiveData<nlohmann::json>& getIndexList() noexcept;
		MutableLiveData<nlohmann::json>& getSceneList() noexcept;

		const MutableLiveData<nlohmann::json>& getIndexList() const noexcept;
		const MutableLiveData<nlohmann::json>& getSceneList() const noexcept;

		void createMaterialPreview(const std::shared_ptr<octoon::Material>& material, QPixmap& pixmap, int w, int h);

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(MaterialImporter);
		}

		void save() const noexcept(false);

	private:
		void initMaterialScene() noexcept(false);
		void initPackageIndices() noexcept(false);

	private:
		MaterialImporter(const MaterialImporter&) = delete;
		MaterialImporter& operator=(const MaterialImporter&) = delete;

	private:
		std::string defaultPath_;

		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		MutableLiveData<nlohmann::json> indexList_;
		MutableLiveData<nlohmann::json> sceneList_;

		std::map<std::string, nlohmann::json> packageList_;

		octoon::GraphicsFramebufferPtr framebuffer_;

		std::map<std::string, std::shared_ptr<octoon::MeshStandardMaterial>, std::less<>> materials_;
		std::map<std::weak_ptr<octoon::Material>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::Material>>> materialList_;
		std::map<std::weak_ptr<octoon::Material>, std::string, std::owner_less<std::weak_ptr<octoon::Material>>> materialPathList_;

		std::shared_ptr<octoon::PerspectiveCamera> camera_;
		std::shared_ptr<octoon::Geometry> geometry_;
		std::shared_ptr<octoon::DirectionalLight> directionalLight_;
		std::shared_ptr<octoon::EnvironmentLight> environmentLight_;
		std::shared_ptr<octoon::RenderScene> scene_;
	};
}

#endif