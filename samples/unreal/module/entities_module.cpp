#include "entities_module.h"

namespace unreal
{
	EntitiesModule::EntitiesModule() noexcept
	{
		this->reset();
	}

	EntitiesModule::~EntitiesModule() noexcept
	{
	}

	void
	EntitiesModule::reset() noexcept
	{
		this->camera.getValue().reset();
		this->objects.clear();
		this->objects.shrink_to_fit();
	}

	void 
	EntitiesModule::load(octoon::runtime::json& reader) noexcept
	{
	}

	void 
	EntitiesModule::save(octoon::runtime::json& reader) noexcept
	{
	}
}