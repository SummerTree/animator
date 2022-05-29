#include "grid_module.h"

namespace unreal
{
	GridModule::GridModule() noexcept
	{
		this->reset();
	}

	GridModule::~GridModule() noexcept
	{
	}

	void
	GridModule::reset() noexcept
	{
	}

	void 
	GridModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader["enable"].is_boolean())
			this->enable = reader["enable"].get<nlohmann::json::boolean_t>();
	}

	void 
	GridModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["enable"] = this->enable.getValue();
	}
}