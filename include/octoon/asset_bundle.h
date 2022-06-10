#ifndef OCTOON_ASSET_BUNDLE_H_
#define OCTOON_ASSET_BUNDLE_H_

#include <octoon/game_object.h>
#include <octoon/texture/texture.h>
#include <octoon/material/material.h>
#include <octoon/runtime/singleton.h>
#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT AssetBundle final : public AssetImporter
	{
		OctoonDeclareSingleton(AssetBundle)
	public:
		AssetBundle() noexcept;
		~AssetBundle() noexcept;

		std::shared_ptr<octoon::Material> loadAssetAtPath(std::string_view uuid) noexcept(false);

		nlohmann::json importPackage(std::string_view path, bool generateMipmap = false) noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::Texture>& texture, std::string_view outputPath) noexcept(false);
	};
}

#endif