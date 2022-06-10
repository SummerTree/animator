#ifndef OCTOON_ASSET_DATABASE_H_
#define OCTOON_ASSET_DATABASE_H_

#include <octoon/asset_importer.h>
#include <octoon/pmx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/animation/animation.h>
#include <octoon/light/directional_light.h>
#include <octoon/light/environment_light.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/video/renderer.h>

namespace octoon
{
	class OCTOON_EXPORT AssetDatabase final
	{
		OctoonDeclareSingleton(AssetDatabase)
	public:
		AssetDatabase() noexcept;
		virtual ~AssetDatabase() noexcept;

		void open() noexcept(false);
		void close() noexcept;

		nlohmann::json createAsset(std::string_view filepath, std::string_view outputPath) noexcept(false);
		nlohmann::json createAsset(const Texture& texture, std::string_view outputPath) noexcept(false);
		nlohmann::json createAsset(const Animation& animation, std::string_view outputPath) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<Material>& material, std::string_view outputPath) noexcept(false);
		nlohmann::json createAsset(const PMX& pmx, std::string_view outputPath) noexcept(false);

		std::string getAssetPath(const std::shared_ptr<RttiObject>& asset) noexcept;
		std::string getAssetPath(const std::shared_ptr<RttiObject>& asset) const noexcept;

		std::string getAssetGuid(const std::shared_ptr<RttiObject>& asset) noexcept;
		std::string getAssetGuid(const std::shared_ptr<RttiObject>& asset) const noexcept;

		nlohmann::json getPackage(std::string_view uuid, std::string_view outputPath) noexcept;
		nlohmann::json getPackage(const std::shared_ptr<octoon::RttiObject>& asset) const noexcept(false);

		std::shared_ptr<RttiObject> loadAssetAtPath(std::string_view path) noexcept(false);
		std::shared_ptr<RttiObject> loadAssetAtPath(std::string_view path, octoon::PMXLoadFlags flags) noexcept(false);
		std::shared_ptr<RttiObject> loadAssetAtPackage(const nlohmann::json& package, const Rtti& type) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(std::string_view path) noexcept(false)
		{
			auto asset = loadAssetAtPath(path);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of<RttiObject, T>::value>>
		std::shared_ptr<T> loadAssetAtPackage(const nlohmann::json& package) noexcept(false)
		{
			auto asset = loadAssetAtPackage(package, *T::getRtti());
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

		std::shared_ptr<octoon::GraphicsTexture> createMaterialPreview(const std::shared_ptr<Material>& material);

	private:
		void initRenderScene() noexcept(false);
		void initMaterialScene() noexcept(false);

		void createMaterialPreview(const std::shared_ptr<Material>& material, octoon::Texture& texture);
		void createModelPreview(const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, octoon::Texture& texture);

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		std::map<std::string, nlohmann::json> packageList_;

		std::map<std::string, std::shared_ptr<octoon::RttiObject>> assetCache_;
		std::map<std::weak_ptr<octoon::RttiObject>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetList_;
		std::map<std::weak_ptr<octoon::RttiObject>, std::string, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetPathList_;
		std::map<std::weak_ptr<octoon::RttiObject>, std::string, std::owner_less<std::weak_ptr<octoon::RttiObject>>> assetGuidList_;

		std::shared_ptr<octoon::PerspectiveCamera> camera_;
		std::shared_ptr<octoon::Geometry> geometry_;
		std::shared_ptr<octoon::DirectionalLight> directionalLight_;
		std::shared_ptr<octoon::EnvironmentLight> environmentLight_;
		std::shared_ptr<octoon::RenderScene> scene_;
		std::shared_ptr<octoon::GraphicsFramebuffer> framebuffer_;

		std::shared_ptr<PerspectiveCamera> materialCamera_;
		std::shared_ptr<Geometry> materialGeometry_;
		std::shared_ptr<DirectionalLight> materialDirectionalLight_;
		std::shared_ptr<EnvironmentLight> materialEnvironmentLight_;
		std::shared_ptr<RenderScene> materialScene_;
		std::shared_ptr<GraphicsFramebuffer> materialFramebuffer_;
	};
}

#endif