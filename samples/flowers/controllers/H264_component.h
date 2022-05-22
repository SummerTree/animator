#ifndef FLOWER_H264_COMPONENT_H_
#define FLOWER_H264_COMPONENT_H_

#include "module/encode_module.h"
#include "flower_component.h"
#include <octoon/math/vector3.h>

struct x264_t;
struct x264_picture_t;

namespace flower
{
	class H264Component final : public RabbitComponent<EncodeModule>
	{
	public:
		H264Component() noexcept;

		bool create(std::string_view filepath) noexcept(false);
		void write(const octoon::math::Vector3* data) noexcept(false);
		void close() noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(H264Component);
		}

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	private:
		void convert(float* rgb, int w, int h, std::uint8_t* yuvBuf) noexcept;

	private:
		H264Component(const H264Component&) = delete;
		H264Component& operator=(const H264Component&) = delete;

	private:
		std::uint32_t width_;
		std::uint32_t height_;

		x264_t* encoder_;
		std::shared_ptr<x264_picture_t> frame_;
		std::shared_ptr<x264_picture_t> encoded_frame_;

		std::unique_ptr<char[]> enc_;
		std::unique_ptr<char[]> scratch_;
		std::unique_ptr<std::uint8_t[]> buf_;

		std::string filepath_;
		std::shared_ptr<std::ostream> ostream_;
	};
}

#endif