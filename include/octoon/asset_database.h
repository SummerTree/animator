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
#include <octoon/runtime/uuid.h>
#include <filesystem>
#include <set>

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

		nlohmann::json createAsset(const std::shared_ptr<Texture>& texture, const std::filesystem::path& outputPath) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<Animation>& animation, const std::filesystem::path& outputPath) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<Material>& material, const std::filesystem::path& outputPath) noexcept(false);
		nlohmann::json createAsset(const std::shared_ptr<GameObject>& object, const std::filesystem::path& outputPath) noexcept(false);
		nlohmann::json createAsset(const PMX& pmx, const std::filesystem::path& outputPath) noexcept(false);

		std::string getAssetPath(const std::shared_ptr<const RttiObject>& asset) noexcept;
		std::string getAssetPath(const std::shared_ptr<const RttiObject>& asset) const noexcept;

		std::string getAssetGuid(const std::shared_ptr<const RttiObject>& asset) noexcept;
		std::string getAssetGuid(const std::shared_ptr<const RttiObject>& asset) const noexcept;

		nlohmann::json getPackage(std::string_view uuid, const std::filesystem::path& outputPath) noexcept;
		nlohmann::json getPackage(const std::shared_ptr<RttiObject>& asset) const noexcept(false);

		std::shared_ptr<RttiObject> loadAssetAtPath(const std::filesystem::path& path, std::string_view uuid) noexcept(false);
		std::shared_ptr<RttiObject> loadAssetAtPackage(const nlohmann::json& package, const Rtti& type) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& path, std::string_view uuid = make_guid()) noexcept(false)
		{
			auto asset = loadAssetAtPath(path, uuid);
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

		bool needUpdate(std::string_view uuid) const noexcept;
		void addUpdateList(std::string_view uuid) noexcept(false);
		void removeUpdateList(std::string_view uuid) noexcept(false);
		void clearUpdate() noexcept;
		const std::set<std::string>& getUpdateList() const noexcept;

		std::shared_ptr<GraphicsTexture> createMaterialPreview(const std::shared_ptr<Material>& material);

	private:
		void initRenderScene() noexcept(false);
		void initMaterialScene() noexcept(false);

		void createMaterialPreview(const std::shared_ptr<Material>& material, Texture& texture);
		void createModelPreview(const std::shared_ptr<Geometry>& geometry, const math::BoundingBox& boundingBox, Texture& texture);

	private:
		AssetDatabase(const AssetDatabase&) = delete;
		AssetDatabase& operator=(const AssetDatabase&) = delete;

	private:
		std::uint32_t previewWidth_;
		std::uint32_t previewHeight_;

		std::set<std::string> updateList_;
		std::map<std::string, nlohmann::json> packageList_;

		std::map<std::weak_ptr<const RttiObject>, nlohmann::json, std::owner_less<std::weak_ptr<const RttiObject>>> assetList_;
		std::map<std::weak_ptr<const RttiObject>, std::string, std::owner_less<std::weak_ptr<const RttiObject>>> assetPathList_;
		std::map<std::weak_ptr<const RttiObject>, std::string, std::owner_less<std::weak_ptr<const RttiObject>>> assetGuidList_;

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