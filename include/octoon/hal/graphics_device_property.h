#ifndef OCTOON_GRAPHICS_DEVICE_PROPERTY_H_
#define OCTOON_GRAPHICS_DEVICE_PROPERTY_H_

#include <octoon/hal/graphics_types.h>
#include <octoon/hal/system_info.h>

namespace octoon
{
	namespace hal
	{
		class GraphicsDeviceProperty : public runtime::RttiInterface
		{
			OctoonDeclareSubInterface(GraphicsDeviceProperty, runtime::RttiInterface)
		public:
			GraphicsDeviceProperty() noexcept;
			virtual ~GraphicsDeviceProperty() noexcept;

			virtual const SystemInfo& getSystemInfo() const noexcept = 0;

		private:
			GraphicsDeviceProperty(const GraphicsDeviceProperty&) = delete;
			GraphicsDeviceProperty& operator=(const GraphicsDeviceProperty&) = delete;
		};
	}
}

#endif