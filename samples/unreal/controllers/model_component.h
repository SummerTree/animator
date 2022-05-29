#ifndef UNREAL_MODEL_COMPONENT_H_
#define UNREAL_MODEL_COMPONENT_H_

#include <unreal_component.h>
#include <octoon/game_object.h>
#include <octoon/video/renderer.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/camera/ortho_camera.h>

#include <qpixmap.h>

#include "../unreal_component.h"
#include "../module/resource_module.h"

namespace unreal
{
	class ModelComponent final : public UnrealComponent<ResourceModule>
	{
	public:
		ModelComponent() noexcept;
		~ModelComponent() noexcept;

		nlohmann::json importModel(std::string_view path) noexcept(false);
		nlohmann::json getPackage(std::string_view uuid) noexcept;
		bool removePackage(std::string_view uuid) noexcept;

		void createModelPreview(const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, QPixmap& pixmap, int w, int h);

		void save() noexcept(false);

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(ModelComponent);
		}

	private:
		void initRenderScene() noexcept(false);
		void initPackageIndices() noexcept(false);

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		std::map<std::string, nlohmann::json> packageList_;

		std::shared_ptr<octoon::PerspectiveCamera> camera_;
		std::shared_ptr<octoon::Geometry> geometry_;
		std::shared_ptr<octoon::DirectionalLight> directionalLight_;
		std::shared_ptr<octoon::EnvironmentLight> environmentLight_;
		std::shared_ptr<octoon::RenderScene> scene_;
		std::shared_ptr<octoon::GraphicsFramebuffer> framebuffer_;
	};
}

#endif