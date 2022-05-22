
#ifndef FLOWER_FRAME_SEQUENCE_COMPONENT_H_
#define FLOWER_FRAME_SEQUENCE_COMPONENT_H_

#include "flower_component.h"
#include "module/encode_module.h"

#include <octoon/math/vector3.h>

namespace flower
{
	class FrameSequenceComponent final : public RabbitComponent<EncodeModule>
	{
	  public:
		FrameSequenceComponent() noexcept;

		bool create(std::string_view filepath) noexcept(false);
		void write(const octoon::math::Vector3* data) noexcept(false);
		void close() noexcept;

		virtual const std::type_info&
		type_info() const noexcept
		{
			return typeid(FrameSequenceComponent);
		}

	  private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	  private:
		FrameSequenceComponent(const FrameSequenceComponent&) = delete;
		FrameSequenceComponent& operator=(const FrameSequenceComponent&) = delete;

	  private:
		std::uint32_t width_;
		std::uint32_t height_;

		std::string filepath_;
		std::string dirpath_;
		std::string filename_;
		std::string basename_;
		std::string extension_;
		std::int32_t count_;
	};

}

#endif