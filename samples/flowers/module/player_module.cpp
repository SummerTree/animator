#include "player_module.h"

namespace flower
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
		this->playing_ = false;
		this->spp = 100;
		this->sppCount_ = 0;
		this->recordFps = 30.0f;
		this->playTimeStep = 1.0f / 60.f;
		this->normalTimeStep = 1.0f / 60.f;
		this->endFrame = 0;
		this->startFrame = 0;
		this->timeLength = 0;
		this->curTime = 0;
	}

	void 
	PlayerModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("spp") != reader.end())
			this->spp = reader["spp"];
		if (reader.find("recordFps") != reader.end())
			this->recordFps = reader["recordFps"];
		if (reader.find("playTimeStep") != reader.end())
			this->playTimeStep = reader["playTimeStep"];
		if (reader.find("normalTimeStep") != reader.end())
			this->normalTimeStep = reader["normalTimeStep"];
	}

	void 
	PlayerModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["spp"] = this->spp;
		writer["recordFps"] = this->recordFps;
		writer["playTimeStep"] = this->playTimeStep;
		writer["normalTimeStep"] = this->normalTimeStep;
	}
}