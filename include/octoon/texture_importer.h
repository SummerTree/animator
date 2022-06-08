#ifndef UNREAL_TEXTURE_IMPORTER_H_
#define UNREAL_TEXTURE_IMPORTER_H_

#include <octoon/game_object.h>
#include <octoon/hal/graphics_texture.h>
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

		std::shared_ptr<octoon::GraphicsTexture> importTexture(std::string_view path, bool generatorMipmap = false) noexcept(false);

		nlohmann::json createPackage(std::string_view path, bool generateMipmap = false) noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::GraphicsTexture>& texture, std::string_view outputPath = "") noexcept(false);

		std::shared_ptr<octoon::GraphicsTexture> loadPackage(const nlohmann::json& package, std::string_view outputPath = "") noexcept(false);
	};
}

#endif