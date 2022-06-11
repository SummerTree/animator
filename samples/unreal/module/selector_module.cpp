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
	SelectorModule::load(nlohmann::json& reader, std::shared_ptr<octoon::AssetBundle>& ab) noexcept
	{
	}

	void 
	SelectorModule::save(nlohmann::json& writer, std::shared_ptr<octoon::AssetBundle>& ab) noexcept
	{
	}
}