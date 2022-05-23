#ifndef OCTOON_GRAPHICS_RESOURCE_H_
#define OCTOON_GRAPHICS_RESOURCE_H_

#include <octoon/hal/graphics_types.h>

namespace octoon
{
	namespace hal
	{
		class OCTOON_EXPORT GraphicsResource : public runtime::RttiInterface
		{
			OctoonDeclareSubInterface(GraphicsResource, runtime::RttiInterface)
		public:
			GraphicsResource() noexcept;
			virtual ~GraphicsResource() noexcept;

			virtual GraphicsDevicePtr getDevice() noexcept = 0;

		private:
			GraphicsResource(const GraphicsResource&) = delete;
			GraphicsResource& operator=(const GraphicsResource&) = delete;
		};
	}
}

#endif