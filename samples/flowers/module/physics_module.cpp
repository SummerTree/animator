#include "physics_module.h"

namespace flower
{
	PhysicsModule::PhysicsModule() noexcept
	{
		this->reset();
	}

	PhysicsModule::~PhysicsModule() noexcept
	{
	}

	void
	PhysicsModule::reset() noexcept
	{
		this->gravity = octoon::math::float3(0.0, -9.8f * 10.f, 0.0f);
		this->fixedTimeStep = 1.0f / 60.f;
		this->playSolverIterationCounts = 1;
		this->previewSolverIterationCounts = 1;
		this->recordSolverIterationCounts = 10;
	}

	void 
	PhysicsModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("playSolverIterationCounts") != reader.end())
			this->playSolverIterationCounts = reader["playSolverIterationCounts"];
		if (reader.find("recordSolverIterationCounts") != reader.end())
			this->recordSolverIterationCounts = reader["recordSolverIterationCounts"];
		if (reader.find("previewSolverIterationCounts") != reader.end())
			this->previewSolverIterationCounts = reader["previewSolverIterationCounts"];
	}

	void 
	PhysicsModule::save(octoon::runtime::json& reader) noexcept
	{
		reader["gravity"] = { this->gravity[0], this->gravity[1], this->gravity[2] };
		reader["playSolverIterationCounts"] = this->playSolverIterationCounts;
		reader["recordSolverIterationCounts"] = this->recordSolverIterationCounts;
		reader["previewSolverIterationCounts"] = this->previewSolverIterationCounts;
	}
}