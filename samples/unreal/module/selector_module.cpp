#include "selector_module.h"

namespace unreal
{
	SelectorModule::SelectorModule() noexcept
	{
		this->reset();
	}

	SelectorModule::~SelectorModule() noexcept
	{
	}

	void
	SelectorModule::reset() noexcept
	{
		this->selectedItem_ = std::nullopt;
		this->selectedItemHover_ = std::nullopt;
	}

	void 
	SelectorModule::load(octoon::runtime::json& reader, std::string_view path) noexcept
	{
	}

	void 
	SelectorModule::save(octoon::runtime::json& writer, std::string_view path) noexcept
	{
	}
}