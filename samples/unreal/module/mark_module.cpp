#include "mark_module.h"

namespace unreal
{
	MarkModule::MarkModule() noexcept
	{
		this->reset();
	}

	MarkModule::~MarkModule() noexcept
	{
	}

	void
	MarkModule::reset() noexcept
	{
		x = 24;
		y = 24;
		width = 0;
		height = 0;
		channel = 8;
		this->setEnable(false);
	}

	void 
	MarkModule::load(octoon::runtime::json& reader, std::string_view path) noexcept
	{
	}

	void 
	MarkModule::save(octoon::runtime::json& writer, std::string_view path) noexcept
	{
	}
}