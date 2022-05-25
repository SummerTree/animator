#include "encode_module.h"

namespace unreal
{
	EncodeModule::EncodeModule() noexcept
	{
		this->reset();
	}

	EncodeModule::~EncodeModule() noexcept
	{
	}

	void
	EncodeModule::reset() noexcept
	{
		this->crf = 18;
		this->encodeMode = EncodeMode::H265;
		this->frame_type = 0;
		this->encode_speed = 0;
		this->quality = VideoQuality::High;
	}

	void 
	EncodeModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("crf") != reader.end())
			this->crf = reader["crf"].get<nlohmann::json::number_float_t>();
		if (reader.find("frame_type") != reader.end())
			this->frame_type = reader["frame_type"].get<nlohmann::json::number_unsigned_t>();
		if (reader.find("encode_speed") != reader.end())
			this->encode_speed = reader["encode_speed"].get<nlohmann::json::number_unsigned_t>();
	}

	void 
	EncodeModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["crf"] = this->crf.getValue();
		writer["frame_type"] = this->frame_type.getValue();
		writer["encode_speed"] = this->encode_speed.getValue();
	}

	void
	EncodeModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->crf.disconnect();
		this->encodeMode.disconnect();
		this->frame_type.disconnect();
		this->encode_speed.disconnect();
		this->quality.disconnect();
	}
}