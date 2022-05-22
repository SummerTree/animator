#ifndef UNREAL_PHYSICS_MODULE_H_
#define UNREAL_PHYSICS_MODULE_H_

#include <unreal_model.h>
#include <cstdint>
#include <octoon/math/vector3.h>

namespace unreal
{
	class PhysicsModule final : public UnrealModule
	{
	public:
		PhysicsModule() noexcept;
		virtual ~PhysicsModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		PhysicsModule(const PhysicsModule&) = delete;
		PhysicsModule& operator=(const PhysicsModule&) = delete;

	public:
		octoon::math::float3 gravity;

		float fixedTimeStep;
		float gravityScale;

		std::uint32_t playSolverIterationCounts;
		std::uint32_t recordSolverIterationCounts;
		std::uint32_t previewSolverIterationCounts;
	};
}

#endif