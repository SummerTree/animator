#ifndef OCTOON_TEXTURE_LOADER_H_
#define OCTOON_TEXTURE_LOADER_H_

#include <octoon/hal/graphics_texture.h>
#include <octoon/image/image.h>

namespace octoon
{
	class OCTOON_EXPORT TextureLoader final
	{
	public:
		static std::shared_ptr<GraphicsTexture> load(std::string_view path, bool generatorMipmap = false) noexcept(false);
		static std::shared_ptr<GraphicsTexture> load(const Image& image, std::string_view filepath, bool generatorMipmap = false) noexcept(false);

		static bool save(std::string_view filepath, const std::shared_ptr<GraphicsTexture>& texture) noexcept(false);
	};
}

#endif