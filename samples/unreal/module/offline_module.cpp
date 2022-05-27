#include "offline_module.h"

namespace unreal
{
	OfflineModule::OfflineModule() noexcept
	{
		this->reset();
	}

	OfflineModule::~OfflineModule() noexcept
	{
	}

	void
	OfflineModule::reset() noexcept
	{
		this->bounces = 5;
		this->setEnable(false);
	}

	void 
	OfflineModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader["bounces"].is_number_unsigned())
			this->bounces = reader["bounces"].get<nlohmann::json::number_unsigned_t>();
	}

	void 
	OfflineModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["bounces"] = this->bounces.getValue();
	}

	void
	OfflineModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->bounces.disconnect();
	}
}