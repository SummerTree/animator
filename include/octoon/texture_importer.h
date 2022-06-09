#ifndef OCTOON_TEXTURE_IMPORTER_H_
#define OCTOON_TEXTURE_IMPORTER_H_

#include <octoon/game_object.h>
#include <octoon/texture/texture.h>
#include <octoon/runtime/singleton.h>
#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT TextureImporter final : public AssetImporter
	{
		OctoonDeclareSingleton(TextureImporter)
	public:
		TextureImporter() noexcept;
		~TextureImporter() noexcept;

		nlohmann::json importPackage(std::string_view path, bool generateMipmap = false) noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::Texture>& texture, std::string_view outputPath = "") noexcept(false);
	};
}

#endif