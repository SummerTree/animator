#include "sound_module.h"

namespace unreal
{
	SoundModule::SoundModule() noexcept
	{
		this->reset();
	}

	SoundModule::~SoundModule() noexcept
	{
	}

	void
	SoundModule::reset() noexcept
	{
		this->enable = true;
		this->volume = 0.5f;
		this->filepath = std::string();
	}

	void 
	SoundModule::load(nlohmann::json& reader, std::string_view path) noexcept
	{
		if (reader["enable"].is_boolean())
			this->enable = reader["enable"].get<nlohmann::json::boolean_t>();
		if (reader["volume"].is_number_float())
			this->volume = reader["volume"].get<nlohmann::json::number_float_t>();
		if (reader["filepath"].is_string())
			this->filepath = reader["filepath"].get<nlohmann::json::string_t>();
	}

	void 
	SoundModule::save(nlohmann::json& writer, std::string_view path) noexcept
	{
		writer["enable"] = this->enable.getValue();
		writer["volume"] = this->volume.getValue();
		writer["filepath"] = this->filepath.getValue();
	}
}