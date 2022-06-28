#ifndef OCTOON_PMX_LOADER_H_
#define OCTOON_PMX_LOADER_H_

#include <octoon/game_object.h>
#include <octoon/geometry/geometry.h>
#include <octoon/pmx.h>

namespace octoon
{
	struct PMXLoadFlagBits
	{
		enum Flags
		{
			NoneBit = 0,
			MaterialBit = 1,
			AllBit = MaterialBit
		};
	};

	typedef std::uint32_t PMXLoadFlags;

	class OCTOON_EXPORT PMXLoader final
	{
	public:
		static std::shared_ptr<GameObject> load(const std::filesystem::path& path, PMXLoadFlags flags) noexcept(false);

		static bool save(const GameObject& gameObject, PMX& pmx, const std::filesystem::path& path) noexcept(false);
		static bool save(const GameObject& gameObject, const std::filesystem::path& path) noexcept(false);
	};
}

#endif