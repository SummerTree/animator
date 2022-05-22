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
		this->gravity = octoon::math::float3(0.0, -9.8f, 0.0f);
		this->gravityScale = 5.0f;
		this->fixedTimeStep = 1.0f / 60.f;
		this->playSolverIterationCounts = std::numeric_limits<int>::max();
		this->previewSolverIterationCounts = std::numeric_limits<int>::max();
		this->recordSolverIterationCounts = std::numeric_limits<int>::max();
	}

	void 
	PhysicsModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("gravityScale") != reader.end())
			this->gravityScale = reader["gravityScale"];
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
		reader["gravityScale"] = this->gravityScale;
		reader["playSolverIterationCounts"] = this->playSolverIterationCounts;
		reader["recordSolverIterationCounts"] = this->recordSolverIterationCounts;
		reader["previewSolverIterationCounts"] = this->previewSolverIterationCounts;
	}
}