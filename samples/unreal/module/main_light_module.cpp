#include "main_light_module.h"

namespace unreal
{
	MainLightModule::MainLightModule() noexcept
	{
		this->reset();
	}

	MainLightModule::~MainLightModule() noexcept
	{
	}

	void
	MainLightModule::reset() noexcept
	{
		this->enable = true;
		this->size = 0.1f;
		this->intensity = 2.0f;
		this->color = octoon::math::float3(0.90196078f, 0.90196078f, 0.925490196f);
		this->rotation = octoon::math::float3::Zero;
	}

	void 
	MainLightModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("enable") != reader.end())
			this->enable = reader["enable"].get<nlohmann::json::boolean_t>();

		if (reader.find("size") != reader.end())
			this->size = reader["size"].get<nlohmann::json::number_float_t>();

		if (reader.find("intensity") != reader.end())
			this->intensity = reader["intensity"].get<nlohmann::json::number_float_t>();

		if (reader.count("color"))
			this->color = octoon::math::float3(reader["color"][0], reader["color"][1], reader["color"][2]);

		if (reader.count("rotation"))
			this->rotation = octoon::math::float3(reader["rotation"][0], reader["rotation"][1], reader["rotation"][2]);
	}

	void 
	MainLightModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["enable"] = this->enable.getValue();
		writer["intensity"] = this->intensity.getValue();
		writer["size"] = this->size.getValue();
		writer["color"].push_back(this->color.getValue().x);
		writer["color"].push_back(this->color.getValue().y);
		writer["color"].push_back(this->color.getValue().z);
		writer["rotation"].push_back(this->rotation.getValue().x);
		writer["rotation"].push_back(this->rotation.getValue().y);
		writer["rotation"].push_back(this->rotation.getValue().z);
	}

	void
	MainLightModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->size.disconnect();
		this->intensity.disconnect();
		this->color.disconnect();
		this->rotation.disconnect();
	}
}