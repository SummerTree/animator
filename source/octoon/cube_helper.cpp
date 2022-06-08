#include <octoon/cube_helper.h>
#include <octoon/mesh_filter_component.h>
#include <octoon/mesh_renderer_component.h>
#include <octoon/mesh/cube_mesh.h>

namespace octoon
{
	GameObjectPtr
	CubeHelper::create(float width, float height, float depth, std::uint32_t widthSegments, std::uint32_t heightSegments, std::uint32_t depthSegments)
	{
		auto object = GameObject::create(std::string_view("GameObject"));
		object->addComponent<MeshFilterComponent>(CubeMesh::create(width, height, depth, widthSegments, heightSegments, depthSegments));
		object->addComponent<MeshRendererComponent>(std::make_shared<Material>());
		return object;
	}
}