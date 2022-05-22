#include "frame_sequence_component.h"
#include "flower_behaviour.h"

namespace flower
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