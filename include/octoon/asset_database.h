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

		void importAsset(const std::filesystem::path& diskPath, const std::filesystem::path& assetPath) noexcept(false);
		void importPackage(const std::filesystem::path& diskPath) noexcept(false);

		void createAsset(const std::shared_ptr<const Texture>& texture, const std::filesystem::path& assetPath) noexcept(false);
		void createAsset(const std::shared_ptr<const Animation>& animation, const std::filesystem::path& assetPath) noexcept(false);
		void createAsset(const std::shared_ptr<const Material>& material, const std::filesystem::path& assetPath) noexcept(false);
		void createAsset(const std::shared_ptr<const GameObject>& object, const std::filesystem::path& assetPath) noexcept(false);

		bool contains(const std::shared_ptr<const Object>& asset) const noexcept;

		void deleteAsset(const std::filesystem::path& assetPath) noexcept(false);
		void saveAssets() noexcept(false);

		void createFolder(const std::filesystem::path& assetFolder) noexcept(false);
		void deleteFolder(const std::filesystem::path& assetFolder) noexcept(false);

		std::filesystem::path getAssetPath(const std::string& uuid) const noexcept;
		std::filesystem::path getAssetPath(const std::shared_ptr<const Object>& asset) const noexcept;
		std::filesystem::path getAbsolutePath(const std::filesystem::path& assetPath) const noexcept(false);

		std::string getAssetGuid(const std::filesystem::path& assetPath) const noexcept;
		std::string getAssetGuid(const std::shared_ptr<const Object>& asset) const noexcept;

		std::shared_ptr<Object> loadAssetAtPath(const std::filesystem::path& assetPath) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& assetPath) noexcept(false)
		{
			auto asset = loadAssetAtPath(assetPath);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

		bool isDirty() const noexcept;
		bool isDirty(const std::shared_ptr<Object>& object) const noexcept;
		void setDirty(const std::shared_ptr<Object>& object, bool dirty = true) noexcept(false);
		void clearUpdate() noexcept;

	private:
		void createMetadataAtPath(const std::filesystem::path& path) noexcept(false);
		void removeMetadataAtPath(const std::filesystem::path& path) noexcept;
		nlohmann::json loadMetadataAtPath(const std::filesystem::path& path) noexcept(false);

	private:
		AssetDatabase(const AssetDatabase&) = delete;
		AssetDatabase& operator=(const AssetDatabase&) = delete;

	private:
		std::filesystem::path rootPath_;

		std::set<std::weak_ptr<const Object>, std::owner_less<std::weak_ptr<const Object>>> dirtyList_;

		std::map<std::filesystem::path, std::string> assetPaths_;
		std::map<std::string, std::filesystem::path> assetUniques_;
		std::map<std::filesystem::path, std::filesystem::path> packagePath_;

		std::map<std::filesystem::path, std::weak_ptr<Object>> objectCaches_;
		std::map<std::weak_ptr<const Object>, std::filesystem::path, std::owner_less<std::weak_ptr<const Object>>> objectPathList_;
	};
}

#endif