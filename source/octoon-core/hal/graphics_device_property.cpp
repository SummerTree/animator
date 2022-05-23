#include <octoon/hal/graphics_device_property.h>

namespace octoon
{
	namespace hal
	{
		OctoonImplementSubInterface(GraphicsDeviceProperty, runtime::RttiInterface, "GraphicsDeviceProperty")

		GraphicsDeviceProperty::GraphicsDeviceProperty() noexcept
		{
		}

		GraphicsDeviceProperty::~GraphicsDeviceProperty() noexcept
		{
		}
	}
}