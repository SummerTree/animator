#include "environment_module.h"
#include "../importer/texture_importer.h"

namespace unreal
{
	EnvironmentModule::EnvironmentModule() noexcept
	{
		this->reset();
	}

	EnvironmentModule::~EnvironmentModule() noexcept
	{
	}

	void
	EnvironmentModule::reset() noexcept
	{
		this->intensity = 1.0f;
		this->showBackground = true;
		this->useTexture = true;
		this->texture = nullptr;
		this->offset = octoon::math::float2(0.f, 0.f);
		this->color = octoon::math::float3(0.90196078f, 0.90196078f, 0.925490196f);
	}

	void 
	EnvironmentModule::load(octoon::runtime::json& reader, std::string_view path) noexcept
	{
		if (reader["enable"].is_boolean())
			this->enable = reader["enable"].get<nlohmann::json::boolean_t>();
		if (reader["showBackground"].is_boolean())
			this->showBackground = reader["showBackground"].get<nlohmann::json::boolean_t>();
		if (reader["useTexture"].is_boolean())
			this->useTexture = reader["useTexture"].get<nlohmann::json::boolean_t>();
		if (reader["texture"].is_object())
			this->texture = TextureImporter::instance()->loadMetaData(reader["texture"]);
		if (reader["color"].is_array())
			this->color = octoon::math::float3(reader["color"][0], reader["color"][1], reader["color"][2]);
		if (reader["offset"].is_array())
			this->offset = octoon::math::float2(reader["offset"][0], reader["offset"][1]);
	}

	void 
	EnvironmentModule::save(octoon::runtime::json& writer, std::string_view path) noexcept
	{
		writer["enable"] = this->enable.getValue();
		writer["intensity"] = this->intensity.getValue();
		writer["showBackground"] = this->showBackground.getValue();
		writer["useTexture"] = this->useTexture.getValue();
		writer["offset"].push_back(this->offset.getValue().x);
		writer["offset"].push_back(this->offset.getValue().y);
		writer["color"].push_back(this->color.getValue().x);
		writer["color"].push_back(this->color.getValue().y);
		writer["color"].push_back(this->color.getValue().z);

		if (this->texture.getValue())
		{
			nlohmann::json json;
			writer["texture"] = TextureImporter::instance()->createMetadata(this->texture.getValue());
		}
	}

	void
	EnvironmentModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->intensity.disconnect();
		this->showBackground.disconnect();
		this->useTexture.disconnect();
		this->texture.disconnect();
		this->offset.disconnect();
		this->color.disconnect();
	}
}