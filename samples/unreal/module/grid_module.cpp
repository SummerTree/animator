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
	GridModule::load(nlohmann::json& reader, std::shared_ptr<octoon::AssetBundle>& ab) noexcept
	{
		if (reader["enable"].is_boolean())
			this->enable = reader["enable"].get<nlohmann::json::boolean_t>();
	}

	void 
	GridModule::save(nlohmann::json& writer, std::shared_ptr<octoon::AssetBundle>& ab) noexcept
	{
		writer["enable"] = this->enable.getValue();
	}
}