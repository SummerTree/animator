#ifndef OCTOON_PMX_LOADER_H_
#define OCTOON_PMX_LOADER_H_

#include <octoon/game_object.h>
#include <octoon/geometry/geometry.h>
#include <octoon/pmx.h>

namespace octoon
{
	class OCTOON_EXPORT PMXLoader final
	{
	public:
		static std::shared_ptr<GameObject> load(const PMX& pmx) noexcept(false);
		static std::shared_ptr<GameObject> load(std::string_view path) noexcept(false);

		static std::shared_ptr<Geometry> loadGeometry(const PMX& pmx) noexcept(false);

		static bool save(const GameObject& gameObject, PMX& pmx, std::string_view path) noexcept(false);
		static bool save(const GameObject& gameObject, std::string_view path) noexcept(false);
		static bool save(const GameObject& gameObject, std::wstring_view path) noexcept(false);
	};
}

#endif