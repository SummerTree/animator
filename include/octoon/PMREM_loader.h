#ifndef OCTOON_PMREM_LOADER_H_
#define OCTOON_PMREM_LOADER_H_

#include <octoon/hal/graphics_texture.h>

namespace octoon
{
	class OCTOON_EXPORT PMREMLoader final
	{
	public:
		static std::shared_ptr<GraphicsTexture> load(std::string_view path, std::uint8_t mipNums = 8) noexcept(false);
		static std::shared_ptr<GraphicsTexture> load(const std::shared_ptr<GraphicsTexture>& texture, std::uint8_t mipNums = 8) noexcept(false);
	};
}

#endif