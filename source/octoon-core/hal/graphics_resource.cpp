#include <octoon/hal/graphics_resource.h>

namespace octoon
{
	OctoonImplementSubInterface(GraphicsResource, runtime::RttiObject, "GraphicsResource")

	GraphicsResource::GraphicsResource() noexcept
	{
	}

	GraphicsResource::~GraphicsResource() noexcept
	{
	}
}