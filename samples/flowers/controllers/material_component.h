#ifndef FLOWER_MATERIAL_COMPONENT_H_
#define FLOWER_MATERIAL_COMPONENT_H_

#include "../flower_component.h"
#include "../module/resource_module.h"

#include <set>
#include <map>
#include <optional>
#include <qpixmap.h>
#include <octoon/video/renderer.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/material/material.h>

namespace flower
{
	class MaterialComponent final : public RabbitComponent<ResourceModule>
	{
	public:
		MaterialComponent() noexcept;
		virtual ~MaterialComponent() noexcept;

		nlohmann::json importMdl(std::string_view path) noexcept(false);
		nlohmann::json getPackage(std::string_view uuid) noexcept(false);

		const nlohmann::json& getIndexList() const noexcept;
		const nlohmann::json& getSceneList() const noexcept;

		void save() const noexcept;

		const std::shared_ptr<octoon::Material> getMaterial(std::string_view uuid) noexcept(false);
		void createMaterialPreview(const std::shared_ptr<octoon::Material>& material, QPixmap& pixmap, int w, int h);

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(MaterialComponent);
		}

	private:
		void onEnable() noexcept(false);
		void onDisable() noexcept;

	private:
		void initMaterialScene() noexcept(false);
		void initPackageIndices() noexcept(false);

	private:
		MaterialComponent(const MaterialComponent&) = delete;
		MaterialComponent& operator=(const MaterialComponent&) = delete;

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		nlohmann::json indexList_;
		nlohmann::json sceneList_;

		std::map<std::string, nlohmann::json> packageList_;

		octoon::hal::GraphicsFramebufferPtr framebuffer_;

		std::set<void*> materialSets_;
		std::map<std::string, octoon::MaterialPtr, std::less<>> materials_;
		std::map<octoon::MaterialPtr, std::string, std::less<>> materialsRemap_;

		std::shared_ptr<octoon::PerspectiveCamera> camera_;
		std::shared_ptr<octoon::Geometry> geometry_;
		std::shared_ptr<octoon::DirectionalLight> directionalLight_;
		std::shared_ptr<octoon::EnvironmentLight> environmentLight_;
		std::shared_ptr<octoon::RenderScene> scene_;
	};
}

#endif