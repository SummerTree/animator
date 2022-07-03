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

			auto metadata = this->loadMetadataAtPath(path);
			if (metadata.is_object())
			{
				if (metadata.contains("mipmap"))
					texture->setMipLevel(metadata["mipmap"].get<nlohmann::json::number_integer_t>());
			}
			else
			{
				auto ext = path.extension().u8string();
				for (auto& it : ext)
					it = (char)std::tolower(it);

				if (ext == u8".hdr")
					texture->setMipLevel(8);
			}

			texture->apply();

			return texture;
		}

		return nullptr;
	}
}