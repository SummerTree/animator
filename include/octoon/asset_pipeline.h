#ifndef OCTOON_ASSET_PIPELINE_H_
#define OCTOON_ASSET_PIPELINE_H_

#include <octoon/game_object.h>
#include <octoon/asset_importer.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT AssetPipeline final
	{
	public:
		AssetPipeline(const std::u8string& name) noexcept;
		~AssetPipeline() noexcept;

		void open(const std::filesystem::path& diskPath) noexcept(false);
		void close() noexcept;

		const std::u8string& getName() const noexcept;

		bool isValidPath(const std::filesystem::path& diskPath) const noexcept;

		std::filesystem::path getAbsolutePath(const std::filesystem::path& relativePath) const noexcept;

		void saveAssets() noexcept(false);

		std::shared_ptr<Object> loadAssetAtPath(const std::filesystem::path& assetPath) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& assetPath) noexcept(false)
		{
			auto asset = loadAssetAtPath(assetPath);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

	private:
		std::filesystem::path getRelativePath(const std::filesystem::path& assetPath) const noexcept(false);

	private:
		AssetPipeline(const AssetPipeline&) = delete;
		AssetPipeline& operator=(const AssetPipeline&) = delete;

	private:
		std::u8string name_;
		std::filesystem::path rootPath_;

		std::vector<std::filesystem::path> assets_;
	};
}

#endif