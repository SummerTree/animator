#ifndef OCTOON_MODEL_IMPORTER_H_
#define OCTOON_MODEL_IMPORTER_H_

#include <octoon/game_object.h>
#include <octoon/pmx_loader.h>
#include <octoon/video/renderer.h>
#include <octoon/texture/texture.h>
#include <octoon/light/directional_light.h>
#include <octoon/light/environment_light.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/runtime/singleton.h>
#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT ModelImporter final : public AssetImporter
	{
		OctoonDeclareSingleton(ModelImporter)
	public:
		ModelImporter() noexcept;
		~ModelImporter() noexcept;

		void open(std::string indexPath) noexcept(false) override;
		void close() noexcept override;

		octoon::GameObjectPtr loadPackage(const nlohmann::json& package, octoon::PMXLoadFlags flags = octoon::PMXLoadFlagBits::AllBit) noexcept(false);
		octoon::GameObjectPtr loadMetaData(const nlohmann::json& metadata, octoon::PMXLoadFlags flags = octoon::PMXLoadFlagBits::AllBit) noexcept;

		nlohmann::json createPackage(std::string_view path) noexcept(false);
		nlohmann::json createPackage(const octoon::GameObjectPtr& gameObject) const noexcept;

	private:
		void initRenderScene() noexcept(false);
		void createModelPreview(const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, octoon::Texture& texture);

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		std::shared_ptr<octoon::PerspectiveCamera> camera_;
		std::shared_ptr<octoon::Geometry> geometry_;
		std::shared_ptr<octoon::DirectionalLight> directionalLight_;
		std::shared_ptr<octoon::EnvironmentLight> environmentLight_;
		std::shared_ptr<octoon::RenderScene> scene_;
		std::shared_ptr<octoon::GraphicsFramebuffer> framebuffer_;
	};
}

#endif