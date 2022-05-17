#ifndef FLOWER_H265_MODULE_H_
#define FLOWER_H265_MODULE_H_

#include <flower_model.h>
#include <cstdint>

namespace flower
{
	enum EncodeMode
	{
		H264,
		H265
	};

	enum VideoQuality
	{
		Low,
		Medium,
		High
	};

	class EncodeModule final : public FlowerModule
	{
	public:
		EncodeModule() noexcept;
		virtual ~EncodeModule() noexcept;

		void setVideoQuality(VideoQuality quality);

	public:
		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		EncodeModule(const EncodeModule&) = delete;
		EncodeModule& operator=(const EncodeModule&) = delete;

	public:
		bool enable;

		EncodeMode encodeMode;
		VideoQuality quality;

		double crf;

		std::uint32_t frame_type;
		std::uint32_t encode_speed;
	};
}

#endif