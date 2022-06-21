#ifndef OCTOON_ASSET_PREVIEW_H_
#define OCTOON_ASSET_PREVIEW_H_

#include <octoon/pmx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/animation/animation.h>
#include <octoon/light/directional_light.h>
#include <octoon/light/environment_light.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/video/render_scene.h>
#include <filesystem>
#include <set>

namespace octoon
{
	class OCTOON_EXPORT AssetPreview final
	{
		OctoonDeclareSingleton(AssetPreview)
	public:
		AssetPreview() noexcept;
		virtual ~AssetPreview() noexcept;

		void open() noexcept(false);
		void close() noexcept;

		std::shared_ptr<Texture> getAssetPreview(const std::shared_ptr<Texture>& hdri);
		std::shared_ptr<Texture> getAssetPreview(const std::shared_ptr<Material>& material);
		std::shared_ptr<Texture> getAssetPreview(const std::shared_ptr<GameObject>& gameObject);
		std::shared_ptr<Texture> getAssetPreview(const PMX& pmx);

	private:
		void initRenderScene() noexcept(false);
		void initMaterialScene() noexcept(false);

	private:
		AssetPreview(const AssetPreview&) = delete;
		AssetPreview& operator=(const AssetPreview&) = delete;

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		std::shared_ptr<PerspectiveCamera> camera_;
		std::shared_ptr<Geometry> geometry_;
		std::shared_ptr<DirectionalLight> directionalLight_;
		std::shared_ptr<EnvironmentLight> environmentLight_;
		std::shared_ptr<RenderScene> scene_;
		std::shared_ptr<GraphicsFramebuffer> framebuffer_;

		std::shared_ptr<PerspectiveCamera> materialCamera_;
		std::shared_ptr<Geometry> materialGeometry_;
		std::shared_ptr<DirectionalLight> materialDirectionalLight_;
		std::shared_ptr<EnvironmentLight> materialEnvironmentLight_;
		std::shared_ptr<RenderScene> materialScene_;
		std::shared_ptr<GraphicsFramebuffer> materialFramebuffer_;
	};
}

#endif