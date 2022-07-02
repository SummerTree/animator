#include <octoon/texture_importer.h>

namespace octoon
{
	OctoonImplementSubClass(TextureImporter, AssetImporter, "TextureImporter")

	TextureImporter::TextureImporter() noexcept
	{
	}

	TextureImporter::~TextureImporter()
	{
	}

	std::shared_ptr<Texture>
	TextureImporter::load(const std::filesystem::path& path) noexcept
	{
		assetPath_ = path;

		auto texture = std::make_shared<Texture>();
		if (texture->load(assetPath_))
		{
			texture->setName((char*)assetPath_.filename().u8string().c_str());
			texture_ = texture;
			return texture;
		}

		return nullptr;
	}
}