#ifndef OCTOON_MESH_LOADER_H_
#define OCTOON_MESH_LOADER_H_

#include <octoon/game_object.h>
#include <octoon/model/model.h>
#include <octoon/geometry/geometry.h>

namespace octoon
{
	class OCTOON_EXPORT MeshLoader final
	{
	public:
		static std::shared_ptr<GameObject> load(std::string_view path, bool cache = true) noexcept(false);
		static std::shared_ptr<Geometry> load(const Model& model) noexcept(false);
	};
}

#endif