#ifndef UNREAL_MODEL_IMPORTER_H_
#define UNREAL_MODEL_IMPORTER_H_

#include <qpixmap.h>
#include <unreal_component.h>
#include <octoon/game_object.h>
#include <octoon/video/renderer.h>
#include <octoon/light/directional_light.h>
#include <octoon/light/environment_light.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/runtime/singleton.h>

#include "../unreal_component.h"
#include "../module/resource_module.h"

namespace unreal
{
	class ModelImporter final
	{
		OctoonDeclareSingleton(ModelImporter)
	public:
		ModelImporter() noexcept;
		~ModelImporter() noexcept;

		void open(std::string indexPath) noexcept(false);
		void close() noexcept;

		octoon::GameObjectPtr importModel(std::string_view path) noexcept(false);

		nlohmann::json importPackage(std::string_view path) noexcept(false);
		nlohmann::json getPackage(std::string_view uuid) noexcept;
		octoon::GameObjectPtr loadPackage(const nlohmann::json& package) noexcept(false);
		bool removePackage(std::string_view uuid) noexcept(false);

		octoon::GameObjectPtr loadMetaData(const nlohmann::json& metadata) noexcept;
		nlohmann::json createMetadata(const octoon::GameObjectPtr& gameObject) const noexcept;

		MutableLiveData<nlohmann::json>& getIndexList() noexcept;

		void save() noexcept(false);

	private:
		void initRenderScene() noexcept(false);
		void initPackageIndices() noexcept(false);
		void createModelPreview(const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, QPixmap& pixmap, int w, int h);

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;
		std::string assertPath_;

		MutableLiveData<nlohmann::json> modelIndexList_;

		std::map<std::string, nlohmann::json> packageList_;
		std::map<octoon::GameObjectWeakPtr, nlohmann::json, std::owner_less<octoon::GameObjectWeakPtr>> modelList_;
		std::map<octoon::GameObjectWeakPtr, std::string, std::owner_less<octoon::GameObjectWeakPtr>> modelPathList_;

		std::shared_ptr<octoon::PerspectiveCamera> camera_;
		std::shared_ptr<octoon::Geometry> geometry_;
		std::shared_ptr<octoon::DirectionalLight> directionalLight_;
		std::shared_ptr<octoon::EnvironmentLight> environmentLight_;
		std::shared_ptr<octoon::RenderScene> scene_;
		std::shared_ptr<octoon::GraphicsFramebuffer> framebuffer_;
	};
}

#endif