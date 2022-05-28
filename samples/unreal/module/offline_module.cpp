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
		this->spp = 150;
		this->sppCount = 0;
		this->setEnable(false);
	}

	void 
	OfflineModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader["spp"].is_number_float())
			this->spp = reader["spp"].get<nlohmann::json::number_unsigned_t>();
		if (reader["bounces"].is_number_unsigned())
			this->bounces = reader["bounces"].get<nlohmann::json::number_unsigned_t>();
	}

	void 
	OfflineModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["spp"] = this->spp.getValue();
		writer["bounces"] = this->bounces.getValue();
	}

	void
	OfflineModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->bounces.disconnect();
		this->spp.disconnect();
		this->sppCount.disconnect();
	}
}