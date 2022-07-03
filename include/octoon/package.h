#ifndef OCTOON_PACKAGE_H_
#define OCTOON_PACKAGE_H_

#include <octoon/game_object.h>
#include <octoon/asset_importer.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT Package final
	{
	public:
		Package(const std::u8string& name) noexcept;
		~Package() noexcept;

		void open(const std::filesystem::path& diskPath) noexcept(false);
		void close() noexcept;

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
		Package(const Package&) = delete;
		Package& operator=(const Package&) = delete;

	private:
		std::u8string name_;
		std::filesystem::path rootPath_;

		std::vector<std::filesystem::path> assets_;
	};
}

#endif