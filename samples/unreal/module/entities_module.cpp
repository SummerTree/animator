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
		this->entities.getValue().clear();
		this->objects.clear();
	}

	void 
	EntitiesModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader["entities"].is_array())
		{
			for (auto& it : reader["entities"])
			{
				EntitesObject object;
				object.name = it["name"].get<nlohmann::json::string_t>();
				object.filepath = it["filepath"].get<nlohmann::json::string_t>();

				this->entities.getValue().push_back(object);
			}

			this->entities.submit();
		}
	}

	void 
	EntitiesModule::save(octoon::runtime::json& writer) noexcept
	{
		nlohmann::json jsons;

		for (auto& it : this->entities.getValue())
		{
			nlohmann::json json;
			json["name"] = it.name;
			json["filepath"] = it.filepath;

			jsons.push_back(std::move(json));
		}

		writer["entities"] = jsons;
	}
}