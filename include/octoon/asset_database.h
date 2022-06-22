#ifndef OCTOON_ASSET_DATABASE_H_
#define OCTOON_ASSET_DATABASE_H_

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

		void open(const std::filesystem::path& assetPath) noexcept(false);
		void close() noexcept;

		void createAsset(const std::shared_ptr<Texture>& texture, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<Animation>& animation, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<Material>& material, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<PMX>& pmx, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<GameObject>& object, const std::filesystem::path& relativePath) noexcept(false);

		void deleteAsset(const std::filesystem::path& relativePath) noexcept(false);

		std::filesystem::path getAssetPath(const std::shared_ptr<const RttiObject>& asset) const noexcept;

		std::string getAssetGuid(const std::filesystem::path& relativePath) const noexcept;
		std::string getAssetGuid(const std::shared_ptr<const RttiObject>& asset) const noexcept;

		std::shared_ptr<RttiObject> loadAssetAtPath(const std::filesystem::path& relativePath) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& relativePath) noexcept(false)
		{
			auto asset = loadAssetAtPath(relativePath);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

	private:
		nlohmann::json loadMetadataAtPath(const std::filesystem::path& path) noexcept(false);

	private:
		AssetDatabase(const AssetDatabase&) = delete;
		AssetDatabase& operator=(const AssetDatabase&) = delete;

	private:
		std::filesystem::path assetPath_;

		std::map<std::filesystem::path, std::string> assetGuidList_;
		std::map<std::weak_ptr<const RttiObject>, std::filesystem::path, std::owner_less<std::weak_ptr<const RttiObject>>> assetPathList_;
	};
}

#endif