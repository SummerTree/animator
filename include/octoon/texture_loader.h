#ifndef OCTOON_TEXTURE_LOADER_H_
#define OCTOON_TEXTURE_LOADER_H_

#include <octoon/hal/graphics_types.h>
#include <octoon/image/image.h>

namespace octoon
{
	class OCTOON_EXPORT TextureLoader final
	{
	public:
		static hal::GraphicsTexturePtr load(std::string_view path, bool generatorMipmap = false, bool cache = true) noexcept(false);
		static hal::GraphicsTexturePtr load(const Image& image, bool generatorMipmap = false) noexcept(false);
	};
}

#endif