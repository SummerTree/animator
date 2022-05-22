#include "frame_sequence_component.h"
#include "unreal_behaviour.h"

namespace unreal
{
	FrameSequenceComponent::FrameSequenceComponent() noexcept {}

	void
	FrameSequenceComponent::onEnable() noexcept
	{
	}

	void
	FrameSequenceComponent::onDisable() noexcept
	{
		this->close();
	}

	bool
	FrameSequenceComponent::create(std::string_view filepath) noexcept(false)
	{
		return false;
	}

	void
	FrameSequenceComponent::write(
		const octoon::math::Vector3* data) noexcept(false)
	{
	}

	void
	FrameSequenceComponent::close() noexcept
	{
	}
}