#include "environment_module.h"
#include <octoon/asset_bundle.h>
#include <octoon/asset_database.h>

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
	EnvironmentModule::load(nlohmann::json& reader, std::string_view profilePath) noexcept
	{
		if (reader["enable"].is_boolean())
			this->enable = reader["enable"].get<nlohmann::json::boolean_t>();
		if (reader["showBackground"].is_boolean())
			this->showBackground = reader["showBackground"].get<nlohmann::json::boolean_t>();
		if (reader["useTexture"].is_boolean())
			this->useTexture = reader["useTexture"].get<nlohmann::json::boolean_t>();
		if (reader["color"].is_array())
			this->color = octoon::math::float3(reader["color"].get<std::array<float, 3>>());
		if (reader["offset"].is_array())
			this->offset = octoon::math::float2(reader["offset"].get<std::array<float, 2>>());
		if (reader["texture"].is_object())
			this->texture = octoon::AssetDatabase::instance()->loadAssetAtPackage<octoon::Texture>(reader["texture"]);
	}

	void 
	EnvironmentModule::save(nlohmann::json& writer, std::string_view profilePath) noexcept
	{
		writer["enable"] = this->enable.getValue();
		writer["intensity"] = this->intensity.getValue();
		writer["showBackground"] = this->showBackground.getValue();
		writer["useTexture"] = this->useTexture.getValue();
		writer["offset"] = this->offset.getValue().to_array();
		writer["color"] = this->color.getValue().to_array();

		if (this->texture.getValue())
			writer["texture"] = octoon::AssetBundle::instance()->createAsset(this->texture.getValue());
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