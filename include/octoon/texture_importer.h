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
		TextureImporter(const std::filesystem::path& path) noexcept;
		virtual ~TextureImporter() noexcept;

		virtual std::shared_ptr<Object> importer() noexcept(false) override;

	private:
		TextureImporter(const TextureImporter&) = delete;
		TextureImporter& operator=(const TextureImporter&) = delete;
	};
}

#endif