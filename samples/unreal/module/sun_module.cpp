#include "sun_module.h"

namespace unreal
{
	SunModule::SunModule() noexcept
	{
		this->reset();
	}

	SunModule::~SunModule() noexcept
	{
	}

	void
	SunModule::reset() noexcept
	{
		this->enable = false;
		this->size = 0.1f;
		this->intensity = 2.0f;
		this->color = octoon::math::float3(0.90196078f, 0.90196078f, 0.925490196f);
	}

	void 
	SunModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("enable") != reader.end())
			this->enable = reader["enable"].get<nlohmann::json::boolean_t>();

		if (reader.find("intensity") != reader.end())
			this->intensity = reader["intensity"].get<nlohmann::json::number_float_t>();

		if (reader.count("color"))
			this->color = octoon::math::float3(reader["color"][0], reader["color"][1], reader["color"][2]);

		if (reader.count("rotation"))
			this->rotation = octoon::math::float3(reader["rotation"][0], reader["rotation"][1], reader["rotation"][2]);
	}

	void 
	SunModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["enable"] = this->enable.getValue();
		writer["intensity"] = this->intensity.getValue();
		writer["color"].push_back(this->color.getValue().x);
		writer["color"].push_back(this->color.getValue().y);
		writer["color"].push_back(this->color.getValue().z);
		writer["rotation"].push_back(this->rotation.getValue().x);
		writer["rotation"].push_back(this->rotation.getValue().y);
		writer["rotation"].push_back(this->rotation.getValue().z);
	}
}