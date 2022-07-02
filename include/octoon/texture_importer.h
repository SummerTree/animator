#ifndef OCTOON_TEXTURE_IMPORTER_H_
#define OCTOON_TEXTURE_IMPORTER_H_

#include <octoon/asset_importer.h>
#include <octoon/texture/texture.h>

namespace octoon
{
	class OCTOON_EXPORT TextureImporter final : public AssetImporter
	{
		OctoonDeclareSubClass(TextureImporter, AssetImporter)
	public:
		TextureImporter() noexcept;
		virtual ~TextureImporter() noexcept;

		std::shared_ptr<Texture> load(const std::filesystem::path& path) noexcept;

	private:
		TextureImporter(const TextureImporter&) = delete;
		TextureImporter& operator=(const TextureImporter&) = delete;

	private:
		std::weak_ptr<const Texture> texture_;
	};
}

#endif