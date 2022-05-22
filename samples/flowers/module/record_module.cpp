#include "record_module.h"

namespace flower
{
	RecordModule::RecordModule() noexcept
	{
		this->reset();
	}

	RecordModule::~RecordModule() noexcept
	{
	}

	void
	RecordModule::reset() noexcept
	{
		this->width = 1280;
		this->height = 720;

		this->hdr = false;
		this->srgb = true;
		this->active = false;
		this->denoise = true;
	}

	void
	RecordModule::resize(std::uint32_t w, std::uint32_t h) noexcept
	{
		this->width = w;
		this->height = h;

		this->hdr = false;
		this->srgb = true;
		this->active = false;
		this->denoise = true;
	}

	void 
	RecordModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("width") != reader.end())
			this->width = reader["width"];
		if (reader.find("height") != reader.end())
			this->height = reader["height"];
	}

	void 
	RecordModule::save(octoon::runtime::json& reader) noexcept
	{
		reader["width"] = this->width;
		reader["height"] = this->height;
	}
}