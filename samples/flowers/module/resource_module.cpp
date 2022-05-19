#include "resource_module.h"

namespace flower
{
	ResourceModule::ResourceModule() noexcept
	{
		this->reset();
	}

	ResourceModule::~ResourceModule() noexcept
	{
	}

	void
	ResourceModule::reset() noexcept
	{
		this->hdriPath = "../../system/hdri";
		this->materialPath = "../../system/materials";
	}

	void 
	ResourceModule::load(octoon::runtime::json& reader) noexcept
	{
	}

	void 
	ResourceModule::save(octoon::runtime::json& writer) noexcept
	{
	}
}