#include "physics_module.h"

namespace unreal
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
			this->gravityScale = reader["gravityScale"].get<nlohmann::json::number_float_t>();
		if (reader.find("playSolverIterationCounts") != reader.end())
			this->playSolverIterationCounts = reader["playSolverIterationCounts"].get<nlohmann::json::number_unsigned_t>();
		if (reader.find("recordSolverIterationCounts") != reader.end())
			this->recordSolverIterationCounts = reader["recordSolverIterationCounts"].get<nlohmann::json::number_unsigned_t>();
		if (reader.find("previewSolverIterationCounts") != reader.end())
			this->previewSolverIterationCounts = reader["previewSolverIterationCounts"].get<nlohmann::json::number_unsigned_t>();
	}

	void 
	PhysicsModule::save(octoon::runtime::json& reader) noexcept
	{
		reader["gravity"] = { this->gravity.getValue()[0], this->gravity.getValue()[1], this->gravity.getValue()[2] };
		reader["gravityScale"] = this->gravityScale.getValue();
		reader["playSolverIterationCounts"] = this->playSolverIterationCounts.getValue();
		reader["recordSolverIterationCounts"] = this->recordSolverIterationCounts.getValue();
		reader["previewSolverIterationCounts"] = this->previewSolverIterationCounts.getValue();
	}
}