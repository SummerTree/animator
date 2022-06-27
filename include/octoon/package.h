#ifndef OCTOON_PACKAGE_H_
#define OCTOON_PACKAGE_H_

#include <octoon/pmx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/material/material.h>
#include <octoon/animation/animation.h>
#include <octoon/game_object.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT Package final
	{
	public:
		Package(AssetDatabase* assetDatabase) noexcept;
		~Package() noexcept;

		void open(const std::filesystem::path& path) noexcept(false);
		void close() noexcept;

		void importAsset(const std::filesystem::path& diskPath, const std::filesystem::path& relativePath) noexcept(false);

		void createAsset(const std::shared_ptr<const Texture>& texture, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<const Animation>& animation, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<const Material>& material, const std::filesystem::path& relativePath) noexcept(false);
		void createAsset(const std::shared_ptr<const GameObject>& object, const std::filesystem::path& relativePath) noexcept(false);

		void createPrefab(const std::shared_ptr<const GameObject>& object, const std::filesystem::path& relativePath) noexcept(false);

		bool contains(const std::shared_ptr<const Object>& asset) const noexcept;

		std::filesystem::path getAssetPath(const std::string& uuid) const noexcept;
		std::filesystem::path getAssetPath(const std::shared_ptr<const Object>& asset) const noexcept;
		std::filesystem::path getAbsolutePath(const std::filesystem::path& relativePath) const noexcept;

		std::string getAssetGuid(const std::filesystem::path& relativePath) const noexcept;
		std::string getAssetGuid(const std::shared_ptr<const Object>& asset) const noexcept;

		void deleteAsset(const std::filesystem::path& relativePath) noexcept(false);
		void saveAssets() noexcept(false);

		void createFolder(const std::filesystem::path& folderPath) noexcept(false);
		void deleteFolder(const std::filesystem::path& folderPath) noexcept(false);

		std::shared_ptr<Object> loadAssetAtPath(const std::filesystem::path& relativePath) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& relativePath) noexcept(false)
		{
			auto asset = loadAssetAtPath(relativePath);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

		void setLabels(const std::shared_ptr<const Object>& asset, std::vector<std::string>&& labels) noexcept;
		void setLabels(const std::shared_ptr<const Object>& asset, const std::vector<std::string>& labels) noexcept;
		const std::vector<std::string>& getLabels(const std::shared_ptr<const Object>& asset) noexcept;

		bool getGUIDAndLocalIdentifier(const std::shared_ptr<const Object>& asset, const std::string& outGuid, std::int64_t& outLocalId);

	private:
		void createMetadataAtPath(const std::filesystem::path& path) noexcept(false);
		void createMetadataAtPath(const std::filesystem::path& path, const nlohmann::json& json) noexcept(false);
		void removeMetadataAtPath(const std::filesystem::path& path) noexcept;
		nlohmann::json loadMetadataAtPath(const std::filesystem::path& path) noexcept(false);

	private:
		Package(const Package&) = delete;
		Package& operator=(const Package&) = delete;

	private:
		AssetDatabase* assetDatabase_;

		std::filesystem::path rootPath_;
		std::vector<std::string> defaultLabel_;

		std::map<std::string, std::filesystem::path> uniques_;
		std::map<std::filesystem::path, std::string> paths_;
		std::map<std::filesystem::path, std::weak_ptr<Object>> objectCaches_;

		std::map<std::weak_ptr<const Object>, std::vector<std::string>, std::owner_less<std::weak_ptr<const Object>>> labels_;
	};
}

#endif