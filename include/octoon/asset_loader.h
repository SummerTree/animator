#ifndef OCTOON_ASSET_LOADER_H_
#define OCTOON_ASSET_LOADER_H_

#include <octoon/pmx_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/animation/animation.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT AssetLoader final
	{
		OctoonDeclareSingleton(AssetLoader)
	public:
		AssetLoader() noexcept;
		virtual ~AssetLoader() noexcept;

		std::filesystem::path getAssetPath(const std::shared_ptr<const RttiObject>& asset) const noexcept;
		std::filesystem::path getAssetExtension(const std::shared_ptr<const RttiObject>& asset, std::string_view defaultExtension = "") const noexcept;

		std::shared_ptr<RttiObject> loadAssetAtPath(const std::filesystem::path& path) noexcept(false);

		template<typename T>
		std::shared_ptr<T> loadAssetAtPath(const std::filesystem::path& path) noexcept(false)
		{
			auto asset = loadAssetAtPath(path);
			if (asset)
				return asset->downcast_pointer<T>();
			return nullptr;
		}

	private:
		AssetLoader(const AssetLoader&) = delete;
		AssetLoader& operator=(const AssetLoader&) = delete;

	private:
		std::map<std::weak_ptr<const RttiObject>, std::filesystem::path, std::owner_less<std::weak_ptr<const RttiObject>>> pathList_;
	};
}

#endif