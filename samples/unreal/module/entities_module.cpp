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
		this->objects.getValue().clear();
	}

	void 
	EntitiesModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader["scene"].is_array())
		{
			for (auto& it : reader["scene"])
			{
				auto name = it["name"];
			}
		}
	}

	void 
	EntitiesModule::save(octoon::runtime::json& writer) noexcept
	{
		nlohmann::json jsons;

		for (auto& it : this->objects.getValue())
		{
			nlohmann::json json;
			json["name"] = it->getName();

			jsons.push_back(std::move(json));
		}

		writer["scene"] = jsons;
	}
}