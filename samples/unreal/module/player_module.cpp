#include "player_module.h"

namespace unreal
{
	PlayerModule::PlayerModule() noexcept
	{
		this->reset();
	}

	PlayerModule::~PlayerModule() noexcept
	{
	}

	void
	PlayerModule::reset() noexcept
	{
		this->finish = true;
		this->isPlaying = false;
		this->spp = 150;
		this->sppCount = 0;
		this->recordFps = 30.0f;
		this->playTimeStep = 1.0f / 60.f;
		this->previewTimeStep = 1.0f / 60.f;
		this->endFrame = 0;
		this->startFrame = 0;
		this->timeLength = 0;
		this->curTime = 0;
		this->takeupTime = 0;
		this->estimatedTime = 0;
	}

	void 
	PlayerModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("spp") != reader.end())
			this->spp = reader["spp"].get<nlohmann::json::number_unsigned_t>();
		if (reader.find("recordFps") != reader.end())
			this->recordFps = reader["recordFps"].get<nlohmann::json::number_float_t>();
		if (reader.find("playTimeStep") != reader.end())
			this->playTimeStep = reader["playTimeStep"].get<nlohmann::json::number_float_t>();
		if (reader.find("normalTimeStep") != reader.end())
			this->previewTimeStep = reader["normalTimeStep"].get<nlohmann::json::number_float_t>();
	}

	void 
	PlayerModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["spp"] = this->spp.getValue();
		writer["recordFps"] = this->recordFps.getValue();
		writer["playTimeStep"] = this->playTimeStep.getValue();
		writer["normalTimeStep"] = this->previewTimeStep.getValue();
	}

	void
	PlayerModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->finish.disconnect();
		this->isPlaying.disconnect();
		this->spp.disconnect();
		this->sppCount.disconnect();
		this->recordFps.disconnect();
		this->playTimeStep.disconnect();
		this->previewTimeStep.disconnect();
		this->endFrame.disconnect();
		this->startFrame.disconnect();
		this->timeLength.disconnect();
		this->curTime.disconnect();
		this->takeupTime.disconnect();
		this->estimatedTime.disconnect();
	}
}