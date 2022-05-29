#ifndef OCTOON_MODEL_LOADER_H_
#define OCTOON_MODEL_LOADER_H_

#include <octoon/pmx_loader.h>

namespace octoon
{
	class OCTOON_EXPORT ModelLoader final
	{
	public:
		ModelLoader() = default;
		~ModelLoader() = default;

		static Model load(const PMX& pmx) noexcept;
	};
}

#endif