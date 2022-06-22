#ifndef OCTOON_ASSET_DATABASE_H_
#define OCTOON_ASSET_DATABASE_H_

#include <octoon/pmx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/animation/animation.h>
#include <octoon/video/renderer.h>
#include <filesystem>
#include <set>
#include <map>

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

		void createAsset(const std::shared_ptr<const Texture>& texture, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<const Animation>& animation, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<const Material>& material, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<const PMX>& pmx, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<const GameObject>& object, const std::filesystem::path& relativePath) noexcept(false);

		bool contains(const std::shared_ptr<const RttiObject>& asset) const noexcept;

		void deleteAsset(const std::filesystem::path& relativePath) noexcept(false);
		void saveAssets() noexcept(false);

		std::filesystem::path getAssetPath(const std::string& uuid) const noexcept;
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

		bool isDirty() const noexcept;
		bool isDirty(const std::shared_ptr<RttiObject>& object) const noexcept;
		void setDirty(const std::shared_ptr<RttiObject>& object, bool dirty = true) noexcept(false);
		void clearUpdate() noexcept;

	private:
		nlohmann::json loadMetadataAtPath(const std::filesystem::path& path) noexcept(false);

	private:
		AssetDatabase(const AssetDatabase&) = delete;
		AssetDatabase& operator=(const AssetDatabase&) = delete;

	private:
		std::filesystem::path assetPath_;

		std::set<std::weak_ptr<const RttiObject>, std::owner_less<std::weak_ptr<const RttiObject>>> dirtyList_;

		std::map<std::filesystem::path, std::string> assetGuidList_;
		std::map<std::string, std::filesystem::path> assetPathList_;
		std::map<std::weak_ptr<const RttiObject>, std::filesystem::path, std::owner_less<std::weak_ptr<const RttiObject>>> objectPathList_;
	};
}

#endif